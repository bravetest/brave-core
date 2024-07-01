/* Copyright (c) 2024 The Brave Authors. All rights reserved.
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/split_view/split_view.h"

#include "brave/browser/ui/tabs/features.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "brave/browser/ui/tabs/split_view_browser_data.h"

SplitView::SplitView() {
  CHECK(base::FeatureList::IsEnabled(tabs::features::kBraveSplitView));

    SplitViewBrowserData::CreateForBrowser(browser_.get());

    auto devtools_web_view =
        std::make_unique<views::WebView>(browser_->profile());
    devtools_web_view->SetVisible(false);
    auto contents_web_view =
        std::make_unique<ActivatableContentsWebView>(browser_->profile());
    contents_web_view->SetVisible(false);

    secondary_devtools_web_view_ =
        contents_container_->AddChildView(std::move(devtools_web_view));
    secondary_contents_web_view_ =
        contents_container_->AddChildView(std::move(contents_web_view));
    split_view_separator_ = contents_container_->AddChildView(
        std::make_unique<SplitViewSeparator>(browser_.get()));

    auto* contents_layout_manager = static_cast<BraveContentsLayoutManager*>(
        contents_container()->GetLayoutManager());
    contents_layout_manager->set_secondary_contents_view(
        secondary_contents_web_view_);
    contents_layout_manager->set_secondary_devtools_view(
        secondary_devtools_web_view_);
    contents_layout_manager->SetSplitViewSeparator(split_view_separator_);

    auto* split_view_browser_data =
        SplitViewBrowserData::FromBrowser(browser_.get());
    contents_layout_manager->set_split_view_browser_data(
        split_view_browser_data);
    split_view_observation_.Observe(split_view_browser_data);
}

SplitView::~SplitView() = default;

BEGIN_METADATA(SplitView)
END_METADATA
