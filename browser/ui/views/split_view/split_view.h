/* Copyright (c) 2024 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_VIEWS_SPLIT_VIEW_SPLIT_VIEW_H_
#define BRAVE_BROWSER_UI_VIEWS_SPLIT_VIEW_SPLIT_VIEW_H_

#include <vector>

#include "base/functional/callback_forward.h"
#include "base/memory/weak_ptr.h"
#include "base/types/pass_key.h"
#include "brave/browser/ui/tabs/split_view_browser_data.h"
#include "brave/browser/ui/tabs/split_view_browser_data_observer.h"
#include "brave/browser/ui/views/split_view/split_view_layout_manager.h"
#include "chrome/browser/ui/tabs/tab_model.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

namespace content {
class WebContents;
}  // namespace content

namespace views {
class WebView;
}  // namespace views

class BraveBrowserView;
class Browser;
class ContentsWebView;
class DevToolsContentsResizingStrategy;
class SplitViewLayoutManager;
class SplitViewSeparator;

// Contains a pair of contents container view.
class SplitView : public views::View, public SplitViewBrowserDataObserver {
  METADATA_HEADER(SplitView, views::View)
 public:
  using BrowserViewKey = base::PassKey<BraveBrowserView>;

  SplitView(Browser* browser,
            views::View* contents_container,
            ContentsWebView* contents_web_view);

  ~SplitView() override;

  // Called that must be called by BraveBrowserView. This must be called before
  // BrowserView swaps active web contents, we don't observe tab strip model
  // directly. returns callback that should be called after swapping active web
  // contents
  using AfterSwapWebContents =
      base::OnceCallback<void(content::WebContents* old_contents,
                              content::WebContents* new_contents)>;
  [[nodiscard]] AfterSwapWebContents WillSwapActiveWebContents(
      BrowserViewKey,
      content::WebContents* new_contents,
      int index);

  // Fills secondary web views if accessible.
  void GetAccessiblePanes(BrowserViewKey, std::vector<views::View*>* panes);

  // Update dev tools
  void UpdateSecondaryDevtoolsLayoutAndVisibility();

  auto secondary_contents_web_view() { return secondary_contents_web_view_; }
  auto secondary_devtools_web_view() { return secondary_devtools_web_view_; }

  // Sets the contents resizing strategy.
  void SetSecondaryContentsResizingStrategy(
      const DevToolsContentsResizingStrategy& strategy);

  // views::View:
  void OnThemeChanged() override;

  // SplitViewBrowserDataObserver:
  void OnTileTabs(const SplitViewBrowserData::Tile& tile) override;
  void OnWillBreakTile(const SplitViewBrowserData::Tile& tile) override;
  void OnSwapTabsInTile(const SplitViewBrowserData::Tile& tile) override;

 private:
  void DidSwapActiveWebContents(bool need_to_reset_fast_resize,
                                content::WebContents* old_contents,
                                content::WebContents* new_contents);

  tabs::TabHandle GetActiveTabHandle() const;
  bool IsActiveWebContentsTiled(const SplitViewBrowserData::Tile& tile) const;
  void UpdateSplitViewSizeDelta(content::WebContents* old_contents,
                                content::WebContents* new_contents);
  void UpdateContentsWebViewVisual();
  void UpdateContentsWebViewBorder();
  void UpdateSecondaryContentsWebViewVisibility();

  SplitViewLayoutManager* GetSplitViewLayoutManager();
  const SplitViewLayoutManager* GetSplitViewLayoutManager() const;

  raw_ptr<Browser> browser_ = nullptr;

  raw_ptr<views::View> contents_container_ = nullptr;
  raw_ptr<views::WebView> contents_web_view_ = nullptr;

  raw_ptr<views::View> secondary_contents_container_ = nullptr;
  raw_ptr<views::WebView> secondary_devtools_web_view_ = nullptr;
  raw_ptr<ContentsWebView> secondary_contents_web_view_ = nullptr;

  raw_ptr<SplitViewSeparator> split_view_separator_ = nullptr;

  base::ScopedObservation<SplitViewBrowserData, SplitViewBrowserDataObserver>
      split_view_observation_{this};

  base::WeakPtrFactory<SplitView> weak_ptr_factory_{this};
};

#endif  // BRAVE_BROWSER_UI_VIEWS_SPLIT_VIEW_SPLIT_VIEW_H_
