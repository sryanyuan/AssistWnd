#include "stdafx.h"
#include "LauncherUtils.h"

#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <time.h>
#include <TlHelp32.h>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
//////////////////////////////////////////////////////////////////////////
BOOL EnableDebugPrivilege(BOOL fEnable) 
{
	// Enabling the debug privilege allows the application to see
	// information about service applications
	BOOL fOk = FALSE;    // Assume function fails
	HANDLE hToken;

	// Try to open this process's access token
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		// Attempt to modify the "Debug" privilege
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		fOk = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return(fOk);
}

bool KillProcess(DWORD dwProcessId)
{
	bool	bRet = false;
	HANDLE	hProcess;
	EnableDebugPrivilege(TRUE);

	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);

	if(hProcess)
	{
		bRet = TerminateProcess(hProcess, (UINT)0xFFFFFFFF) == TRUE ? true : false;
		CloseHandle(hProcess);
	}
	EnableDebugPrivilege(FALSE);
	return bRet;
}

void GetRootPath(char* _pszBuf)
{
	GetModuleFileName(NULL, _pszBuf, MAX_PATH);
	PathRemoveFileSpec(_pszBuf);
}
//////////////////////////////////////////////////////////////////////////
bool CheckPortCanBind(const char* _pszAddress, int _nPort)
{
	//----------------------

	// Initialize Winsock

	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup()/n");
		return false;
	}

	bool bRet = false;
	SOCKET ListenSocket = INVALID_SOCKET;

	do 
	{
		ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (ListenSocket == INVALID_SOCKET) {
			printf("Error at socket(): %ld/n", WSAGetLastError());
			break;
		}

		//----------------------
		// The sockaddr_in structure specifies the address family,
		// IP address, and port for the socket that is being bound.
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = inet_addr(_pszAddress);
		service.sin_port = htons(_nPort);

		//----------------------
		// Bind the socket.
		if (bind( ListenSocket, 
			(SOCKADDR*) &service, 
			sizeof(service)) == SOCKET_ERROR) {
				printf("bind() failed./n");
				closesocket(ListenSocket);
				break;
		}

		bRet = true;

	} while (0);

	//	clean up
	if(INVALID_SOCKET != ListenSocket)
	{
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
	}
	WSACleanup();
	return bRet;
}


bool RunProcess(const char* _pszExePath, const char* _pszRunParameters)
{
	STARTUPINFO si = { sizeof(si) };   
	PROCESS_INFORMATION pi;   

	si.dwFlags = STARTF_USESHOWWINDOW;   
	si.wShowWindow = TRUE; //TRUE表示显示创建的进程的窗口   

	char szRunParamters[MAX_PATH];
	strcpy(szRunParamters, _pszRunParameters);
  
	BOOL bRet = ::CreateProcess (   
		_pszExePath,  
		szRunParamters, //在Unicode版本中此参数不能为常量字符串，因为此参数会被修改     
		NULL,   
		NULL,   
		FALSE,   
		CREATE_NEW_CONSOLE,   
		NULL,   
		NULL,   
		&si,   
		&pi);   

	int error = GetLastError();  
	if(bRet)   
	{   
		::CloseHandle (pi.hThread);   
		::CloseHandle (pi.hProcess);   

		printf(" 新进程的进程ID号：%d /n", pi.dwProcessId);   
		printf(" 新进程的主线程ID号：%d /n", pi.dwThreadId);   
	}   
	else  
	{  
		printf("error code:%d/n",error );  
	}  

	return (bRet == TRUE ? true : false);
}

bool ProcessExist(const char* _pszImgName, DWORD* _pProcessID/* = NULL */)
{
	bool bRet = false;

	PROCESSENTRY32 my;  //用来存放快照进程信息的一个结构体

	HANDLE l = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //创建进程快照

	if (((int)l) != -1)

	{

		my.dwSize = sizeof(my); //必须这样初始化，否则调用Process32First失败

		if (Process32First(l, &my)) //获得进程快照中的第一个进程

		{

			do

			{

				//CharLowerBuff(my.szExeFile, MAX_PATH);//转换成小写字母

				if(strcmp(_pszImgName ,my.szExeFile) == 0)  //查找对应进程

				{

					//dwProcID = my.th32ProcessID;//获取进程ID

					// ProcHWND = GetHWND(dwProcID);//获取窗口句柄
					if(_pProcessID)
					{
						*_pProcessID = my.th32ProcessID;
					}

					bRet = true;

					break;

				}

				else

				{

					bRet = false;

				}

			}while (Process32Next(l, &my)); //获取下一个进程



		}

		CloseHandle(l);

	}

	return bRet;
}

void SetRandomTitle(HWND _hWnd)
{
	char str[]="ABCDEFGHIJKLMHOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	char szCaption[256]={0};
	INT i,leg;
	srand((unsigned)time(NULL));	//让每次产生的随机数不同

	for(i=0;i<rand()%4+10;i++){
		//标题长度由rand()%4+10控制,长度为10或11、12、13、14
		leg = rand()%strlen(str);
		szCaption[i]=str[leg];	//给标题赋值
	}

	SetWindowText(_hWnd, szCaption);
}

int CheckDisplayColorDepth()
{
	HDC screenDC;
	BYTE numOfBits;

	screenDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	if (!screenDC)
		return 0;
	numOfBits = GetDeviceCaps(screenDC, BITSPIXEL);
	DeleteDC(screenDC);

	return numOfBits;
}



//////////////////////////////////////////////////////////////////////////
void Launcher_Single(int _nPort)
{
	bool bNeedRunServer = true;
	char szPath[MAX_PATH] = {0};
	GetRootPath(szPath);

	char szRunParameters[MAX_PATH] = {0};

	//	run server
	strcat(szPath, "/gamesvr.exe");
	if(!PathFileExists(szPath))
	{
		MessageBox(NULL, "无法找到服务器(GameSvr.exe)，文件可能丢失。请重新安装游戏程序。", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	DWORD dwProcessID = 0;
	if(ProcessExist("GameSvr.exe", &dwProcessID))
	{
		if(IDYES == MessageBox(NULL, "检测到服务器已运行，是否要关闭该服务器运行实例？", "询问", MB_YESNO | MB_ICONQUESTION))
		{
			if(!KillProcess(dwProcessID))
			{
				MessageBox(NULL, "服务器实例强行终止失败！", "错误", MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			bNeedRunServer = false;
		}
	}

	if(bNeedRunServer)
	{
		if(!CheckPortCanBind("127.0.0.1", _nPort))
		{
			sprintf(szRunParameters, "端口[%d]无法监听。该端口可能已被占用，请尝试更换其它端口。", _nPort);
			MessageBox(NULL, szRunParameters, "错误", MB_OK | MB_ICONERROR);
			return;
		}

		sprintf(szRunParameters, "listenip=127.0.0.1:%d",
			_nPort);

		if(!RunProcess(szPath, szRunParameters))
		{
			MessageBox(NULL, "无法运行游戏服务器。可能被安全软件禁止，或者缺少相应的运行时库。请将GameSvr.exe添加为信任程序，并且安装vs2008运行时库。", "错误", MB_OK | MB_ICONERROR);
			return;
		}
	}

	Sleep(3000);

	//	run client
	GetRootPath(szPath);
	strcat(szPath, "/backmir.exe");
	if(!PathFileExists(szPath))
	{
		MessageBox(NULL, "无法找到客户端(BackMIR.exe)，文件可能丢失。请重新安装游戏程序。", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	sprintf(szRunParameters, "svrip=127.0.0.1:%d",
		_nPort);

	if(!RunProcess(szPath, szRunParameters))
	{
		MessageBox(NULL, "无法运行游戏客户端。可能被安全软件禁止，或者缺少相应的运行时库。请将BackMIR.exe添加为信任程序，并且安装vs2008运行时库。", "错误", MB_OK | MB_ICONERROR);
		return;
	}
}

void Launcher_Server(const char* _pszIp, int _nPort)
{
	bool bNeedRunServer = true;
	char szPath[MAX_PATH] = {0};
	GetRootPath(szPath);

	char szRunParameters[MAX_PATH] = {0};

	//	run server
	strcat(szPath, "/gamesvr.exe");
	if(!PathFileExists(szPath))
	{
		MessageBox(NULL, "无法找到服务器(GameSvr.exe)，文件可能丢失。请重新安装游戏程序。", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	DWORD dwProcessID = 0;
	if(ProcessExist("GameSvr.exe", &dwProcessID))
	{
		if(IDYES == MessageBox(NULL, "检测到服务器已运行，是否要关闭该服务器运行实例？", "询问", MB_YESNO | MB_ICONQUESTION))
		{
			if(!KillProcess(dwProcessID))
			{
				MessageBox(NULL, "服务器实例强行终止失败！", "错误", MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			bNeedRunServer = false;
		}
	}

	if(bNeedRunServer)
	{
		if(!CheckPortCanBind(_pszIp, _nPort))
		{
			sprintf(szRunParameters, "端口[%d]无法监听。该端口可能已被占用，请尝试更换其它端口。", _nPort);
			MessageBox(NULL, szRunParameters, "错误", MB_OK | MB_ICONERROR);
			return;
		}

		sprintf(szRunParameters, "listenip=%s:%d",
			_pszIp, _nPort);

		if(!RunProcess(szPath, szRunParameters))
		{
			MessageBox(NULL, "无法运行游戏服务器。可能被安全软件禁止，或者缺少相应的运行时库。请将GameSvr.exe添加为信任程序，并且安装vs2008运行时库。", "错误", MB_OK | MB_ICONERROR);
			return;
		}
	}
}

void Launcher_Client(const char* _pszIp, int _nPort, const char* _pszAccount, const char* _pszPassword)
{
	if(NULL == _pszAccount)
	{
		_pszAccount = "";
	}
	if(NULL == _pszPassword)
	{
		_pszPassword = "";
	}

	char szPath[MAX_PATH] = {0};
	GetRootPath(szPath);

	char szRunParameters[MAX_PATH] = {0};

	GetRootPath(szPath);
	strcat(szPath, "/backmir.exe");
	if(!PathFileExists(szPath))
	{
		MessageBox(NULL, "无法找到客户端(BackMIR.exe)，文件可能丢失。请重新安装游戏程序。", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	sprintf(szRunParameters, "svrip=%s:%d account=%s password=%s",
		_pszIp, _nPort, _pszAccount, _pszPassword);

	if(!RunProcess(szPath, szRunParameters))
	{
		MessageBox(NULL, "无法运行游戏客户端。可能被安全软件禁止，或者缺少相应的运行时库。请将BackMIR.exe添加为信任程序，并且安装vs2008运行时库。", "错误", MB_OK | MB_ICONERROR);
		return;
	}
}

bool VersionHigher(const char* _pszCurrentVersion, const char* _pszCheckedVersion)
{
	int nCurrentMainVersion = 0;
	int nCurrentSubVersion = 0;
	int nCurrentMinVersion = 0;
	if(3 != sscanf(_pszCurrentVersion, "%d.%d.%d",
		&nCurrentMainVersion,
		&nCurrentSubVersion,
		&nCurrentMinVersion))
	{
		return false;
	}

	int nPatcherMainVersion = 0;
	int nPatcherSubVersion = 0;
	int nPatcherMinVersion = 0;
	if(3 != sscanf(_pszCheckedVersion,
		"%d.%d.%d",
		&nPatcherMainVersion,
		&nPatcherSubVersion,
		&nPatcherMinVersion))
	{
		return false;
	}

	if(nPatcherMainVersion > nCurrentMainVersion)
	{
		return true;
	}
	else if(nPatcherMainVersion < nCurrentMainVersion)
	{
		return false;
	}

	//	main version equal
	if(nPatcherSubVersion > nCurrentSubVersion)
	{
		return true;
	}
	else
	{
		return false;
	}

	if(nPatcherMinVersion > nCurrentMinVersion)
	{
		return true;
	}

	return false;
}

void Launcher_CheckPatcher(const char* _pszPath)
{
	char szCurrentVersion[32] = {0};
	char szPath[MAX_PATH] = {0};
	GetRootPath(szPath);
	strcat(szPath, "/ver.txt");

	//	read current version
	HANDLE hFile = CreateFile(szPath,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD dwRead = 0;
	ReadFile(hFile, szCurrentVersion, sizeof(szCurrentVersion), &dwRead, NULL);
	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;

	//	loop and read patcher version
	char szPatcherVersion[32] = {0};
	strcpy(szPatcherVersion, "2.09.33");

	if(VersionHigher(szCurrentVersion, szPatcherVersion))
	{
		char szMsg[MAX_PATH];
		sprintf(szMsg, "当前版本为 %s , 检测到新版本补丁 %s ,是否进行版本更新?",
			szCurrentVersion, szPatcherVersion);
		if(IDYES == MessageBox(NULL, szMsg, "询问", MB_YESNO | MB_ICONQUESTION))
		{
			//	extra

			//	delete
		}

		return;
	}
}