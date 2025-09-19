#ifndef ACCESS_INI_H
#define ACCESS_INI_H

int GetIniKeyString(char *filename, char *title, char *key, char *szValue, int iSizeOfValue);
int GetIniKeyInt(char *title, char *key, char *filename, int *piValue);
int GetIniKeyHex(char *szFilename, char *szSection, char *szKey, int *piValue);
int GetIniKeyDouble(char *szFilename, char *szSection, char *szKey, double *pdValue);
void remove_char(char *s, char cSuspect, char cEnd);
#endif