// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/views/ai_rewriter/ai_rewriter_button_view.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "brave/browser/ui/ai_rewriter/ai_rewriter_dialog_delegate.h"
#include "brave/components/vector_icons/vector_icons.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"
#include "content/public/browser/page.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "include/core/SkColor.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/models/image_model.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/widget/widget.h"

namespace ai_rewriter {

AIRewriterButtonView::AIRewriterButtonView(content::WebContents* contents)
    : content::WebContentsObserver(contents) {
  views::Builder<AIRewriterButtonView>(this)
      .SetBackground(views::CreateRoundedRectBackground(SK_ColorWHITE, 8))
      .SetBorder(views::CreateRoundedRectBorder(1, 12, SK_ColorGRAY))
      .SetLayoutManager(std::make_unique<views::FillLayout>())
      .AddChild(
          views::Builder<views::MdTextButton>()
              .SetCustomPadding(gfx::Insets::VH(8, 8))
              .SetImageModel(
                  views::LabelButton::ButtonState::STATE_NORMAL,
                  ui::ImageModel::FromVectorIcon(kLeoProductBraveLeoIcon))
              .SetCallback(base::BindRepeating(
                  &AIRewriterButtonView::OpenDialog, base::Unretained(this))))
      .BuildChildren();
}

AIRewriterButtonView::~AIRewriterButtonView() = default;

AIRewriterButtonView* AIRewriterButtonView::MaybeCreateButton(
    content::WebContents* contents) {
  auto* browser = chrome::FindBrowserWithTab(contents);

  // Possible for non-tab WebContents
  if (!browser) {
    return nullptr;
  }

  auto* button = new AIRewriterButtonView(contents);

  auto* parent_widget = views::Widget::GetWidgetForNativeWindow(
      browser->window()->GetNativeWindow());
  CHECK(parent_widget);

  views::Widget::InitParams params(
      views::Widget::InitParams::Type::TYPE_CONTROL);
  params.parent = parent_widget->GetNativeView();
  params.activatable = views::Widget::InitParams::Activatable::kNo;
  params.delegate = button;
  params.shadow_type = views::Widget::InitParams::ShadowType::kDrop;
  params.corner_radius = 12;
  auto* widget = new views::Widget();
  widget->Init(std::move(params));
  widget->SetBounds(
      gfx::Rect(gfx::Point(100, 100), button->GetPreferredSize()));
  widget->Hide();

  return button;
}

void AIRewriterButtonView::Show(const gfx::Rect& rect) {
  CHECK(GetWidget());
  GetWidget()->Show();

  auto* browser_view = BrowserView::GetBrowserViewForBrowser(
      chrome::FindBrowserWithTab(web_contents()));
  CHECK(browser_view);

  auto top = browser_view->toolbar()->bounds().bottom();

  constexpr int kPaddingY = 8;
  constexpr int kPaddingX = 8;

  auto size = GetPreferredSize();
  auto pos = rect.origin();
  pos.Offset(GetPreferredSize().width() / 2 + kPaddingX,
             top - GetPreferredSize().height() / 2 - kPaddingY);
  GetWidget()->SetBounds(gfx::Rect(pos, size));
}

void AIRewriterButtonView::Hide() {
  GetWidget()->Hide();
}

void AIRewriterButtonView::OpenDialog() {
  auto* host = web_contents()->GetFocusedFrame()->GetRenderWidgetHost();
  CHECK(host);

  auto* host_view = host->GetView();
  if (!host_view) {
    return;
  }

  auto selected = host_view->GetSelectedText();
  AIRewriterDialogDelegate::Show(web_contents(), base::UTF16ToUTF8(selected));
}

void AIRewriterButtonView::PrimaryPageChanged(content::Page& page) {
  Hide();
}

void AIRewriterButtonView::WebContentsDestroyed() {
  GetWidget()->Close();
}

BEGIN_METADATA(AIRewriterButtonView)
END_METADATA

}  // namespace ai_rewriter
