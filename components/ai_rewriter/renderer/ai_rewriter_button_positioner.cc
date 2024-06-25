// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/ai_rewriter/renderer/ai_rewriter_button_positioner.h"

#include "third_party/blink/renderer/core/editing/dom_selection.h"
#include "third_party/blink/renderer/core/editing/selection_controller.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/input/event_handler.h"
#include "third_party/blink/renderer/core/page/context_menu_controller.h"
#include "ui/gfx/geometry/rect.h"

namespace ai_rewriter {

AIRewriterButtonPositioner::AIRewriterButtonPositioner(
    blink::LocalFrame* frame) {
  frame->GetEventHandler().GetSelectionController();
}
AIRewriterButtonPositioner::~AIRewriterButtonPositioner() = default;

gfx::Rect AIRewriterButtonPositioner::GetSelectionRect(
    blink::LocalFrame* frame) {
  auto* document = frame->GetDocument();
  auto* selection = document->GetSelection();
  auto event = AtomicString("selectionchange");
  document->GetEventListeners(event)->Append(const U *, wtf_size_t)

  // TODO: Use the logic from ContextMenuController ComputeSelectionRect
  return gfx::Rect();
}

}  // namespace ai_rewriter
