#include "common.hpp"
#include <gtest/gtest.h>

TEST( Utilities, TrimSuccess )
{
    string input = "  jhfyrwiu noijocwoir ";
    string result = agent_utils::trim(input);
    EXPECT_STREQ(result.c_str(), "jhfyrwiu noijocwoir");
    input = " ";
    result = agent_utils::trim(input);
    EXPECT_STREQ(result.c_str(), "");
    input = "\tuncle tommy  \t";
    result = agent_utils::trim(input);
    EXPECT_STREQ(result.c_str(), "uncle tommy");
    input = "\t Aunty spotted  ";
    result = agent_utils::trim(input);
    EXPECT_STREQ(result.c_str(), "Aunty spotted");
}

// int main(int argc, char **argv)
// {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
