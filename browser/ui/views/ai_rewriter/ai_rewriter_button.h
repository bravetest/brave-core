// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_VIEWS_AI_REWRITER_AI_REWRITER_BUTTON_H_
#define BRAVE_BROWSER_UI_VIEWS_AI_REWRITER_AI_REWRITER_BUTTON_H_

#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

namespace ai_rewriter {
class AIRewriterButton : public views::WidgetDelegateView,
                         public content::WebContentsObserver {
  METADATA_HEADER(AIRewriterButton, views::WidgetDelegateView)

 public:
  explicit AIRewriterButton(content::WebContents* contents);
  AIRewriterButton(const AIRewriterButton&) = delete;
  AIRewriterButton& operator=(const AIRewriterButton&) = delete;
  ~AIRewriterButton() override;

  static AIRewriterButton* CreateButton(content::WebContents* contents);

  void Show(const gfx::Rect& rect);
  void Hide();

  void OpenDialog();

  // content::WebContentsObserver:
  void WebContentsDestroyed() override;
  void PrimaryPageChanged(content::Page& page) override;
};

BEGIN_VIEW_BUILDER(/*no export*/, AIRewriterButton, views::WidgetDelegateView)
END_VIEW_BUILDER

}  // namespace ai_rewriter

DEFINE_VIEW_BUILDER(/*no export*/, ai_rewriter::AIRewriterButton)

#endif  // BRAVE_BROWSER_UI_VIEWS_AI_REWRITER_AI_REWRITER_BUTTON_H_
