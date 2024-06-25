// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_AI_REWRITER_AI_REWRITER_TAB_HELPER_H_
#define BRAVE_BROWSER_AI_REWRITER_AI_REWRITER_TAB_HELPER_H_

#include <optional>
#include "brave/components/ai_rewriter/common/mojom/ai_rewriter.mojom.h"
#include "content/public/browser/visibility.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "ui/gfx/geometry/rect_f.h"

namespace ai_rewriter {
class AIRewriterTabHelper
    : public content::WebContentsUserData<AIRewriterTabHelper>,
      public content::WebContentsObserver,
      public mojom::AIRewriterButton {
 public:
  AIRewriterTabHelper(const AIRewriterTabHelper&) = delete;
  AIRewriterTabHelper& operator=(const AIRewriterTabHelper&) = delete;
  ~AIRewriterTabHelper();

  void Bind(mojo::PendingReceiver<mojom::AIRewriterButton> receiver);

  // content::WebContentsObserver:
  void PrimaryPageChanged(content::Page& page) override;
  void OnVisibilityChanged(content::Visibility visibility) override;

  // mojom::AIRewriterButton:
  void Hide() override;
  void Show(const gfx::RectF& rect) override;

 private:
  struct BindingContext {
    int process_id;
    int frame_id;
  };
  explicit AIRewriterTabHelper(content::WebContents* contents);

  mojo::ReceiverSet<mojom::AIRewriterButton, BindingContext> receivers_;

  friend WebContentsUserData;
  WEB_CONTENTS_USER_DATA_KEY_DECL();
};
}  // namespace ai_rewriter

#endif  // BRAVE_BROWSER_AI_REWRITER_AI_REWRITER_TAB_HELPER_H_
