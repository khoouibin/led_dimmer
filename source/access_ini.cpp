
//#define CONF_FILE_PATH	"Config.ini"

#include <string.h>

#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#else

#define MAX_PATH 260

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#endif

char g_szConfigPath[MAX_PATH];

//从INI文件读取字符串类型数据
#define _MAX_LENGTH_BYTE 1024

void remove_char(char *s, char cSuspect, char cEnd)
{
	const char *d = s;
	int iFoundEnd = 0;
	do
	{
		if (*d == cEnd)
		{
			iFoundEnd = 1;
		}
		if (iFoundEnd == 0)
		{
			while (*d == cSuspect)
			{
				++d;
			}
			if (*d == cEnd)
			{
				iFoundEnd = 1;
			}
		}

	} while (*s++ = *d++);
}

// return :
// 0 = OK, find the key value
// -1= fail to find the key value
// -2= length of a line exceeds limit
int GetIniKeyString(char *szFilename, char *szSection, char *szKey, char *szValue, int iSizeOfValue)
{
	FILE *fp;
	char szLine[_MAX_LENGTH_BYTE];
	char szTmpKey[_MAX_LENGTH_BYTE];
	char szFullSection[_MAX_LENGTH_BYTE];
	int rtnval;
	int i = 0, p;
	int iFlagFindTitle = 0;
	char *tmp;

	if ((fp = fopen(szFilename, "r")) == NULL)
	{
		// printf("have   no   such   file \n");
		return -1;
	}

	strcpy(szFullSection, "[");
	strcat(szFullSection, szSection);
	strcat(szFullSection, "]");

	while (!feof(fp) && i < _MAX_LENGTH_BYTE - 1)
	{
		rtnval = fgetc(fp);
		if (rtnval == EOF)
		{
			break;
		}
		else
		{
			szLine[i++] = rtnval;
		}
		if (szLine[i - 1] == '\n')
		{
			i--;
			// Windows格式的換行是\r\n, Linux格式的換行是\n. 不能預期ini檔案是在哪種平台建立的, 所以都要能處理.
			if (szLine[i - 1] == '\r')
			{
				i--;
			}
			szLine[i] = '\0';
			i = 0;
			// check comment
			tmp = szLine;
			for (p = 0; p < strlen(szLine); p++)
			{
				if (szLine[p] != ' ' && szLine[p] != '\t')
				{
					break;
				}
			}
			if (szLine[p] == ';' || szLine[p] == '#' || (szLine[p] == '/' && szLine[p + 1] == '/'))
			{			  // if the first character is ; or #
				continue; // this line is a comment
			}

			tmp = strchr(szLine, '=');

			if ((tmp != NULL) && (iFlagFindTitle == 1))
			{
				remove_char(szLine, ' ', '=');	// remove space from head, untill '='
				remove_char(szLine, '\t', '='); // remove \t from head, untill '='
				strcpy(szTmpKey, szKey);
				strcat(szTmpKey, "=");
				if (strstr(szLine, szTmpKey) != NULL)
				{
					//注释行
					// if ('#' == szLine[0]){
					//}
					// else{
					// if ( '/' == szLine[0] && '/' == szLine[1] ){
					//}
					// else
					{
						//找打key对应变量
						tmp = strchr(szLine, '='); // 由於 szLine 的內容可能在 remove_char( szLine,,) 被改動了, 所以 tmp 要再重新指定一次.
						while (*(++tmp) == ' ' || *(tmp) == '\t')
							; // remove space character in head.
						while (tmp[strlen(tmp) - 1] == ' ' || tmp[strlen(tmp) - 1] == '\t')
						{
							tmp[strlen(tmp) - 1] = '\0';
						}
						if (strlen(tmp) + 2 < iSizeOfValue)
						{
							strcpy(szValue, tmp);
						}
						fclose(fp);
						return 0;
					}
					//}
				}
			}
			else
			{
				tmp = strchr(szLine, '[');
				if (tmp != NULL)
				{
					tmp = strchr(tmp, ']');
					if (tmp != NULL)
					{
						if (iFlagFindTitle)
						{
							// from target section to another section.
							fclose(fp);
							return -1;
						}
						else
						{
							if (strncmp(szFullSection, szLine, strlen(szFullSection)) == 0)
							{
								//找到title
								iFlagFindTitle = 1;
							}
							// else{	// not in the section
							//	iFlagFindTitle = 0;
							// }
						}
					}
				}
			}
		}
	}
	fclose(fp);
	if (i == _MAX_LENGTH_BYTE - 1)
	{
		return -2;
	}
	return -1;
}

//从INI文件读取整类型数据
// return :
// 0 = OK, find the key value
// -1= fail to find the key value
// -2= length of a line exceeds limit
int GetIniKeyInt(char *szFilename, char *szSection, char *szKey, int *piValue)
{
	char szValue[_MAX_LENGTH_BYTE];
	int iReturn;

	iReturn = GetIniKeyString(szFilename, szSection, szKey, szValue, sizeof(szValue));
	if (iReturn == 0)
	{
		*piValue = (int)atoi(szValue);
	}
	return iReturn;
}

int GetIniKeyDouble(char *szFilename, char *szSection, char *szKey, double *pdValue)
{
	char szValue[_MAX_LENGTH_BYTE];
	int iReturn;

	iReturn = GetIniKeyString(szFilename, szSection, szKey, szValue, sizeof(szValue));
	if (iReturn == 0)
	{
		*pdValue = atof(szValue);
	}
	return iReturn;
}

int GetIniKeyHex(char *szFilename, char *szSection, char *szKey, int *piValue)
{
	char szValue[_MAX_LENGTH_BYTE];
	int iReturn;

	iReturn = GetIniKeyString(szFilename, szSection, szKey, szValue, sizeof(szValue));

	if (iReturn == 0)
	{
		if (szValue[0] == '0' && (szValue[1] == 'x' || szValue[1] == 'X'))
		{
			*piValue = strtol(szValue + 2, 0, 16);
		}
		else
		{
			*piValue = strtol(szValue, 0, 16);
		}
	}
	return iReturn;
}
