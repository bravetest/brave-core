// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_PAGE_CONTEXT_MENU_CONTROLLER_H_
#define BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_PAGE_CONTEXT_MENU_CONTROLLER_H_

#include "third_party/blink/renderer/core/core_export.h"

#include "src/third_party/blink/renderer/core/page/context_menu_controller.h"

namespace blink {
static CORE_EXPORT gfx::Rect ComputeSelectionRect(LocalFrame* selected_frame);
}

#endif  // BRAVE_CHROMIUM_SRC_THIRD_PARTY_BLINK_RENDERER_CORE_PAGE_CONTEXT_MENU_CONTROLLER_H_
