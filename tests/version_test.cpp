#include "tradevault/version.hpp"

#include <gtest/gtest.h>

TEST(VersionTest, ReturnsNonEmptySemanticVersion) {
    const auto v = tradevault::version();
    ASSERT_FALSE(v.empty());
    EXPECT_NE(v.find('.'), std::string::npos);
}