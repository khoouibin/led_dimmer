#include "notification_pkg.h"

NotificationToHost::NotificationToHost(std::string data) : _data(data)
{

}

std::string NotificationToHost::data() const
{
    return _data;
}


NotificationFromHost::NotificationFromHost(std::string data) : _data(data)
{

}

std::string NotificationFromHost::data() const
{
    return _data;
}
