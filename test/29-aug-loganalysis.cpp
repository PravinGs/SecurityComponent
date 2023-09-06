#include "service/loganalysis.hpp"
#include <gtest/gtest.h>
#include "service/configservice.hpp"
#include "agentUtils.hpp"

struct LogAnalysisTest : public testing::Test
{
    LogAnalysis *analysis;
    void SetUp() { analysis = new LogAnalysis(); }
    void TearDown() { delete analysis; }
};

TEST_F(LogAnalysisTest, SyslogCheck)
{
    string log = "Aug 22 18:09:37 ubuntu-20 kernel: [18374.181445] audit: type=1400 audit(1692707977.617:2879):";
    string convertTime;
    string format = "syslog";
    log_event logInfo = analysis->decodeLog(log, format);
    int result = AgentUtils::convertTimeFormat("Aug 22 18:09:37", convertTime);
    ASSERT_TRUE(logInfo.size > 0);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_STREQ(convertTime.c_str(), logInfo.timestamp.c_str());
    EXPECT_STREQ("ubuntu-20", logInfo.user.c_str());
    EXPECT_STREQ("kernel:", logInfo.program.c_str());
}

TEST_F(LogAnalysisTest, FailSyslogCheck)
{
    string log = "Aug 22 9:37 ubuntu-20 kernel: [18374.181445] audit: type=1400 audit(1692707977.617:2879):";
    string convertTime;
    string format = "syslog";
    log_event logInfo = analysis->decodeLog(log, format);
    int result = AgentUtils::convertTimeFormat("Aug 22 9:37", convertTime);
    ASSERT_EQ(result, FAILED);
    cout << "LogInfo size : " << logInfo.size << "\n";
    EXPECT_TRUE(logInfo.size == 0);
}

TEST_F(LogAnalysisTest, CheckConfigFile)
{
    string configFile = "/home/krishna/security/Agent/rules";
    analysis->setConfigFile(configFile, "");
    auto size = analysis->_rules.size();
    ASSERT_TRUE( size > 0); // Assert - Fatal, EXPECT - NonFatal
}

TEST_F(LogAnalysisTest, CheckCorrectConfigFile)
{
    string configFile = "/home/krishna/curity/Agent/rules";
    analysis->setConfigFile(configFile, "");
    auto size = analysis->_rules.size();
    ASSERT_TRUE(size == 0); // Assert - Fatal, EXPECT - NonFatal
}

TEST_F(LogAnalysisTest, CheckInvaidConfigFile)
{
    string configFile = "";
    analysis->setConfigFile(configFile, "");
    auto size = analysis->_rules.size();
    ASSERT_TRUE(size == 0); // Assert - Fatal, EXPECT - NonFatal
}

TEST_F(LogAnalysisTest, dpkgLogCheck)
{
    string log = "2023-08-22 12:32:20 status installed systemd:amd64 245.4-4ubuntu3.22";
    string format = "dpkg";
    log_event logInfo = analysis->decodeLog(log, format);    
    ASSERT_TRUE(logInfo.size > 0);
    EXPECT_STREQ("status", logInfo.program.c_str());
}

TEST_F(LogAnalysisTest, InvaliddpkgLogCheck)
{
    string log = "2023-08-22 32:20  status installed systemd:amd64 245.4-4ubuntu3.22";
    string convertTime;
    string format = "dpkg";
    log_event logInfo = analysis->decodeLog(log, format);
    EXPECT_TRUE(logInfo.size == 0);
}

TEST_F(LogAnalysisTest, AuthlogCheck)
{
    string log = "Mar 21 18:13:07 ubuntu-20 sudo: pam_unix(sudo:session): session opened for user root by (uid=0):";
    string convertTime;
    string format = "authlog";
    log_event logInfo = analysis->decodeLog(log, format);
    int result = AgentUtils::convertTimeFormat("Mar 21 18:13:07", convertTime);
    EXPECT_TRUE(logInfo.size > 0);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_STREQ(convertTime.c_str(), logInfo.timestamp.c_str());
    EXPECT_STREQ("ubuntu-20", logInfo.user.c_str());
}

TEST_F(LogAnalysisTest, InvalidAuthlogCheck)
{
    string log = "Mar 21 18:13 ubuntu-20 sudo: pam_unix(sudo:session): session opened for user root by (uid=0):";
    string convertTime;
    string format = "authlog";
    log_event logInfo = analysis->decodeLog(log, format);
    EXPECT_TRUE(logInfo.size == 0);
}

TEST_F(LogAnalysisTest, Report)
{
    analysis->setConfigFile("/home/krishna/security/Agent/config/test-rules.xml", "");
    //Arrange, Act, Assert
    vector<string> logs = {
        "Aug 22 18:09:37 ubuntu-20 kernel: [18374.181445] audit: type=1400 audit(1692707977.617:2879):",
        "Aug 22 18:09:37 ubuntu-20 nk: [18374.181445] audit: type=1400 audit(1692707977.617:2879):",
        "Aug 22 18:09:37 ubuntu-20 kernvrwel: [18374.181445udit: type=1400 audit(1692707977.617:2879):",
        "Aug 22 18:09:37 ubuntu-20 lnel: [18374.181445] audit: type=1400 audit(1692707977.617:2879):"
    };

    vector<string> ruleInputs = {"124,syslog", "456,syslog", "768,syslog","657,syslog"};
    vector<log_event> alerts;

    for (string l: logs)
    {
        log_event event = analysis->decodeLog(l, "syslog");
        int size = event.size;
        EXPECT_TRUE(size > 0);
        alerts.push_back(event);
    };

    for (int i = 0; i < (int)alerts.size(); i++)
    {
        string rule = ruleInputs[i];
        int ruleId = std::stoi(rule.substr(0, rule.find_first_of(',')));
        cout << ruleId << "\n";
        string group = rule.substr(rule.find_first_of(',') + 1);
        cout << group << "\n";
        alerts[i].rule_id = ruleId;
        alerts[i].group = group;
    }

    // 
    int result = analysis->postAnalysis(alerts);

    // 
    EXPECT_EQ(result, SUCCESS);
}
// TEST_F(LogAnalysisTest, analyzeFile1)
// {
//     string file = "/home/krishna/security/Agent/rules";
//     int result = analysis->analyseFile(file);
//     ASSERT_TRUE(result == SUCCESS); // Assert - Fatal, EXPECT - NonFatal
// }

// TEST_F(LogAnalysisTest, analyzeFile2)
// {
//     string file = "/home/krishna/security/Agent/rules/syslog_rules.xml";
//     int result = analysis->analyseFile(file);
//     ASSERT_TRUE(result == SUCCESS); // Assert - Fatal, EXPECT - NonFatal
// }

// TEST_F(LogAnalysisTest, analyzeFile3)
// {
//     string file = "/home/krishna/security/Agent/rules";
//     int result = analysis->analyseFile(file);
//     ASSERT_TRUE(result == SUCCESS); // Assert - Fatal, EXPECT - NonFatal
// }

// TEST_F(LogAnalysisTest, analyzeFile4)
// {
//     string file = "/home/krishna/security/Agent/rules";
//     int result = analysis->analyseFile(file);
//     ASSERT_TRUE(result == SUCCESS); // Assert - Fatal, EXPECT - NonFatal
// }
