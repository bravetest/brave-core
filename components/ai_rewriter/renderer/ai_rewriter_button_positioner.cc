// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "brave/components/ai_rewriter/renderer/ai_rewriter_button_positioner.h"

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "base/location.h"
#include "base/task/sequenced_task_runner.h"
#include "base/time/time.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_dom_event.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_node.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/events/event.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/dom/events/native_event_listener.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/editing/dom_selection.h"
#include "third_party/blink/renderer/core/editing/selection_controller.h"
#include "third_party/blink/renderer/core/event_type_names.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/input/event_handler.h"
#include "third_party/blink/renderer/core/page/context_menu_controller.h"
#include "third_party/blink/renderer/platform/heap/member.h"
#include "ui/gfx/geometry/rect.h"

namespace ai_rewriter {

namespace {
// class SelectionChangeEventListener : public blink::NativeEventListener {
//  public:
//   SelectionChangeEventListener(
//       blink::Node* node,
//       base::RepeatingCallback<void(blink::Event*)> handler)
//       : node_(node), handler_(std::move(handler)) {}

//   void Invoke(blink::ExecutionContext*, blink::Event* event) override {
//     handler_.Run(event);
//   }

//   void AddListener() {
//     node_->addEventListener(ev)
//   }

//  private:
//   blink::Member<blink::Node> node_;
//   blink::WebNode::EventType event_type_;
//   base::RepeatingCallback<void(blink::Event*)> handler_;
// };
}  // namespace

// See AutoFillAgent::GetCaretBounds for gfx::Rect

AIRewriterButtonPositioner::AIRewriterButtonPositioner(
    content::RenderFrame* frame)
    : content::RenderFrameObserver(frame) {}

AIRewriterButtonPositioner::~AIRewriterButtonPositioner() {
  LOG(ERROR) << " Destroyed button listener";
}

void AIRewriterButtonPositioner::OnDestruct() {
  delete this;
}

void AIRewriterButtonPositioner::DidCreateDocumentElement() {
  auto document = render_frame()->GetWebFrame()->GetDocument();
  CHECK(!document.IsNull());
  remove_listener_ = document.AddEventListener(
      blink::WebNode::EventType::kSelectionchange,
      base::BindRepeating(
          [](blink::WebDocument document, blink::WebDOMEvent event) {
            if (document.IsNull()) {
              return;
            }

            ;
            LOG(ERROR) << "Selection changed: "
                       << document.GetFrame()
                              ->GetSelectionBoundsRectForTesting()
                              .ToString();
          },
          document));
  LOG(ERROR) << "Created button positioner!";
}

}  // namespace ai_rewriter
