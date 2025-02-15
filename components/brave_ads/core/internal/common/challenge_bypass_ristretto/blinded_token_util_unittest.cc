/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/common/challenge_bypass_ristretto/blinded_token_util.h"

#include "brave/components/brave_ads/core/internal/common/challenge_bypass_ristretto/blinded_token.h"
#include "brave/components/brave_ads/core/internal/common/challenge_bypass_ristretto/blinded_token_unittest_util.h"
#include "brave/components/brave_ads/core/internal/common/challenge_bypass_ristretto/token.h"
#include "brave/components/brave_ads/core/internal/common/challenge_bypass_ristretto/token_unittest_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads::cbr {

TEST(BraveAdsBlindedTokenUtilTest, BlindTokens) {
  // Arrange
  const std::vector<Token> tokens = test::GetTokens();

  // Act
  const std::vector<BlindedToken> blinded_tokens = BlindTokens(tokens);

  // Assert
  EXPECT_EQ(test::GetBlindedTokens(), blinded_tokens);
}

TEST(BraveAdsBlindedTokenUtilTToUnblindedTokensest, BlindEmptyTokens) {
  // Act & Assert
  EXPECT_THAT(BlindTokens({}), ::testing::IsEmpty());
}

TEST(BraveAdsBlindedTokenUtilTest, BlindedTokensToRawBlindedTokens) {
  // Arrange
  const std::vector<BlindedToken> blinded_tokens = test::GetBlindedTokens();

  // Act
  const std::vector<challenge_bypass_ristretto::BlindedToken>
      raw_blinded_tokens = ToRawBlindedTokens(blinded_tokens);

  // Assert
  std::vector<challenge_bypass_ristretto::BlindedToken>
      expected_raw_blinded_tokens;
  expected_raw_blinded_tokens.reserve(blinded_tokens.size());
  for (const auto& blinded_token : blinded_tokens) {
    expected_raw_blinded_tokens.push_back(blinded_token.get());
  }
  EXPECT_EQ(expected_raw_blinded_tokens, raw_blinded_tokens);
}

TEST(BraveAdsBlindedTokenUtilTest, EmptyBlindedTokensToRawBlindedTokens) {
  // Act & Assert
  EXPECT_THAT(ToRawBlindedTokens({}), ::testing::IsEmpty());
}

}  // namespace brave_ads::cbr
