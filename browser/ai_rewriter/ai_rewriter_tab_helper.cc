// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/browser/ai_rewriter/ai_rewriter_tab_helper.h"

#include "brave/browser/ui/views/ai_rewriter/ai_rewriter_button.h"
#include "content/public/browser/page.h"
#include "content/public/browser/visibility.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"

namespace ai_rewriter {

AIRewriterTabHelper::AIRewriterTabHelper(content::WebContents* contents)
    : content::WebContentsUserData<AIRewriterTabHelper>(*contents),
      content::WebContentsObserver(contents) {}

AIRewriterTabHelper::~AIRewriterTabHelper() = default;

void AIRewriterTabHelper::OnVisibilityChanged(content::Visibility visibility) {
  if (visibility == content::Visibility::HIDDEN) {
    Hide();
  }
}

void AIRewriterTabHelper::PrimaryPageChanged(content::Page& page) {
  
}

void AIRewriterTabHelper::Hide() {
  GetButton()->Hide();
}

void AIRewriterTabHelper::Show(const gfx::Rect& rect) {
  GetButton()->Show(rect);
}

ai_rewriter::AIRewriterButton* AIRewriterTabHelper::GetButton() {
  if (!button_) {
    button_ = ai_rewriter::AIRewriterButton::CreateButton(web_contents());
    CHECK(button_);
  }

  return button_;
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(AIRewriterTabHelper);

}  // namespace ai_rewriter
