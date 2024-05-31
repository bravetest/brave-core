/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "ui/base/ui_base_features.h"

namespace features {
bool ShouldUseChromeRefresh2023InProfileMenuViewBase() {
  return false;
}
}  // namespace features

#define IsChromeRefresh2023 ShouldUseChromeRefresh2023InProfileMenuViewBase

#include "src/chrome/browser/ui/views/profiles/profile_menu_view_base.cc"

#undef IsChromeRefresh2023
