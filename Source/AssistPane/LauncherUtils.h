#ifndef _INC_LAUNCHERUTILS_
#define _INC_LAUNCHERUTILS_
//////////////////////////////////////////////////////////////////////////
#include <windows.h>
//////////////////////////////////////////////////////////////////////////
bool CheckPortCanBind(const char* _pszAddress, int _nPort);
bool RunProcess(const char* _pszExePath, const char* _pszRunParameters);
bool ProcessExist(const char* _pszImgName, DWORD* _pProcessID = NULL);
void SetRandomTitle(HWND _hWnd);
bool KillProcess(DWORD dwProcessId);
int CheckDisplayColorDepth();

void Launcher_Single(int _nPort);
void Launcher_Server(const char* _pszIp, int _nPort);
void Launcher_Client(const char* _pszIp, int _nPort, const char* _pszAccount, const char* _pszPassword);

void Launcher_CheckPatcher(const char* _pszPath);
//////////////////////////////////////////////////////////////////////////
#endif