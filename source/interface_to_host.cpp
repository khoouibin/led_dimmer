#include "interface_to_host.h"
#include "notification_pkg.h"
#include "HandleCmd.h"
#include "return_code.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Poco/Task.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/TaskManager.h"
#include "Poco/TaskNotification.h"
#include "Poco/NotificationCenter.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Exception.h"
#include "Poco/Mutex.h"
#include "Poco/Condition.h"
#include "Poco/Event.h"
#include "Poco/Timestamp.h"
#include "logger_wrapper.h"
#include <stdarg.h>
#include <cstdlib>

#include "json.hpp"
using json = nlohmann::json;

int CustQueue::size()
{
    int iSize = 0;
    iSize = m_Buffer.size();
    return iSize;
}

bool CustQueue::empty()
{
    return size() == 0;
}

void CustQueue::push(MESSAGE_PACKAGE_t package)
{
    m_Buffer.push_back(package);
}

MESSAGE_PACKAGE_t CustQueue::front()
{
    MESSAGE_PACKAGE_t MessagePackage;
    MessagePackage = m_Buffer[0];
    return MessagePackage;
}

void CustQueue::pop()
{
    m_Buffer.pop_front();
}

TcpConnection::TcpConnection(const StreamSocket &s)
    : TCPServerConnection(s)
{
}

void TcpConnection::run()
{
    StreamSocket &ss = socket();
    try {
        char recvbuffer[327680]; // 80 * 4096.
        const std::string ack = "ack";

        int n = ss.receiveBytes(recvbuffer, sizeof(recvbuffer));
        recvbuffer[n] = '\0';
        //std::cout << "Host command:" << recvbuffer << "\n"
        //          << "size:" << n << std::endl;

        std::string str_recv_msg = std::string(recvbuffer);

        while (n > 0) {
            // 接收由HOST傳來的訊息
            std::string str_ack = parseCmdPackage(str_recv_msg);

            const char *sendbuffer = str_ack.c_str();
            int sendbuffer_length = str_ack.length();

            ss.sendBytes(sendbuffer, sendbuffer_length);
            str_ack.clear();

            n = ss.receiveBytes(recvbuffer, sizeof(recvbuffer));
            recvbuffer[n] = '\0';
            str_recv_msg = std::string(recvbuffer);
        }
    }
    catch (Poco::Exception &exc) {
        std::cerr << "Connection exception: " << exc.displayText() << std::endl;
    }
}

TcpDrvServer::TcpDrvServer()
{

}


/////////////////////// send message to Host /////////////////////////////////////

#include<stdio.h>
#include<sys/socket.h>
#include <mutex>

static std::mutex gmtxSendMessage;
static std::mutex gmtxSendLog;

// gvProhibitMessage 紀錄的是不發送出去的 message.
vector <string> gvProhibitMessage = {"z_position_change"};

// 如果要訂閱, 就是把要訂閱的訊息, 從"不發送出去的 message 列表"裡移除.
int SubscribeMessageToHost(int iSubscriber, string strMessage)
{
    auto iter = gvProhibitMessage.begin();
    while (iter != gvProhibitMessage.end()) {
        if (*iter == strMessage) {
            iter = gvProhibitMessage.erase(iter);
        }
        else {
            ++iter;
        }
    }
//    if (std::find(gvProhibitMessage.begin(), gvProhibitMessage.end(), strMessage) != gvProhibitMessage.end())
    return 0;
}

// 如果要取消訂閱, 就是把要取消訂閱的訊息, 加到 "不發送出去的 message 列表".
int UnsubscribeMessageToHost(int iSubscriber, string strMessage)
{
    if (std::find(gvProhibitMessage.begin(), gvProhibitMessage.end(), strMessage) != gvProhibitMessage.end()) {
        return 0;   // already exist.
    }
    gvProhibitMessage.push_back(strMessage);
    return 0;
}

// 傳送訊息到HOST
int send_message_to_host(std::string strStatus, int iErrorCode, const char *szFormat, ...)
{
    char szMessage[_MAX_LOG_BYTE];
    va_list arg_ptr;

    va_start(arg_ptr, szFormat);
    vsprintf(szMessage, szFormat, arg_ptr);
    va_end(arg_ptr);

    return send_message_to_host(strStatus, iErrorCode, string(szMessage));
}

int send_message_to_host(std::string strStatus, int error_code, std::string message)
{
    while (!gmtxSendMessage.try_lock()) {
        usleep(500);
    }

    if (gvProhibitMessage.size() > 0) {
        if (std::find(gvProhibitMessage.begin(), gvProhibitMessage.end(), strStatus) != gvProhibitMessage.end()) {
            gmtxSendMessage.unlock();
            return 0;
        }
    }
#if 1
    UplinkMessageHandler *uplink_message_handler = UplinkMessageHandler::GetInstance("Msg");
    uplink_message_handler->EnqueueMessage(strStatus, error_code, message);
    gmtxSendMessage.unlock();
    return 0;
#else
    UplinkMessageHandler *uplink_message_handler = UplinkMessageHandler::GetInstance();
    uplink_message_handler->SendMessageToHost(strStatus, error_code, message);
    gmtxSendMessage.unlock();
#endif
}

int send_log_to_host(std::string status, int error_code, std::string message)
{
    while (!gmtxSendMessage.try_lock()) {
        usleep(500);
    }

    UplinkMessageHandler *uplink_message_handler = UplinkMessageHandler::GetInstance("Log");
    uplink_message_handler->EnqueueMessage(status, error_code, message);
    gmtxSendMessage.unlock();
    return 0;
}

static UplinkMessageHandler *instance_msg;
static UplinkMessageHandler *instance_log;

UplinkMessageHandler::UplinkMessageHandler(const std::string &name, int port)
    : Task(name)
{
    m_bRunFlag = true;
    m_iPort = port;
}

UplinkMessageHandler *UplinkMessageHandler::GetInstance(std::string name)
{
    if (name == "Msg") {
        if (NULL == instance_msg) {
            instance_msg = new UplinkMessageHandler(name, 8002);
        }
        return instance_msg;
    }
    else if (name == "Log") {
        if (NULL == instance_log) {
            instance_log = new UplinkMessageHandler(name, 8003);
        }
        return instance_log;
    }
    return NULL;
}

void UplinkMessageHandler::runTask()
{
    int iReturn;
    int iTaskNumber = 0;
    MESSAGE_PACKAGE_t message_package;

    while (m_bRunFlag) {
        m_BufferMutex.lock();
        iTaskNumber = m_queueMessage.size();

        if (iTaskNumber == 0) {
            m_BufferMutex.unlock();
            m_Event.wait();
            continue;
        }

        MESSAGE_PACKAGE_t message_package = m_queueMessage.front();
        iReturn = SendMessageToHost(message_package.status, message_package.error_code, message_package.message);
        if (iReturn == 0) {
            m_queueMessage.pop();
        }

        else {
            // 由於 Host 可能比 Driver 稍晚一點啟動, 所以 Driver 早期的 Log 送給 Host 時會失敗.
            // 但如果累積的 Log 數量不太多, 就先保留下來, 稍後 Host 啟動後再一併送出.
            if (m_iPort == 8003) {
                // 8003 = log port
                if (m_queueMessage.size() > 100) {
                    while (m_queueMessage.size() > 0) {
                        m_queueMessage.pop();
                    }
                    // 注意！一旦累積的訊息超過指定數量, 就判斷為與 Host 之間無網路, 以下設定 LOG 不再傳輸給 Host, 並且也不會恢復.
                    goDriverLogger.SetLogToRemoteEnable(false);
                    goDriverLogger.SetLogToLocalEnable(true);
                }
            }
            else {
                // 8002 = message port
                m_queueMessage.pop();
            }
        }

        if (message_package.status == "exit" && message_package.message == "exit") {
            break;
        }

        m_BufferMutex.unlock();
    }
    //  pthread_detach(pthread_self());
    //	pthread_exit(NULL);
}

void UplinkMessageHandler::ExitTask()
{
    m_bRunFlag = false;
}

typedef struct {
    int time_pre;
    int time_now;
    int time_diff;
} timestamp_t;

int time_stamp(void)
{
    struct timeval tv;

    int _time_now = 0;
    gettimeofday(&tv, NULL);
    _time_now = (((int)tv.tv_sec) % 1000) * 1000;
    _time_now += ((int)tv.tv_usec / 1000);
    return _time_now;
}

int UplinkMessageHandler::SendMessageToHost(std::string status, int error_code, std::string message)
{
    json notification;
    notification["cmd"] = "notification";
    notification["status"] = status;
    notification["error_code"] = error_code;
    notification["message"] = message;
    int iReturn;
    int iSockfd = 0;

    timestamp_t time_delay;
    time_delay.time_pre = time_stamp();

    iSockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (iSockfd == -1) {
        goDriverLogger.Log("ERROR", "Fail to create a socket for sending message to host.");
        return ERR_FAIL_TO_SOCKET;
    }

    struct sockaddr_in tSockAddr;
    struct in_addr;

    bzero(&tSockAddr, sizeof(tSockAddr));
    tSockAddr.sin_family = PF_INET;
    tSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    tSockAddr.sin_port = htons(m_iPort);

    iReturn = connect(iSockfd, (struct sockaddr *)&tSockAddr, sizeof(tSockAddr));
    if (iReturn) {
        time_delay.time_now = time_stamp();
        time_delay.time_diff = time_delay.time_now - time_delay.time_pre;
//printf("send_message_to_host:tcp task connect fail. time=%d. Socket=%d\n", time_delay.time_diff, iSockfd );
        shutdown(iSockfd, SHUT_RDWR);
        close(iSockfd);
//printf("closed.\n");
        return ERR_FAIL_TO_CONNECT;
    }
    string str_msg = notification.dump();

    iReturn = send(iSockfd, str_msg.c_str(), str_msg.length(), 0);
    if (iReturn) {
//time_delay.time_now = time_stamp();
//time_delay.time_diff= time_delay.time_now - time_delay.time_pre;
//printf("tcp task success to send %d byte. time=%d\n", iReturn, time_delay.time_diff );
        //close(iSockfd);
        shutdown(iSockfd, SHUT_RDWR);
        close(iSockfd);
        return 0;
    }
    else {
        time_delay.time_now = time_stamp();
        time_delay.time_diff = time_delay.time_now - time_delay.time_pre;
        goDriverLogger.Log("ERROR", "TCP task fail to send message to host. time=%d.", time_delay.time_diff);
        //close(iSockfd);
        shutdown(iSockfd, SHUT_RDWR);
        close(iSockfd);
        return ERR_FAIL_TO_SEND;
    }
}

void UplinkMessageHandler::EnqueueMessage(std::string status, int error_code, std::string message)
{
    MESSAGE_PACKAGE_t message_package;
    message_package.message = message;
    message_package.error_code = error_code;
    message_package.status = status;

    m_BufferMutex.lock();
    m_queueMessage.push(message_package);
    m_BufferMutex.unlock();
    m_Event.set();
}

static DnlinkMessageHandler *instance_cmd;

DnlinkMessageHandler::DnlinkMessageHandler(const std::string &name, int port) : Task(name)
{
    m_bRunFlag = true;
    m_iPort = port;
}

DnlinkMessageHandler *DnlinkMessageHandler::GetInstance(std::string name)
{
    if (NULL == instance_cmd) {
        instance_cmd = new DnlinkMessageHandler(name, 8001);
    }
    return instance_cmd;
}

void DnlinkMessageHandler::runMultiLink()
{
    Poco::Net::ServerSocket serverSocket(8001);
    Poco::Net::TCPServerParams::Ptr param = new Poco::Net::TCPServerParams;
    param->setMaxQueued(100);
    param->setMaxThreads(100);
    Poco::Net::TCPServer ServerForHost(new TcpDrvServer(), serverSocket);
    ServerForHost.start();
    m_event.wait();
}

void DnlinkMessageHandler::runSingleLink()
{
    const char *host = "0.0.0.0";
    int port = m_iPort;

    int sock_fd, new_fd;
    socklen_t addrlen;
    struct sockaddr_in my_addr, client_addr;
    int status;
    char indata[327680] = {0}, outdata[1024] = {0};
    int on = 1;

    // create a socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("Socket creation error");
        exit(1);
    }

    // for "Address already in use" error message
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) {
        perror("Setsockopt error");
        exit(1);
    }

    // server address
    my_addr.sin_family = AF_INET;
    inet_aton(host, &my_addr.sin_addr);
    my_addr.sin_port = htons(port);

    status = bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if (status == -1) {
        //perror("Binding error");
        printf("\n\e[0;31m Serious Error! Binding error! 用來接收 Host 指令的 Port %d 無法綁定, 請檢查是否有其他應用程式佔用. 程式結束. \e[0m\n", port);
        exit(1);
    }
    //printf("server start at: %s:%d\n", inet_ntoa(my_addr.sin_addr), port);

    status = listen(sock_fd, 5);
    if (status == -1) {
        perror("Listening error");
        exit(1);
    }
    //printf("wait for connection...\n");

    addrlen = sizeof(client_addr);

    while (m_bRunFlag) {
        new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &addrlen);
        //printf("connected by %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (1) {
            int nbytes = recv(new_fd, indata, sizeof(indata), 0);
            if (nbytes <= 0) {
                close(new_fd);
                //printf("client closed connection.\n");
                break;
            }
            //printf("recv: %s\n", indata);
            indata[nbytes] = '\0';
            std::string str_ack = parseCmdPackage(indata);

            const char *sendbuffer = str_ack.c_str();
            send(new_fd, sendbuffer, strlen(sendbuffer), 0);
        }
    }

    close(sock_fd);
}

void DnlinkMessageHandler::runTask()
{
    runSingleLink();
    // 如果是 runMultiLink, 則每次收到 TCP 封包時, 會產生一個 Thread 去處理, 變成 multi-thread 模式,
    // 會造成 parseCmdPackage() 所處理的函式發生 re-entry, 而 Driver 提供的介面函式在設計時並未考慮到支持 reentrant, 所以會發生錯誤.
    // runMultiLink();
}

void DnlinkMessageHandler::ExitTask()
{
    m_bRunFlag = false;
    m_event.set();
}
