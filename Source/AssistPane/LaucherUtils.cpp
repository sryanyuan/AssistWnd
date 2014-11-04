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
	si.wShowWindow = TRUE; //TRUE��ʾ��ʾ�����Ľ��̵Ĵ���   

	char szRunParamters[MAX_PATH];
	strcpy(szRunParamters, _pszRunParameters);
  
	BOOL bRet = ::CreateProcess (   
		_pszExePath,  
		szRunParamters, //��Unicode�汾�д˲�������Ϊ�����ַ�������Ϊ�˲����ᱻ�޸�     
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

		printf(" �½��̵Ľ���ID�ţ�%d /n", pi.dwProcessId);   
		printf(" �½��̵����߳�ID�ţ�%d /n", pi.dwThreadId);   
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

	PROCESSENTRY32 my;  //������ſ��ս�����Ϣ��һ���ṹ��

	HANDLE l = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //�������̿���

	if (((int)l) != -1)

	{

		my.dwSize = sizeof(my); //����������ʼ�����������Process32Firstʧ��

		if (Process32First(l, &my)) //��ý��̿����еĵ�һ������

		{

			do

			{

				//CharLowerBuff(my.szExeFile, MAX_PATH);//ת����Сд��ĸ

				if(strcmp(_pszImgName ,my.szExeFile) == 0)  //���Ҷ�Ӧ����

				{

					//dwProcID = my.th32ProcessID;//��ȡ����ID

					// ProcHWND = GetHWND(dwProcID);//��ȡ���ھ��
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

			}while (Process32Next(l, &my)); //��ȡ��һ������



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
	srand((unsigned)time(NULL));	//��ÿ�β������������ͬ

	for(i=0;i<rand()%4+10;i++){
		//���ⳤ����rand()%4+10����,����Ϊ10��11��12��13��14
		leg = rand()%strlen(str);
		szCaption[i]=str[leg];	//�����⸳ֵ
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
		MessageBox(NULL, "�޷��ҵ�������(GameSvr.exe)���ļ����ܶ�ʧ�������°�װ��Ϸ����", "����", MB_OK | MB_ICONERROR);
		return;
	}

	DWORD dwProcessID = 0;
	if(ProcessExist("GameSvr.exe", &dwProcessID))
	{
		if(IDYES == MessageBox(NULL, "��⵽�����������У��Ƿ�Ҫ�رո÷���������ʵ����", "ѯ��", MB_YESNO | MB_ICONQUESTION))
		{
			if(!KillProcess(dwProcessID))
			{
				MessageBox(NULL, "������ʵ��ǿ����ֹʧ�ܣ�", "����", MB_OK | MB_ICONERROR);
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
			sprintf(szRunParameters, "�˿�[%d]�޷��������ö˿ڿ����ѱ�ռ�ã��볢�Ը��������˿ڡ�", _nPort);
			MessageBox(NULL, szRunParameters, "����", MB_OK | MB_ICONERROR);
			return;
		}

		sprintf(szRunParameters, "listenip=127.0.0.1:%d",
			_nPort);

		if(!RunProcess(szPath, szRunParameters))
		{
			MessageBox(NULL, "�޷�������Ϸ�����������ܱ���ȫ�����ֹ������ȱ����Ӧ������ʱ�⡣�뽫GameSvr.exe���Ϊ���γ��򣬲��Ұ�װvs2008����ʱ�⡣", "����", MB_OK | MB_ICONERROR);
			return;
		}
	}

	Sleep(3000);

	//	run client
	GetRootPath(szPath);
	strcat(szPath, "/backmir.exe");
	if(!PathFileExists(szPath))
	{
		MessageBox(NULL, "�޷��ҵ��ͻ���(BackMIR.exe)���ļ����ܶ�ʧ�������°�װ��Ϸ����", "����", MB_OK | MB_ICONERROR);
		return;
	}

	sprintf(szRunParameters, "svrip=127.0.0.1:%d",
		_nPort);

	if(!RunProcess(szPath, szRunParameters))
	{
		MessageBox(NULL, "�޷�������Ϸ�ͻ��ˡ����ܱ���ȫ�����ֹ������ȱ����Ӧ������ʱ�⡣�뽫BackMIR.exe���Ϊ���γ��򣬲��Ұ�װvs2008����ʱ�⡣", "����", MB_OK | MB_ICONERROR);
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
		MessageBox(NULL, "�޷��ҵ�������(GameSvr.exe)���ļ����ܶ�ʧ�������°�װ��Ϸ����", "����", MB_OK | MB_ICONERROR);
		return;
	}

	DWORD dwProcessID = 0;
	if(ProcessExist("GameSvr.exe", &dwProcessID))
	{
		if(IDYES == MessageBox(NULL, "��⵽�����������У��Ƿ�Ҫ�رո÷���������ʵ����", "ѯ��", MB_YESNO | MB_ICONQUESTION))
		{
			if(!KillProcess(dwProcessID))
			{
				MessageBox(NULL, "������ʵ��ǿ����ֹʧ�ܣ�", "����", MB_OK | MB_ICONERROR);
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
			sprintf(szRunParameters, "�˿�[%d]�޷��������ö˿ڿ����ѱ�ռ�ã��볢�Ը��������˿ڡ�", _nPort);
			MessageBox(NULL, szRunParameters, "����", MB_OK | MB_ICONERROR);
			return;
		}

		sprintf(szRunParameters, "listenip=%s:%d",
			_pszIp, _nPort);

		if(!RunProcess(szPath, szRunParameters))
		{
			MessageBox(NULL, "�޷�������Ϸ�����������ܱ���ȫ�����ֹ������ȱ����Ӧ������ʱ�⡣�뽫GameSvr.exe���Ϊ���γ��򣬲��Ұ�װvs2008����ʱ�⡣", "����", MB_OK | MB_ICONERROR);
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
		MessageBox(NULL, "�޷��ҵ��ͻ���(BackMIR.exe)���ļ����ܶ�ʧ�������°�װ��Ϸ����", "����", MB_OK | MB_ICONERROR);
		return;
	}

	sprintf(szRunParameters, "svrip=%s:%d account=%s password=%s",
		_pszIp, _nPort, _pszAccount, _pszPassword);

	if(!RunProcess(szPath, szRunParameters))
	{
		MessageBox(NULL, "�޷�������Ϸ�ͻ��ˡ����ܱ���ȫ�����ֹ������ȱ����Ӧ������ʱ�⡣�뽫BackMIR.exe���Ϊ���γ��򣬲��Ұ�װvs2008����ʱ�⡣", "����", MB_OK | MB_ICONERROR);
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
		sprintf(szMsg, "��ǰ�汾Ϊ %s , ��⵽�°汾���� %s ,�Ƿ���а汾����?",
			szCurrentVersion, szPatcherVersion);
		if(IDYES == MessageBox(NULL, szMsg, "ѯ��", MB_YESNO | MB_ICONQUESTION))
		{
			//	extra

			//	delete
		}

		return;
	}
}