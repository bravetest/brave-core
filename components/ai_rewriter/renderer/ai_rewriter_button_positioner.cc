// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/ai_rewriter/renderer/ai_rewriter_button_positioner.h"

#include "base/check.h"
#include "base/functional/bind.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_dom_event.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_node.h"
#include "third_party/blink/public/web/web_range.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/editing/dom_selection.h"
#include "third_party/blink/renderer/core/editing/frame_selection.h"
#include "third_party/blink/renderer/core/editing/selection_controller.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/input/event_handler.h"
#include "third_party/blink/renderer/core/layout/geometry/physical_rect.h"
#include "third_party/blink/renderer/core/page/context_menu_controller.h"
#include "ui/gfx/geometry/rect.h"

namespace ai_rewriter {

AIRewriterButtonPositioner::AIRewriterButtonPositioner(
    content::RenderFrame* frame)
    : content::RenderFrameObserver(frame) {
  frame->GetRemoteAssociatedInterfaces()->GetInterface(&button_);
}

AIRewriterButtonPositioner::~AIRewriterButtonPositioner() = default;

void AIRewriterButtonPositioner::OnDestruct() {
  delete this;
}

void AIRewriterButtonPositioner::DidCreateDocumentElement() {
  auto document = render_frame()->GetWebFrame()->GetDocument();
  CHECK(!document.IsNull());
  remove_listener_ = document.AddEventListener(
      blink::WebNode::EventType::kSelectionchange,
      base::BindRepeating(&AIRewriterButtonPositioner::UpdateButton,
                          weak_ptr_factory_.GetWeakPtr(), document));
}

void AIRewriterButtonPositioner::DidChangeScrollOffset() {
  UpdateButton(render_frame()->GetWebFrame()->GetDocument(),
               blink::WebDOMEvent());
}

void AIRewriterButtonPositioner::UpdateButton(blink::WebDocument document,
                                              blink::WebDOMEvent event) {
  if (document.IsNull()) {
    return;
  }

  auto* frame = document.GetFrame();
  auto selection = frame->SelectionRange();

  // Backwards selections have negative length, so use abs
  if (abs(selection.length()) < 2) {
    // Hide button, not much is selected
    button_->Hide();
    return;
  }

  // Only show the button when editable text is selected.
  auto focused = frame->GetDocument().FocusedElement();
  if (!focused || !focused.IsEditable()) {
    button_->Hide();
    return;
  }

  auto* local_frame =
      blink::LocalFrame::FromFrameToken(frame->GetLocalFrameToken());
  auto selection_bounds = blink::ToPixelSnappedRect(
      local_frame->Selection().AbsoluteUnclippedBounds());

  // If nothing is selected, hide the button.
  if (selection_bounds.IsEmpty()) {
    button_->Hide();
    return;
  }

  selection_bounds = local_frame->View()->FrameToViewport(selection_bounds);

  auto viewport_bounds =
      render_frame()->ConvertViewportToWindow(selection_bounds);
  button_->Show(viewport_bounds);
}

}  // namespace ai_rewriter
