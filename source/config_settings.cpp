#include "config_settings.h"
#include "logger_wrapper.h"
#include "return_code.h"

vector<string> path_split(string s, string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;
    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(s.substr(pos_start));
    return res;
}

int GetSpecifyDirectoryPath(string specify_name, string *abs_path)
{
    int res = -1;
    char tmp[256];
    getcwd(tmp, 256);
    string str_tmp0, str_tmp1;
    path path_tmp = tmp;
    vector<string> v_path = path_split(path_tmp.string(), "/");
    for (int i = 0; i < (int)v_path.size(); i++) {
        str_tmp0 = path_tmp.parent_path().string();	// 退回上一層目錄.
        if (str_tmp0 == "/") {
            break;
        }

        str_tmp1 = str_tmp0 + "/" + specify_name;	// 組合 上一層目錄 + "/INI"
        path_tmp = str_tmp1.c_str();
        if (is_directory(path_tmp) == true) {		// 並檢查這樣的目錄是否存在.
            res = 0;
            *abs_path = str_tmp1;
            break;	// 如果這樣的目錄存在, 就當作找到 INI 目錄了.
        }
        path_tmp = str_tmp0;
    }
    return res;
}

int IniToProfileJson(const char *szFilename, json &pjProfile)
{
    FILE *fp;
// 每一行最大的長度
#define _MAX_LENGTH_BYTE 1024
    char szLine[_MAX_LENGTH_BYTE];
    //char szTmpKey[_MAX_LENGTH_BYTE];
    int iChar;
    int i = 0, k, iIndex;
    char *pcTemp;
    string strCurrentSection = "";

    if ((fp = fopen(szFilename, "r")) == NULL) {
        // printf("have   no   such   file \n");
        return -1;
    }

    while (!feof(fp) && i < _MAX_LENGTH_BYTE - 1) {
        iChar = fgetc(fp);
        if (iChar == EOF) {
            break;
        }
        else {
            szLine[i++] = iChar;
        }
        if (szLine[i - 1] == '\n') {
            i--;
            // Windows格式的換行是\r\n, Linux格式的換行是\n. 不能預期ini檔案是在哪種平台建立的, 所以都要能處理.
            if (szLine[i - 1] == '\r') {
                i--;
            }
            szLine[i] = '\0';
            i = 0;
            // check comment
            // pcTemp = szLine;
            for (iIndex = 0; iIndex < (int)strlen(szLine); iIndex++) {
                if (szLine[iIndex] != ' ' && szLine[iIndex] != '\t') {
                    break;
                }
            }
            if (szLine[iIndex] == ';' || szLine[iIndex] == '#' || (szLine[iIndex] == '/' && szLine[iIndex + 1] == '/')) {
                // if the first character is ; or # or //
                continue; // this line is a comment
            }
            // 由字串尾刪除空白和 \t 字元. 目的是過濾沒有內容的一行.
            for (k = strlen(szLine) - 1; k >= 0; k--) {
                if (szLine[k] == ' ' || szLine[k] == '\t') {
                    szLine[k] = 0;
                }
                else {
                    break;
                }
            }
            if (szLine[0] == '\0') {
                continue;
            }
            string strKeyName, strKeyValue;

            // 檢查這行有沒有'=', 有的話可能就是 Key
            pcTemp = strchr(szLine, '=');
            if (pcTemp != NULL) {
                // find a key name
                remove_char(szLine, ' ', '=');	// remove space from head, untill '='
                remove_char(szLine, '\t', '='); // remove \t from head, untill '='
                strKeyName = szLine;
                strKeyName.erase(strKeyName.find('=')); // 移除等號之後的字串, 包含等號, 就剩下 Key Name

                // find key value
                pcTemp = strchr(szLine, '='); // 由於 szLine 的內容可能在 remove_char( szLine,,) 被改動了, 所以 pTemp 要再重新指定一次.
                while (*(++pcTemp) == ' ' || *(pcTemp) == '\t')
                    ; // remove space and \t character in head.
                while (pcTemp[strlen(pcTemp) - 1] == ' ' || pcTemp[strlen(pcTemp) - 1] == '\t') {
                    // 從字串尾端刪除空白 or \t 字元.
                    pcTemp[strlen(pcTemp) - 1] = '\0';
                }
                // get key value
                strKeyValue = pcTemp;
                // printf("	%s=%s\n", strKeyName.c_str(), strKeyValue.c_str());
                if (strCurrentSection.length() > 0) {
                    pjProfile["profile"][strCurrentSection][strKeyName] = strKeyValue;
                }
                else {
                    goDriverLogger.Log("ERROR", "A key named %s without a section.", strKeyName.c_str());
                }
            }
            else {
                // 如果這行不包含'=', 那就檢查是否是 Section Name.
                string strTemp = "";
                pcTemp = strchr(szLine, '[');
                if (pcTemp != NULL) {
                    pcTemp = strchr(pcTemp, ']');
                    if (pcTemp != NULL) {
                        // 如果出現 '[' 再出現 ']', 就可能是 Section Name
                        strTemp = szLine;
                        strTemp = strTemp.substr(strTemp.find('[') + 1);
                        strTemp.erase(strTemp.find(']'));
                        strTemp.erase(0, strTemp.find_first_not_of(" \t")); // 由字首刪除空白 or \t 字元
                        strTemp.erase(strTemp.find_last_not_of(" \t") + 1); // 由字尾刪除空白 or \t 字元
                    }
                }
                if (strTemp.length() > 0) {
                    strCurrentSection = strTemp;
                    // printf("[%s]\n", strCurrentSection.c_str());
                }
                else {
                    // 既不是註解, 也不是 Section 或 Key, 那就是一行無法解析的內容.
                    fclose(fp);
                    goDriverLogger.Log("ERROR", "Fail to parse %s, content=%s", szFilename, szLine);
                    return ERR_FAIL_TO_PARSE;
                }
            }
        }
    }
    fclose(fp);
    if (i == _MAX_LENGTH_BYTE - 1) {
        return ERR_EXCEED_MAX_LIMIT; // 一行的長度超過最大限制.
    }
    return 0;
#undef _MAX_LENGTH_BYTE
}

