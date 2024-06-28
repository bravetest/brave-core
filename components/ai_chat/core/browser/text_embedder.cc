/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/ai_chat/core/browser/text_embedder.h"

#include "base/hash/hash.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/sequence_checker.h"
#include "base/strings/string_split.h"
#include "base/task/bind_post_task.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"
#include "base/timer/elapsed_timer.h"
#include "third_party/tflite/src/tensorflow/lite/core/api/op_resolver.h"
#include "third_party/tflite/src/tensorflow/lite/core/kernels/register.h"
#include "third_party/tflite_support/src/tensorflow_lite_support/cc/task/text/utils/text_op_resolver.h"

using TFLiteTextEmbedder = tflite::task::text::TextEmbedder;
using TFLiteTextEmbedderOptions = tflite::task::text::TextEmbedderOptions;
using tflite::task::text::CreateTextOpResolver;

namespace ai_chat {

// static
std::unique_ptr<TextEmbedder> TextEmbedder::Create(
    const base::FilePath& model_path) {
  auto embedder = base::WrapUnique(new TextEmbedder());
  TFLiteTextEmbedderOptions options;
  options.mutable_base_options()->mutable_model_file()->set_file_name(
      model_path.value().c_str());
  auto maybe_text_embedder =
      TFLiteTextEmbedder::CreateFromOptions(options, CreateTextOpResolver());
  if (!maybe_text_embedder.ok()) {
    LOG(ERROR) << maybe_text_embedder.status().ToString();
    return nullptr;
  }

  embedder->tflite_text_embedder_ = std::move(maybe_text_embedder.value());
  return embedder;
}

TextEmbedder::TextEmbedder()
    : owner_task_runner_(base::SequencedTaskRunner::GetCurrentDefault()),
      embedder_task_runner_(
          base::ThreadPool::CreateSequencedTaskRunner({base::MayBlock()})) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owner_sequence_checker_);
  DETACH_FROM_SEQUENCE(embedder_sequence_checker_);
}

TextEmbedder::~TextEmbedder() = default;

void TextEmbedder::GetTopSimilarityWithPromptTilContextLimit(
    const std::string& prompt,
    const std::string& text,
    uint32_t context_limit,
    TopSimilarityCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(owner_sequence_checker_);
  embedder_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          &TextEmbedder::GetTopSimilarityWithPromptTilContextLimitInternal,
          weak_ptr_factory_.GetWeakPtr(), prompt, text, context_limit,
          base::BindPostTaskToCurrentDefault(std::move(callback))));
}

void TextEmbedder::GetTopSimilarityWithPromptTilContextLimitInternal(
    const std::string& prompt,
    const std::string& text,
    uint32_t context_limit,
    TopSimilarityCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  LOG(ERROR) << text;
  auto text_hash = base::FastHash(base::as_bytes(base::make_span(text)));
  if (text_hash != text_hash_) {
    text_hash_ = text_hash;
    segments_ = SplitSegments(text);
    auto status = EmbedSegments();
    if (!status.ok()) {
      std::move(callback).Run(base::unexpected(status.ToString()));
      return;
    }
  }

  using ScoreType = std::pair<size_t, double>;
  std::vector<ScoreType> ranked_sentences;
  auto maybe_prompt_embed = tflite_text_embedder_->Embed(prompt);
  if (!maybe_prompt_embed.ok()) {
    std::move(callback).Run(
        base::unexpected(maybe_prompt_embed.status().ToString()));
    return;
  }
  for (size_t i = 0; i < embeddings_.size(); i++) {
    auto maybe_similarity = tflite_text_embedder_->CosineSimilarity(
        maybe_prompt_embed->embeddings(0).feature_vector(),
        embeddings_[i].embeddings(0).feature_vector());
    if (!maybe_similarity.ok()) {
      std::move(callback).Run(
          base::unexpected(maybe_similarity.status().ToString()));
      return;
    }
    ranked_sentences.push_back(std::make_pair(i, maybe_similarity.value()));
  }
  LOG(ERROR) << "First Sorting...";
  base::ElapsedTimer timer;
  std::sort(ranked_sentences.begin(), ranked_sentences.end(),
            [](const ScoreType& lhs, const ScoreType& rhs) {
              return lhs.second > rhs.second;
            });
  LOG(ERROR) << "First Sorting Time: " << timer.Elapsed().InMilliseconds()
             << "ms";
  std::vector<size_t> top_k_indices;
  size_t total_length = 0;
  std::string refined_page_content = "";
  for (const auto& ranked_sentence : ranked_sentences) {
    const auto& segment = segments_[ranked_sentence.first];
    if (total_length + segment.size() > context_limit) {
      break;
    }
    total_length += segment.size();
    top_k_indices.push_back(ranked_sentence.first);
  }
  LOG(ERROR) << "Second Sorting...";
  timer = base::ElapsedTimer();
  std::sort(top_k_indices.begin(), top_k_indices.end());
  for (const auto& index : top_k_indices) {
    refined_page_content += segments_[index] + ". ";
  }
  LOG(ERROR) << "Second Sorting Time: " << timer.Elapsed().InMilliseconds()
             << "ms";
  LOG(ERROR) << "Refined page content: " << refined_page_content;
  std::move(callback).Run(base::ok(refined_page_content));
}

std::vector<std::string> TextEmbedder::SplitSegments(const std::string& text) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  base::ElapsedTimer timer;
  auto segments = base::SplitStringUsingSubstr(
      text, ". ", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  LOG(ERROR) << "Segments: " << segments.size();
  if (segments.size() >= 300) {
    std::vector<std::string> new_segments;
    size_t join_size = segments.size() / 300;
    std::string segment;
    for (size_t i = 0; i < segments.size(); ++i) {
      if (i % join_size == 1 && i != 0) {
        new_segments.push_back(segment);
        segment = "";
      }
      segment += segments[i] + " ";
    }
    segments = new_segments;
    LOG(ERROR) << "New Segments: " << segments.size();
  }
  LOG(ERROR) << "Splitting Time: " << timer.Elapsed().InMilliseconds() << "ms";
  return segments;
}

#if 0
std::vector<std::string> TextEmbedder::SplitSCSegments(const std::string& text) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  base::ElapsedTimer timer;
  auto segments = base::SplitStringUsingSubstr(
      text, ". ", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

  struct SemanticChunk {
    std::string text = "";
    std::string combined_text = "";
    tflite::task::processor::EmbeddingResult text_embedding;
    tflite::task::processor::EmbeddingResult combined_text_embedding;
    double distance_to_next = 0;
  } sc_segments[segments.size()];
  for (size_t i = 0; i < segments.size(); ++i) {
    // Combine sentences with -1 and +1 window.
    if (sc_segments[i].text.empty()) {
      sc_segments[i].text = segments[i];
      CHECK(EmbedText(sc_segments[i].text, sc_segments[i].text_embedding).ok());
    }
    if (i + 1 < segments.size()) {
    sc_segments[i + 1].text = segments[i + 1];
    }
    if (i - 1 >= 0) {
      sc_segments[i].combined_text = sc_segments[i - 1].text + " " + segments[i];
    } else {
      sc_segments[i].combined_text = segments[i];
    }
    if (i + 1 < segments.size()) {
      sc_segments[i].combined_text += " " + segments[i + 1];
    }

    // Embed combined text.
    CHECK(EmbedText(sc_segments[i].combined_text, sc_segments[i].combined_text_embedding).ok());
  }

  // Calculate distance for combined text.
  for (size_t i = 0; i < segments.size(); ++i) {
    if (i + 1 < segments.size()) {
      sc_segments[i].distance_to_next = 1 - tflite_text_embedder_->CosineSimilarity(
          sc_segments[i].combined_text_embedding.embeddings(0).feature_vector(),
          sc_segments[i + 1].combined_text_embedding.embeddings(0).feature_vector())
          .value();
    }
  }

  // Calculat percentile
  std::vector<size_t> break_points;
  for (size_t i = 0; i < segments.size(); ++i) {
    size_t count = 0;
    for (size_t j = 0; j < segments.size(); ++j) {
      if (sc_segments[i].distance_to_next > sc_segments[j].distance_to_next) {
        count++;
      }
    }
    int percentile = count * 100 / (segments.size() - 1);
    if (percentile > 95) {
      break_points.push_back(i);
    }
  }

  LOG(ERROR) << "Break Points: " << break_points.size();
  LOG(ERROR) << "Semantic Chuncking Splitting Time: "
             << timer.Elapsed().InMilliseconds() << "ms";
  return {};
}
#endif

absl::Status TextEmbedder::EmbedText(
    const std::string& text,
    tflite::task::processor::EmbeddingResult& embedding) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  auto maybe_embedding = tflite_text_embedder_->Embed(text);
  if (!maybe_embedding.ok()) {
    return maybe_embedding.status();
  }
  embedding = maybe_embedding.value();
  return absl::OkStatus();
}

absl::Status TextEmbedder::EmbedSegments() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(embedder_sequence_checker_);
  if (segments_.empty()) {
    return absl::InvalidArgumentError("No segments to embed.");
  }
  LOG(ERROR) << "Embedding...";
  base::ElapsedTimer timer;
  for (const auto& segment : segments_) {
    tflite::task::processor::EmbeddingResult embedding;
    auto status = EmbedText(segment, embedding);
    if (!status.ok()) {
      return status;
    }
    embeddings_.push_back(embedding);
  }
  LOG(ERROR) << "Embedding Time: " << timer.Elapsed().InMilliseconds() << "ms";
  return absl::OkStatus();
}

}  // namespace ai_chat
