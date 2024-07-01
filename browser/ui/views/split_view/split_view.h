/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_VIEWS_SPLIT_VIEW_SPLIT_VIEW_H_
#define BRAVE_BROWSER_UI_VIEWS_SPLIT_VIEW_SPLIT_VIEW_H_

#include "brave/browser/ui/views/split_view/split_view_separator_delegate.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

namespace views {
class WebView;
}  // namespace views

class ContentsWebView;
class SplitViewSeparator;

// Contains a pair of contents container view.
class SplitView : public views::View,
                  public SplitViewSeparatorDelegate {
 METADATA_HEADER(SplitView, views::View)
 public:
  SplitView();
  ~SplitView() override;

 private:
  raw_ptr<views::WebView> secondary_devtools_web_view_ = nullptr;
  raw_ptr<ContentsWebView> secondary_contents_web_view_ = nullptr;
  raw_ptr<SplitViewSeparator> split_view_separator_ = nullptr;
};

#endif  // BRAVE_BROWSER_UI_VIEWS_SPLIT_VIEW_SPLIT_VIEW_H_
