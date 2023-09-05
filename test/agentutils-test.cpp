#include "utils/agentUtils.hpp"
#include <gtest/gtest.h>

TEST( Utilities, TrimSuccess )
{
    string input = "  jhfyrwiu noijocwoir ";
    string result = AgentUtils::trim(input);
    EXPECT_STREQ(result.c_str(), "jhfyrwiu noijocwoir");
    input = " ";
    result = AgentUtils::trim(input);
    EXPECT_STREQ(result.c_str(), "");
    input = "\tuncle tommy  \t";
    result = AgentUtils::trim(input);
    EXPECT_STREQ(result.c_str(), "uncle tommy");
    input = "\t Aunty spotted  ";
    result = AgentUtils::trim(input);
    EXPECT_STREQ(result.c_str(), "Aunty spotted");
}

// int main(int argc, char **argv)
// {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
