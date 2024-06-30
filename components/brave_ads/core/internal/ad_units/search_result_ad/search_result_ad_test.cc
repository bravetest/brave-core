/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "base/test/mock_callback.h"
#include "base/test/scoped_feature_list.h"
#include "brave/components/brave_ads/core/internal/ad_units/search_result_ad/search_result_ad_handler.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/creatives/search_result_ads/creative_search_result_ad_unittest_util.h"
#include "brave/components/brave_ads/core/internal/serving/permission_rules/permission_rules_unittest_util.h"
#include "brave/components/brave_ads/core/internal/settings/settings_unittest_util.h"
#include "brave/components/brave_ads/core/public/ads_feature.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads {

class BraveAdsSearchResultAdIntegrationTest : public UnitTestBase {
 protected:
  void SetUp() override {
    UnitTestBase::SetUp(/*is_integration_test=*/true);

    test::ForcePermissionRules();
  }

  void SetUpMocks() override {
    EXPECT_CALL(ads_client_mock_, RecordP2AEvents).Times(0);
  }

  void TriggerSearchResultAdEventAndVerifiyExpectations(
      mojom::CreativeSearchResultAdInfoPtr mojom_creative_ad,
      const mojom::SearchResultAdEventType event_type,
      const bool should_fire_event) {
    base::MockCallback<TriggerAdEventCallback> callback;
    EXPECT_CALL(callback, Run(/*success=*/should_fire_event));
    GetAds().TriggerSearchResultAdEvent(std::move(mojom_creative_ad),
                                        event_type, callback.Get());
  }
};

TEST_F(BraveAdsSearchResultAdIntegrationTest, TriggerViewedEvents) {
  // Arrange
  const base::test::ScopedFeatureList scoped_feature_list(
      kShouldAlwaysTriggerBraveSearchResultAdEventsFeature);

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  TriggerSearchResultAdEventAndVerifiyExpectations(
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);
}

TEST_F(BraveAdsSearchResultAdIntegrationTest, TriggerQueuedViewedEvents) {
  // Arrange
  const base::test::ScopedFeatureList scoped_feature_list(
      kShouldAlwaysTriggerBraveSearchResultAdEventsFeature);

  SearchResultAdHandler::DeferTriggeringOfAdViewedEventForTesting();

  TriggerSearchResultAdEventAndVerifiyExpectations(
      // This viewed impression ad event triggering will be deferred.
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      // This viewed impression ad event will be queued as the previous ad
      // viewed impression event has not completed.
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  SearchResultAdHandler::TriggerDeferredAdViewedEventForTesting();
}

TEST_F(BraveAdsSearchResultAdIntegrationTest, TriggerClickedEvent) {
  // Arrange
  const base::test::ScopedFeatureList scoped_feature_list(
      kShouldAlwaysTriggerBraveSearchResultAdEventsFeature);

  const mojom::CreativeSearchResultAdInfoPtr mojom_creative_ad =
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true);

  TriggerSearchResultAdEventAndVerifiyExpectations(
      mojom_creative_ad.Clone(),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      mojom_creative_ad.Clone(), mojom::SearchResultAdEventType::kClicked,
      /*should_fire_event=*/true);
}

TEST_F(BraveAdsSearchResultAdIntegrationTest,
       TriggerViewedEventsForNonRewardsUser) {
  // Arrange
  const base::test::ScopedFeatureList scoped_feature_list(
      kShouldAlwaysTriggerBraveSearchResultAdEventsFeature);

  test::DisableBraveRewards();

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  TriggerSearchResultAdEventAndVerifiyExpectations(
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);
}

TEST_F(
    BraveAdsSearchResultAdIntegrationTest,
    DoNotTriggerViewedEventIfShouldNotAlwaysTriggerAdEventsForNonRewardsUser) {
  // Arrange
  test::DisableBraveRewards();

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/false);
}

TEST_F(BraveAdsSearchResultAdIntegrationTest,
       TriggerQueuedViewedEventsForNonRewardsUser) {
  // Arrange
  const base::test::ScopedFeatureList scoped_feature_list(
      kShouldAlwaysTriggerBraveSearchResultAdEventsFeature);

  test::DisableBraveRewards();

  SearchResultAdHandler::DeferTriggeringOfAdViewedEventForTesting();

  TriggerSearchResultAdEventAndVerifiyExpectations(
      // This ad viewe impressiond event triggering will be deferred.
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      // This viewed impression ad event will be queued as the previous ad
      // viewed impression event has not completed.
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  SearchResultAdHandler::TriggerDeferredAdViewedEventForTesting();
}

TEST_F(BraveAdsSearchResultAdIntegrationTest,
       TriggerClickedEventForNonRewardsUser) {
  // Arrange
  const base::test::ScopedFeatureList scoped_feature_list(
      kShouldAlwaysTriggerBraveSearchResultAdEventsFeature);

  test::DisableBraveRewards();

  const mojom::CreativeSearchResultAdInfoPtr mojom_creative_ad =
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true);

  TriggerSearchResultAdEventAndVerifiyExpectations(
      mojom_creative_ad.Clone(),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/true);

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      mojom_creative_ad.Clone(), mojom::SearchResultAdEventType::kClicked,
      /*should_fire_event=*/true);
}

TEST_F(
    BraveAdsSearchResultAdIntegrationTest,
    DoNotTriggerClickedEventIfShouldNotAlwaysTriggerAdEventsForNonRewardsUser) {
  // Arrange
  test::DisableBraveRewards();

  const mojom::CreativeSearchResultAdInfoPtr mojom_creative_ad =
      test::BuildCreativeSearchResultAd(/*should_generate_random_uuids=*/true);

  TriggerSearchResultAdEventAndVerifiyExpectations(
      mojom_creative_ad.Clone(),
      mojom::SearchResultAdEventType::kViewedImpression,
      /*should_fire_event=*/false);

  // Act & Assert
  TriggerSearchResultAdEventAndVerifiyExpectations(
      mojom_creative_ad.Clone(), mojom::SearchResultAdEventType::kClicked,
      /*should_fire_event=*/false);
}

}  // namespace brave_ads
