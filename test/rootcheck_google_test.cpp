// // #include "service/configservice.hpp"
// #include <iostream>
// #include <gtest/gtest.h>
// #include "agent_utils.hpp"


// struct IniConfigTest : public testing::Test
// {
//     IniConfig * config;
//     // std::map<std::string, std::map<std::string, std::string>> table;
//     void SetUp() { config = new IniConfig();}
//     void TearDown() {delete config;}
// };

// // TEST_F( IniConfigTest, ReadCorrectFileTest ) /* It requires the class which inherits the testinmg::Test*/
// // {
   
// //     string filePath = "/home/krishna/security/Agent/config/agent.config";
// //     int result = config->readConfigFile(filePath, table);
// //     ASSERT_TRUE(result == SUCCESS); //Assert - Fatal, EXPECT - NonFatal
// // }

// // TEST_F( IniConfigTest, ReadInvalidFileTest )
// // {
// //     string filePath = "/home/krishna/surity/Agent/config/agent.config";
// //     int result = config->readConfigFile(filePath, table);
// //     ASSERT_FALSE(result == SUCCESS); //Assert - Fatal, EXPECT - NonFatal
// // }

// // TEST_F(IniConfigTest, testSpaceLine)
// // {
// //     string line = "  mnivcr nimfr\n   ";
// //     string result = config->trim(line);
// //     cout << result << endl;
// //     ASSERT_STREQ(result.c_str(), " mnivcr nimfr");
// // }

// TEST_F(IniConfigTest, validateCorrectText)
// {
//     string line = " sdff" ;
//     bool result = config->validateText(line);
//     EXPECT_TRUE(result == true);
// }

// TEST_F(IniConfigTest, validateFalseText)
// {
//     string line = " sdf;f" ;
//     bool result = config->validateText(line);
//     EXPECT_FALSE(result == true);
// }

// TEST_F(IniConfigTest, validateTrueText)
// {
//     string line = " " ;
//     bool result = config->validateText(line);
//     EXPECT_TRUE(result == true);
// }

// TEST_F(IniConfigTest, validateWrongText)
// {
//     string line = " ;" ;
//     bool result = config->validateText(line);
//     EXPECT_FALSE(result == true);
// }

// // TEST_F(IniConfigTest, to_vector)
// // {

// //     vector<string> result = config->to_vector(columns,sep);
// //     EXPECT_FALSE(result == true);
// // }
// // TEST (Parent, Child) /*Independent test case format*/
// // {
// //     //Arrange { Gather required inputs for the act. }
// //     //Act     { Do the Act }
// //     //Assert  { Validate the Act by Assert }
// // }



// // int main(int argc, char **argv)
// // {
// //     testing::InitGoogleTest(&argc, argv);
// //     return RUN_ALL_TESTS();
// // }