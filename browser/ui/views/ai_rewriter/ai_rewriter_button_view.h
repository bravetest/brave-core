// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_VIEWS_AI_REWRITER_AI_REWRITER_BUTTON_VIEW_H_
#define BRAVE_BROWSER_UI_VIEWS_AI_REWRITER_AI_REWRITER_BUTTON_VIEW_H_

#include "base/memory/weak_ptr.h"
#include "brave/browser/ui/ai_rewriter/ai_rewriter_dialog_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

namespace ai_rewriter {
class AIRewriterButtonView : public views::WidgetDelegateView,
                             public content::WebContentsObserver {
  METADATA_HEADER(AIRewriterButtonView, views::WidgetDelegateView)

 public:
  explicit AIRewriterButtonView(content::WebContents* contents);
  AIRewriterButtonView(const AIRewriterButtonView&) = delete;
  AIRewriterButtonView& operator=(const AIRewriterButtonView&) = delete;
  ~AIRewriterButtonView() override;

  static base::WeakPtr<AIRewriterButtonView> MaybeCreateButton(
      content::WebContents* contents);

  void Show(const gfx::Rect& rect);
  void Hide();

  AIRewriterDialogDelegate* OpenDialog();

  base::WeakPtr<AIRewriterButtonView> GetWeakPtr();

  // content::WebContentsObserver:
  void WebContentsDestroyed() override;
  void PrimaryPageChanged(content::Page& page) override;

 private:
  int top_offset_;

  base::WeakPtrFactory<AIRewriterButtonView> weak_ptr_factory_{this};
};

BEGIN_VIEW_BUILDER(/*no export*/,
                   AIRewriterButtonView,
                   views::WidgetDelegateView)
END_VIEW_BUILDER

}  // namespace ai_rewriter

DEFINE_VIEW_BUILDER(/*no export*/, ai_rewriter::AIRewriterButtonView)

#endif  // BRAVE_BROWSER_UI_VIEWS_AI_REWRITER_AI_REWRITER_BUTTON_VIEW_H_
