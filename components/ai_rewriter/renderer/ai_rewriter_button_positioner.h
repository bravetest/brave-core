// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_AI_REWRITER_RENDERER_AI_REWRITER_BUTTON_POSITIONER_H_
#define BRAVE_COMPONENTS_AI_REWRITER_RENDERER_AI_REWRITER_BUTTON_POSITIONER_H_

#include "base/component_export.h"
#include "base/functional/callback_helpers.h"
#include "content/public/renderer/render_frame_observer.h"
#include "third_party/blink/public/web/web_element.h"

namespace content {
class RenderFrame;
}

namespace ai_rewriter {

class COMPONENT_EXPORT(AI_REWRITER_RENDERER) AIRewriterButtonPositioner
    : public content::RenderFrameObserver {
 public:
  explicit AIRewriterButtonPositioner(content::RenderFrame* frame);
  AIRewriterButtonPositioner(const AIRewriterButtonPositioner&) = delete;
  AIRewriterButtonPositioner& operator=(const AIRewriterButtonPositioner&) =
      delete;
  ~AIRewriterButtonPositioner() override;

 protected:
  // content::RenderFrameObserver:
  void DidCreateDocumentElement() override;
  void OnDestruct() override;

 private:
  base::ScopedClosureRunner remove_listener_;
};

}  // namespace ai_rewriter

#endif  // BRAVE_COMPONENTS_AI_REWRITER_RENDERER_AI_REWRITER_BUTTON_POSITIONER_H_
