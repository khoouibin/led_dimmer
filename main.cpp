#include <QCoreApplication>
#include <QThread>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include "cli.h"
#include "config_settings.h"
#include "logger_wrapper.h"
#include <filesystem>

using namespace std;

int main(int argc, char *argv[])
{
    int iCLI = -1;
    int iReturn;
    string strConfigDir;
    string strConfigPath;
    json json_msg;
    Poco::TaskManager *tm = new Poco::TaskManager();
    tm->start(&goDriverLogger); // 啟動 LoggerWrapper::runTask()

    QCoreApplication a(argc, argv);
    for (int i = 0; i < argc; ++i)
    {
        // std::cout << "  Argument " << i << ": " << argv[i] << std::endl;
        if (i >= 1)
        {
            if (std::string(argv[i]) == "-cli")
            {
                iCLI = 1;
            }
        }
    }

    if (GetSpecifyDirectoryPath("config", &strConfigDir) != 0)
    {
        goDriverLogger.Log("ERROR", "Fail to find the profile directory \"INI\"");
        // goto EXIT_DRIVER;
    }
    strConfigPath = strConfigDir + "/" + "led_dimmer.ini";
    goDriverLogger.Log("INFO", "start led dimming...");
    if (std::filesystem::exists(strConfigPath))
    {
        json_msg.clear();
        iReturn = IniToProfileJson(strConfigPath.c_str(), json_msg);
        if (iReturn == 0)
        {
            string tmp_string;
            tmp_string.clear();
            tmp_string.assign(json_msg.dump());
            goDriverLogger.Log("INFO", tmp_string);
        }
    }

    if (iCLI == 1)
    {
        CommandLineInterface();
    }
    else
    {
        int cnt = 0;
        while (true)
        {
            printf("sleep = %d \n", cnt);
            cnt += 1;
            QThread::msleep(1);
        }
    }
    goDriverLogger.ExitTask();
    QThread::msleep(1000);
    _exit(0); // 要用 _exit() 才能頭也不回的結束. 用 exit(0) 不行.
    return 0;
}
