// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_AI_REWRITER_RENDERER_AI_REWRITER_BUTTON_POSITIONER_H_
#define BRAVE_COMPONENTS_AI_REWRITER_RENDERER_AI_REWRITER_BUTTON_POSITIONER_H_

#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "ui/accessibility/ax_tree_observer.h"
#include "ui/gfx/geometry/rect.h"
namespace ai_rewriter {

class AIRewriterButtonPositioner : public ui::AXTreeObserver {
 public:
  AIRewriterButtonPositioner(blink::LocalFrame* frame);
  AIRewriterButtonPositioner(const AIRewriterButtonPositioner&) = delete;
  AIRewriterButtonPositioner& operator=(const AIRewriterButtonPositioner&) =
      delete;
  ~AIRewriterButtonPositioner();

  gfx::Rect GetSelectionRect(blink::LocalFrame* frame);
};

}  // namespace ai_rewriter

#endif  // BRAVE_COMPONENTS_AI_REWRITER_RENDERER_AI_REWRITER_BUTTON_POSITIONER_H_
