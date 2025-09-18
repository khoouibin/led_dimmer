#include "cli.h"
#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
// #include <entity_io.h>
#include <iostream>
#include <algorithm>
#include <sys/ioctl.h>
#include <sys/socket.h>
// #include "USBComm.h"
// #include "USBComm_Driver.h"
// #include "Msg_Prot.h"

// #include "ReceivMsg.h"
// #include "global.h"
// #include "login.h"
// #include "RS485_comm.h"

// #include "interface_to_host.h"
//  #include "notification_pkg.h"
//  #include "httpinterface.h"
//  #include "json.hpp"
//  #include "return_code.h"
// #include "SEWPGM.h"
// #include "RPM.h"
//  #include "sew_pgs_controller.h"
// #include "ONS_Trace.h"
// #include "sew_alarm.h"
//  #include "xy_control.h"
// #include "pf_apf.h"
// #include "rtc_init.h"
// #include "XY_World.h"
// #include "machine_ctrl.h"
// #include "rtc_status.h"
//  #include "logger_wrapper.h"
// #include "pathfinder.h"
// #include "APF_Def.h"
// #include "PGS_Merge.h"
// #include "electric_meter.h"
//  #include "clamp.h"
//  #include "z_control.h"
//  #include "pf_apf.h"
//  #include "register.h"
//  #include "jy_rtc_comm.h"
//  #include "jy_rtc_io.h"
//  #include <QList>
//  #include "tension_control.h"

// extern int PgsTest();
// extern bool g_485_msg;
using namespace std;

// extern Sew_PGM My_Sew_PGM;

// for test
// static int giDryRunSpeed = 5;
// static int giSewingSpeed = 5;

int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

vector<string> split(string s, string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

vector<string> helper_doc(void)
{
    vector<string> doc;
    /*doc.push_back(" common command:");*/
    doc.push_back(" Press 'q' to quit.");

    for (int i = 0; i < (int)doc.size(); i += 1)
    {
        cout << doc[i] << endl;
    }

    return doc;
}

// actually, i don't know what's this to do.
int kbhit(void)
{
    struct timeval tv;
    struct termios old_termios, new_termios;
    int error;
    int count = 0;
    tcgetattr(0, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~ICANON;
    new_termios.c_lflag &= ~ECHO;
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;
    error = tcsetattr(0, TCSANOW, &new_termios);
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    select(1, NULL, NULL, NULL, &tv);
    error += ioctl(0, FIONREAD, &count);
    error += tcsetattr(0, TCSANOW, &old_termios);
    return error == 0 ? count : -1;
}

extern std::string parseCmdPackage(const std::string &pkg);

void CommandLineInterface()
{
    // int iUsbStatus;
    int cli_break = 0;
    std::string command_line;
    vector<string> vCommandHistory;

    string delimiter = " ";
    std::cout << endl;
    std::cout << " press 'help' to read A simple guide. " << endl;
    std::cout << ">>";
    // vCommandHistory.push_back("update_profile_from_file /home/orisol/Code/Orisol/sew_jy_bitbucket/INI/ONS_Profile.ini");
    // vCommandHistory.push_back("update_config_from_file /home/orisol/Code/Orisol/sew_jy_bitbucket/INI/ONS_Config.ini");
    // vCommandHistory.push_back("home");

    while (cli_break == 0)
    {
        getline(cin, command_line);
        usleep(50000);
    USER_SELECT:
        vector<string> v = split(command_line, delimiter);
        std::string cmd;
        if (v.size() == 0)
        {
            break;
        }
        else
        {
            cmd = v[0];
        }

        if (cmd == "help")
        {
            helper_doc();
        }
        else if (cmd == "q")
        {
            cli_break = 1;
        }
        // else if (cmd == "test")
        // {
        //     printf("vect size: %d", (int)v.size());
            
        // }
        else
        {
            if (cmd.size() == 0 && vCommandHistory.size() > 0)
            {
                for (int i = 0; i < (int)vCommandHistory.size(); i++)
                {
                    printf("[%c]%s\n", 'a' + i, vCommandHistory[i].c_str());
                }
                char szUserKey[2] = {0};
                szUserKey[0] = (char)getch();
                int iUserKey = szUserKey[0];
                iUserKey -= 'a';
                if (iUserKey >= 0 && iUserKey < (int)vCommandHistory.size())
                {
                    printf("%s", vCommandHistory[iUserKey].c_str());
                    command_line = vCommandHistory[iUserKey];
                    goto USER_SELECT;
                }
                else
                {
                    printf("Unknown choice.\n");
                    printf("\n>>");
                }
                continue;
            }
            else
            {
                cout << "unsupported command:" << command_line << endl;
                printf("\n>>");
                // fflush(stdout);
                // fflush(stdin);
                continue;
            }
        }

        vector<string>::iterator result = find(vCommandHistory.begin(), vCommandHistory.end(), command_line);
        if (result == vCommandHistory.end())
        {
            vCommandHistory.push_back(command_line);
            if (vCommandHistory.size() > 26)
            {
                vCommandHistory.pop_back();
            }
        }
        printf("\n>>");
        // fflush(stdout);
        // 因kbhit的影響, 此處未清出stdout的話, 畫面顯示會有問題.
    }
    cout << "quit cli ..." << endl;
}
