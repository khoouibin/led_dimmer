#include <QCoreApplication>
#include <QThread>
#include <unistd.h>
// #include <iostream>
#include <stdlib.h>
#include <string>
#include "cli.h"


using namespace std;

int main(int argc, char *argv[])
{
    int iCLI = -1;

    QCoreApplication a(argc, argv);
    for (int i = 0; i < argc; ++i) {
        // std::cout << "  Argument " << i << ": " << argv[i] << std::endl;
        if (i>= 1)
        {
            if (std::string(argv[i]) == "-cli")
            {
                iCLI = 1;
            }
        }
    }
    
    if (iCLI == 1)
    {
        CommandLineInterface();
    }
    else
    {
        int cnt = 0;
        while(true)
        {
            printf("sleep = %d \n",cnt);
            cnt +=1;
            QThread::msleep(500);
        }
    }



    _exit(0); // 要用 _exit() 才能頭也不回的結束. 用 exit(0) 不行.
    return 0;
}
