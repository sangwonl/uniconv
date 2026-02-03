#include <gtest/gtest.h>
#include "utils/version_utils.h"

using namespace uniconv::utils;

TEST(VersionUtilsTest, ParseSemver)
{
    auto v = parse_semver("1.2.3");
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->major, 1);
    EXPECT_EQ(v->minor, 2);
    EXPECT_EQ(v->patch, 3);
}

TEST(VersionUtilsTest, ParseSemverTwoParts)
{
    auto v = parse_semver("1.2");
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->major, 1);
    EXPECT_EQ(v->minor, 2);
    EXPECT_EQ(v->patch, 0);
}

TEST(VersionUtilsTest, ParseSemverWithV)
{
    auto v = parse_semver("v1.0.5");
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->major, 1);
    EXPECT_EQ(v->minor, 0);
    EXPECT_EQ(v->patch, 5);
}

TEST(VersionUtilsTest, ParseSemverInvalid)
{
    EXPECT_FALSE(parse_semver("").has_value());
    EXPECT_FALSE(parse_semver("abc").has_value());
    EXPECT_FALSE(parse_semver("1").has_value());
}

TEST(VersionUtilsTest, CompareVersions)
{
    EXPECT_EQ(compare_versions("1.0.0", "1.0.0"), 0);
    EXPECT_EQ(compare_versions("1.0.0", "0.9.0"), 1);
    EXPECT_EQ(compare_versions("0.9.0", "1.0.0"), -1);
    EXPECT_EQ(compare_versions("1.2.3", "1.2.4"), -1);
    EXPECT_EQ(compare_versions("2.0.0", "1.9.9"), 1);
}

TEST(VersionUtilsTest, CompareVersionsInvalid)
{
    EXPECT_EQ(compare_versions("abc", "1.0.0"), 0);
    EXPECT_EQ(compare_versions("1.0.0", "abc"), 0);
}

TEST(VersionUtilsTest, SatisfiesConstraintGte)
{
    EXPECT_TRUE(satisfies_constraint("1.0.0", ">=1.0.0"));
    EXPECT_TRUE(satisfies_constraint("1.1.0", ">=1.0.0"));
    EXPECT_TRUE(satisfies_constraint("2.0.0", ">=1.0.0"));
    EXPECT_FALSE(satisfies_constraint("0.9.0", ">=1.0.0"));
}

TEST(VersionUtilsTest, SatisfiesConstraintGt)
{
    EXPECT_FALSE(satisfies_constraint("1.0.0", ">1.0.0"));
    EXPECT_TRUE(satisfies_constraint("1.0.1", ">1.0.0"));
}

TEST(VersionUtilsTest, SatisfiesConstraintLte)
{
    EXPECT_TRUE(satisfies_constraint("1.0.0", "<=1.0.0"));
    EXPECT_TRUE(satisfies_constraint("0.9.0", "<=1.0.0"));
    EXPECT_FALSE(satisfies_constraint("1.0.1", "<=1.0.0"));
}

TEST(VersionUtilsTest, SatisfiesConstraintLt)
{
    EXPECT_FALSE(satisfies_constraint("1.0.0", "<1.0.0"));
    EXPECT_TRUE(satisfies_constraint("0.9.9", "<1.0.0"));
}

TEST(VersionUtilsTest, SatisfiesConstraintEq)
{
    EXPECT_TRUE(satisfies_constraint("1.0.0", "=1.0.0"));
    EXPECT_TRUE(satisfies_constraint("1.0.0", "==1.0.0"));
    EXPECT_FALSE(satisfies_constraint("1.0.1", "=1.0.0"));
}

TEST(VersionUtilsTest, SatisfiesConstraintEmpty)
{
    EXPECT_TRUE(satisfies_constraint("1.0.0", ""));
}

TEST(VersionUtilsTest, SatisfiesConstraintWithSpace)
{
    EXPECT_TRUE(satisfies_constraint("1.0.0", ">= 1.0.0"));
}
