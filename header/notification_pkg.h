#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/ThreadPool.h"
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/Mutex.h"
#include "Poco/Random.h"
#include "Poco/AutoPtr.h"
#include <iostream>

#ifndef _NOTIFICATION_PKG_H_
#define _NOTIFICATION_PKG_H_

using namespace Poco;
using namespace std;


class NotificationToHost: public Notification
{
public:
	typedef AutoPtr<NotificationToHost> Ptr;	
	NotificationToHost(std::string data);
	std::string data() const;

private:
	std::string _data;
};

class NotificationFromHost: public Notification
{
public:
	typedef AutoPtr<NotificationFromHost> Ptr;	
	NotificationFromHost(std::string data);
	std::string data() const;

private:
	std::string _data;
};

#endif
