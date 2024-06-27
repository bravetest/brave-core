// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/views/ai_rewriter/ai_rewriter_button.h"

#include <memory>

#include "base/functional/bind.h"
#include "brave/browser/ui/ai_rewriter/ai_rewriter_dialog_delegate.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "content/public/browser/page.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "include/core/SkColor.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/layout/layout_types.h"
#include "ui/views/metadata/view_factory.h"
#include "ui/views/widget/widget.h"
#include "util/basictypes.h"

namespace ai_rewriter {

AIRewriterButton::AIRewriterButton(content::WebContents* contents)
    : content::WebContentsObserver(contents) {
  views::Builder<AIRewriterButton>(this)
      .SetLayoutManager(std::make_unique<views::FillLayout>())
      .AddChild(views::Builder<views::MdTextButton>()
                    .SetText(u"Rewrite")
                    .SetCallback(base::BindRepeating(
                        &AIRewriterButton::OpenDialog, base::Unretained(this))))
      .BuildChildren();
}
AIRewriterButton::~AIRewriterButton() = default;

AIRewriterButton* AIRewriterButton::CreateButton(
    content::WebContents* contents) {
  auto* button = new AIRewriterButton(contents);

  auto* browser = chrome::FindBrowserWithTab(contents);
  CHECK(browser);

  auto* parent_widget = views::Widget::GetWidgetForNativeWindow(
      browser->window()->GetNativeWindow());
  CHECK(parent_widget);

  views::Widget::InitParams params(
      views::Widget::InitParams::Type::TYPE_CONTROL);
  params.parent = parent_widget->GetNativeView();
  params.activatable = views::Widget::InitParams::Activatable::kYes;
  params.delegate = button;
  auto* widget = new views::Widget();
  widget->Init(std::move(params));
  widget->SetBounds(
      gfx::Rect(gfx::Point(100, 100), button->GetPreferredSize()));

  return button;
}

void AIRewriterButton::Show(const gfx::Rect& rect) {
  CHECK(GetWidget());
  GetWidget()->Show();

  auto size = GetPreferredSize();
  auto pos = rect.top_right();
  pos.Offset(size.width(), -size.height());
  GetWidget()->SetBounds(gfx::Rect(pos, size));
}

void AIRewriterButton::Hide() {
  GetWidget()->Hide();
}

void AIRewriterButton::OpenDialog() {
  LOG(ERROR) << "Open Dialog";
}

void AIRewriterButton::PrimaryPageChanged(content::Page& page) {
  Hide();
}

void AIRewriterButton::WebContentsDestroyed() {
  GetWidget()->Close();
}

BEGIN_METADATA(AIRewriterButton)
END_METADATA

}  // namespace ai_rewriter
