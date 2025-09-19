#ifndef TCP_PROT_H_
#define TCP_PROT_H_

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>

#include "Poco/Exception.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/JSON/Object.h"
#include "Poco/TaskNotification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "Poco/Net/TCPServer.h"

using namespace std;
using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Net;

// #define USING_CIRCULAR_BUFFER 1

class TcpConnection: public TCPServerConnection
{
public:
  TcpConnection(const StreamSocket& s);
  void run();
};


class TcpDrvServer : public TCPServerConnectionFactory
{
public:
    TcpDrvServer();
    inline TCPServerConnection* createConnection(const StreamSocket &socket)
    {
        return new TcpConnection(socket);
    }
};

int SubscribeMessageToHost(int iSubscriber, string strMessage);
int UnsubscribeMessageToHost(int iSubscriber, string strMessage);
int send_message_to_host(std::string strStatus, int iErrorCode, const char *szFormat, ...);
int send_message_to_host(std::string status, int error_code, std::string message);
int send_log_to_host(std::string status, int error_code, std::string message);

typedef struct
{
    std::string status;
    int error_code;
    std::string message;
} MESSAGE_PACKAGE_t;

typedef boost::circular_buffer<MESSAGE_PACKAGE_t> circular_buffer;
class CustQueue
{
private:
    Poco::Event m_Event;
    Poco::Mutex m_BufferMutex;
    circular_buffer m_Buffer{64};
    // std::queue<MESSAGE_PACKAGE_t> m_Buffer;
public:
    int size();
    bool empty();
    void push(MESSAGE_PACKAGE_t package);
    MESSAGE_PACKAGE_t front();
    void pop();
};

class UplinkMessageHandler : public Poco::Task
{
private:
    Poco::Event m_Event;
    Poco::Mutex m_BufferMutex;
    // std::queue<MESSAGE_PACKAGE_t> m_queueMessage;
    CustQueue m_queueMessage;

    bool m_bRunFlag;
    int m_iPort;
public:
    UplinkMessageHandler(const std::string &name, int port);
    static UplinkMessageHandler *GetInstance(std::string name);
    void runTask();
    int SendMessageToHost(std::string status, int error_code, std::string message);
    void EnqueueMessage(std::string status, int error_code, std::string message);
    void ExitTask();
};

class DnlinkMessageHandler : public Poco::Task
{
private:
    Poco::Event m_event;
    bool m_bRunFlag;
    int m_iPort;
public:
    DnlinkMessageHandler(const std::string &name, int port);
    static DnlinkMessageHandler *GetInstance(std::string name);
    void runSingleLink();
    void runMultiLink();
    void runTask();
    void ExitTask();
};

#endif
