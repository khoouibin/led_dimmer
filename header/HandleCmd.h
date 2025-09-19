#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <iostream>

#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/TaskNotification.h"
#include "Poco/NotificationCenter.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Exception.h"

#include "interface_to_host.h"
//#include "global.h"
//#include "home_mode.h"
//#include "login.h"
#include "notification_pkg.h"

using namespace Poco;

void enable_simulation_mode(bool en);
std::string parseCmdPackage(const std::string &pkg);
//void SetStatus_UpdateONSProfile(int iStatus);
//void SetStatus_UpdateONSConfig(int iStatus);
void SetStatus_RtcLogin(int iStatus);
void SetStatus_InitRS485(int iStatus);

class HostCmdHandler : public Poco::Task {
private:
    bool m_b_running;
    bool m_b_simulation_en;
    NotificationQueue &_queue_in;
    NotificationQueue &_queue_out;

public:
    HostCmdHandler(const std::string &name, NotificationQueue &queue_in, NotificationQueue &queue_out)
        : Task(name),
          _queue_in(queue_in),
          _queue_out(queue_out)
    {
        m_b_running = true;
    }
    void runTask();
    void enSimulation(bool en)
    {
        m_b_simulation_en = en;
    }
    void ExitTask();
};
