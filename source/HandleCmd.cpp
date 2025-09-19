#include "HandleCmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <QString>
#include <QMap>
#include <QDebug>
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/TaskNotification.h"
#include "Poco/NotificationCenter.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Exception.h"

#include "interface_to_host.h"
#include "logger_wrapper.h"
#include "notification_pkg.h"
#include "json.hpp"
#include "return_code.h"

using namespace std;
using namespace Poco;

using json = nlohmann::json;

//static HostCmdHandler *instance = NULL;

bool b_simulation_en;

std::string parseCmdPackage(const std::string &pkg)
{
    json json_msg = json::parse(pkg);
    std::string cmd_type = json_msg["cmd"];
    bool b_is_command_supported = true;
    int iReturn = ORS_SUCCESS;

    json json_ack;
    json_ack["id"] = json_msg["id"];
    json_ack["cmd"] = "ack";

    goDriverLogger.Log("DEBUG", "Receive command package:cmd=%s, id=%s", json_msg["cmd"].dump().c_str(), json_msg["id"].dump().c_str());


    if (cmd_type == "dimmer_version") {
        iReturn = ORS_SUCCESS;
        goDriverLogger.Log("debug", "dimmer_version");
    }
    else {
        b_is_command_supported = false;
    }

    if (b_is_command_supported) {
        json_ack["status"] = iReturn;
    }
    else {
        json_ack["status"] = ERR_UNKNOWN_COMMAND;
        json_ack["message"] = "invalid command " + cmd_type;
    }
    std::string str_ack = json_ack.dump();
    goDriverLogger.Log("DEBUG", "parseCmdPackage exit");
    return str_ack;
}
