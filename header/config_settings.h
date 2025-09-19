#ifndef CONFIG_SETTINGS_H_
#define CONFIG_SETTINGS_H_
#include <fstream>
#include <experimental/filesystem>
#include <string>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include "json.hpp"
#include "access_ini.h"

using namespace std;
using namespace std::experimental::filesystem;
using json = nlohmann::json;

int GetSpecifyDirectoryPath(string specify_name, string *abs_path);
int IniToProfileJson(const char *szFilename, json &pjProfile);

#endif
