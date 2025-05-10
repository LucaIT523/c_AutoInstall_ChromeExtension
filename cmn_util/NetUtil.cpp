#include "NetUtil.h"
#include <iphlpapi.h>
#include <iomanip>
#include <iostream>
#include <Windows.h>
#include <sstream>
#include <random>
#include <atlstr.h> 
#include "ProcessUtil.h"

// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "wininet.lib")
int             lv_nPort = 80;


// Function to get the Windows OS version using RtlGetVersion
std::string GetOSVersion() {
    typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    RTL_OSVERSIONINFOW osInfo = { 0 };
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);

    RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion");
    if (RtlGetVersion) {
        RtlGetVersion(&osInfo);
        return std::to_string(osInfo.dwMajorVersion) + "." + std::to_string(osInfo.dwMinorVersion);
    }
    return "Unknown Windows Version";
}

// Function to get the MAC address
std::string GetMACAddress() {
    IP_ADAPTER_INFO AdapterInfo[16];       // Allocate memory for up to 16 NICs
    DWORD dwBufLen = sizeof(AdapterInfo);  // Size of the buffer

    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);  // Call to GetAdaptersInfo
    if (dwStatus != ERROR_SUCCESS) {
        return "Error getting MAC address";
    }

    // Loop through the network adapters to get the first MAC address
    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    std::ostringstream macStream;
    for (int i = 0; i < pAdapterInfo->AddressLength; i++) {
        if (i != 0) {
            macStream << "-";
        }
        macStream << std::hex << std::setw(2) << std::setfill('0') << (int)pAdapterInfo->Address[i];
    }

    return macStream.str();
}

bool HttpGet(const std::string& server, const std::string& path, std::string& response) {
    HINTERNET hSession = InternetOpen(L"HTTP_GET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) {
        return false;
    }

    // Convert server and path to wide strings
    std::wstring wServer(server.begin(), server.end());
    std::wstring wPath(path.begin(), path.end());

    // Use the HTTPS port and set appropriate flags
    HINTERNET hConnect = InternetConnect(hSession, wServer.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hSession);
        return false;
    }

    // Set the INTERNET_FLAG_SECURE flag for HTTPS
    HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", wPath.c_str(), NULL, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return false;
    }

    // Send the request
    BOOL bSend = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
    if (!bSend) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return false;
    }

    // Read the response
    char buffer[1024];
    DWORD bytesRead;
    response.clear();

    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }

    // Clean up handles
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return true;
}


bool HTTPSPost_GetUUID(const std::string server, const std::string path, const std::string& data, std::string& response, const std::string authToken) {
    HINTERNET hSession = InternetOpen(L"HTTP_POST", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) {
//        std::cerr << "InternetOpen failed: " << GetLastError() << std::endl;
        return false;
    }

    HINTERNET hConnect = InternetConnect(hSession, std::wstring(server.begin(), server.end()).c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
//        std::cerr << "InternetConnect failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hSession);
        return false;
    }

    HINTERNET hRequest = HttpOpenRequest(hConnect, L"POST", std::wstring(path.begin(), path.end()).c_str(), NULL, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
//        std::cerr << "HttpOpenRequest failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return false;
    }

    // Prepare the headers
    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"Authorization: Bearer " + std::wstring(authToken.begin(), authToken.end()) + L"\r\n";

    // Send the POST request with the JSON data
    BOOL bSend = HttpSendRequest(hRequest, headers.c_str(), (DWORD)headers.length(), (LPVOID)data.c_str(), (DWORD)data.length());
    if (!bSend) {
//        std::cerr << "HttpSendRequest failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return false;
    }

    // Read the response
    char buffer[1024];
    DWORD bytesRead;
    response.clear();

    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }

    // Clean up handles
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return true;
}
//
//
//bool DownloadFile(const std::string& server, const std::string& path, const std::string& localFilePath) {
//    HINTERNET hSession = InternetOpen(L"HTTP_DOWNLOAD", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
//    if (!hSession) {
//        return false;
//    }
//
//    HINTERNET hConnect = InternetConnect(hSession, std::wstring(server.begin(), server.end()).c_str(), lv_nPort/*INTERNET_DEFAULT_HTTP_PORT*/, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
//    if (!hConnect) {
//        InternetCloseHandle(hSession);
//        return false;
//    }
//
//    HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", std::wstring(path.begin(), path.end()).c_str(), NULL, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
//    if (!hRequest) {
//        InternetCloseHandle(hConnect);
//        InternetCloseHandle(hSession);
//        return false;
//    }
//
//    BOOL bSend = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
//    if (!bSend) {
//        InternetCloseHandle(hRequest);
//        InternetCloseHandle(hConnect);
//        InternetCloseHandle(hSession);
//        return false;
//    }
//
//    std::ofstream outFile(localFilePath, std::ios::binary);
//    if (!outFile) {
//        InternetCloseHandle(hRequest);
//        InternetCloseHandle(hConnect);
//        InternetCloseHandle(hSession);
//        return false;
//    }
//
//    char buffer[4096];
//    DWORD bytesRead;
//    int     w_nTotalReadLen = 0;
//
//    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead) {
//        outFile.write(buffer, bytesRead);
//        w_nTotalReadLen += bytesRead;
//    }
//
//    outFile.close();
//    InternetCloseHandle(hRequest);
//    InternetCloseHandle(hConnect);
//    InternetCloseHandle(hSession);
//
//    if (w_nTotalReadLen < 1024) {
//        return false;
//    }
//    else {
//        return true;
//    }
//}

bool HTTPSGet_DownloadFile(const std::string server, const std::string& path, const std::string& localFilePath, const std::string bearerToken) {
    HINTERNET hSession = InternetOpen(L"HTTPS_DOWNLOAD", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) {
        return false;
    }

    HINTERNET hConnect = InternetConnect(hSession, std::wstring(server.begin(), server.end()).c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hSession);
        return false;
    }

    HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", std::wstring(path.begin(), path.end()).c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return false;
    }

    // Construct the Authorization header with the Bearer Token
    std::string headers = "Authorization: Bearer " + bearerToken + "\r\n";

    BOOL bSend = HttpSendRequest(hRequest, std::wstring(headers.begin(), headers.end()).c_str(), -1, NULL, 0);
    if (!bSend) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return false;
    }

    std::ofstream outFile(localFilePath, std::ios::binary);
    if (!outFile) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    int totalReadLen = 0;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead) {
        outFile.write(buffer, bytesRead);
        totalReadLen += bytesRead;
    }

    outFile.close();
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return totalReadLen >= 1024; // Return false if the downloaded file is less than 1KB
}
void splitServerAndPathInfo(const std::string& input, std::string& server, std::string& path) {
    // Find the position of the first '/'
    size_t pos = input.find('/');

    if (pos != std::string::npos) {
        // Extract the server part before the '/'
        server = input.substr(0, pos);
        // Extract the path part after the '/'
        path = input.substr(pos + 1);
    }
    else {
        // If '/' is not found, treat the entire input as server
        server = input;
        path = "";  // No path
    }
}

bool HTTPSGet(const std::string server, const std::string path, std::string& response, const std::string authToken) {
    try{
        HINTERNET hSession = InternetOpen(L"HTTP_GET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hSession) {
            //std::cerr << "InternetOpen failed: " << GetLastError() << std::endl;
            return false;
        }

        HINTERNET hConnect = InternetConnect(hSession, std::wstring(server.begin(), server.end()).c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) {
            //std::cerr << "InternetConnect failed: " << GetLastError() << std::endl;
            InternetCloseHandle(hSession);
            return false;
        }

        HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", std::wstring(path.begin(), path.end()).c_str(), NULL, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if (!hRequest) {
            //std::cerr << "HttpOpenRequest failed: " << GetLastError() << std::endl;
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hSession);
            return false;
        }
        BOOL bSend = FALSE;

        if (authToken.empty() == false) {
            // Prepare the headers
            std::wstring headers = L"Authorization: Bearer " + std::wstring(authToken.begin(), authToken.end()) + L"\r\n";
            // Send the GET request
            bSend = HttpSendRequest(hRequest, headers.c_str(), (DWORD)headers.length(), NULL, 0);

        }
        else {
            bSend = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
        }
        if (!bSend) {
            //std::cerr << "HttpSendRequest failed: " << GetLastError() << std::endl;
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hSession);
            return false;
        }

        // Read the response
        char buffer[1024];
        DWORD bytesRead;
        response.clear();

        while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }

        // Clean up handles
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
    }
    catch (const std::exception& ex) {
        //std::cerr << "Exception occurred: " << ex.what() << std::endl;
        return false;
    }
    catch (...) {
        //std::cerr << "An unknown exception occurred." << std::endl;
        return false;
    }
    return true;
}

std::string GetIDVaule(std::string& p_sDataString)
{
    std::string idValue = "";

    std::string jsonString = p_sDataString;

    // Find the position of the "id" key
    size_t idPos = jsonString.find("\"id\":");
    if (idPos != std::string::npos) {
        // Move the position past '"id": '
        idPos = jsonString.find("\"", idPos + 5) + 1;

        // Find the position of the closing quote
        size_t endPos = jsonString.find("\"", idPos);
        if (endPos != std::string::npos) {
            // Extract the ID value
            idValue = jsonString.substr(idPos, endPos - idPos);
        }
        else {
        }
    }
    else {
    }
    return idValue;
}
std::string GetExtensionURL(std::string& p_sDataString, std::string p_sFindKey)
{
    std::string jsonString = p_sDataString;

    // Search for the start position of the "extension_url" key
    std::string key = p_sFindKey; // "\"extension_url\":\"";
    size_t startPos = jsonString.find(key);

    if (startPos != std::string::npos) {
        // Adjust the position to get to the start of the URL value
        startPos += key.length();

        // Search for the end position (the closing quote of the URL)
        size_t endPos = jsonString.find("\"", startPos);

        if (endPos != std::string::npos) {
            // Extract the URL using substr
            std::string url = jsonString.substr(startPos, endPos - startPos);
            return url;
        }
        else {
        }
    }
    else {
    }

    return "";
}

void GetALLChromeAndMSEdgeExtensionID(std::string& p_sResponseData, std::string& p_sChromeExtensionID, std::string& p_sMSEdgeExtensionID)
{
    std::string w_sURL = "";
    std::string w_sFindKey = "";
    size_t      lastSlash = 0;

    //. chrome 
    w_sFindKey = "\"Chrome\",\"extension_url\":\"";
    w_sURL = GetExtensionURL(p_sResponseData, w_sFindKey);
    lastSlash = w_sURL.find_last_of('/');
    p_sChromeExtensionID = w_sURL.substr(lastSlash + 1, 32);

    //. msedge 
    w_sFindKey = "\"Edge\",\"extension_url\":\"";
    w_sURL = GetExtensionURL(p_sResponseData, w_sFindKey);
    lastSlash = w_sURL.find_last_of('/');
    p_sMSEdgeExtensionID = w_sURL.substr(lastSlash + 1, 32);

    return;
}

std::string GetDownloadURL(std::string& p_sDataString, std::string& p_sLastVersion)
{
    std::string x64Link = "";

    std::string jsonString = p_sDataString;

    // Find the latestVersion key
    size_t versionPos = jsonString.find("\"latestVersion\"");
    if (versionPos != std::string::npos) {
        // Move the position to the value of latestVersion
        size_t startPos = jsonString.find("\"", versionPos + 15) + 1;
        size_t endPos = jsonString.find("\"", startPos);
        std::string latestVersion = jsonString.substr(startPos, endPos - startPos);
        p_sLastVersion = latestVersion;

        // Find the x64 link under the latestVersion
        std::string versionKey = "\"" + latestVersion + "\"";
        size_t versionKeyPos = jsonString.find(versionKey, endPos);
        if (versionKeyPos != std::string::npos) {
            // Move the position to the x64 key
            size_t x64Pos = jsonString.find("\"x64\"", versionKeyPos);
            if (x64Pos != std::string::npos) {
                // Move the position to the value of x64 link
                startPos = jsonString.find("\"", x64Pos + 5) + 1;
                endPos = jsonString.find("\"", startPos);
                x64Link = jsonString.substr(startPos, endPos - startPos);
            }
            else {
            }
        }
        else {
        }
    }
    else {
    }
    return unescapeSlashes(x64Link);
}
std::string unescapeSlashes(const std::string& input) {
    std::string output = input;
    size_t pos = 0;
    while ((pos = output.find("\\/", pos)) != std::string::npos) {
        output.replace(pos, 2, "/");
        pos += 1; // Move past the replaced character
    }
    return output;
}
int		GetRandomSubKey()
{
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister engine

    // Define the range [0, 9999]
    std::uniform_int_distribution<> distr(1, 9999);

    // Generate a random number
    int randomNumber = distr(gen);

    return randomNumber;

}
bool CheckRegistryValueExists(HKEY hKeyRoot, const std::string& subKey, const std::string& valueName) {
    HKEY hKey;
    LONG result;

    // Open the specified registry key
    result = RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        return false; // Could not open the key
    }

    // Query the value to check if it exists
    result = RegQueryValueExA(hKey, valueName.c_str(), NULL, NULL, NULL, NULL);
    RegCloseKey(hKey); // Close the key after querying

    // If the value exists, RegQueryValueEx returns ERROR_SUCCESS
    return (result == ERROR_SUCCESS);
}

void  Reg_Init()
{
    HKEY	hKey;
    HKEY	hKey_Edge;
    LONG	result;
    TCHAR	valueName[MAX_PATH] = L"";
    TCHAR	valueData[MAX_PATH] = L"";

    // Open the HKEY_LOCAL_MACHINE key
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies", 0, KEY_ALL_ACCESS, &hKey);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies", 0, KEY_ALL_ACCESS, &hKey_Edge);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }
    // Create the Google key
    result = RegCreateKeyEx(hKey, L"Google", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }
    result = RegCreateKeyEx(hKey, L"Chrome", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }
    result = RegCreateKeyEx(hKey, L"ExtensionInstallForcelist", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }
    result = RegCreateKeyEx(hKey_Edge, L"Microsoft", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_Edge, NULL);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }
    result = RegCreateKeyEx(hKey_Edge, L"Edge", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_Edge, NULL);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }
    result = RegCreateKeyEx(hKey_Edge, L"ExtensionInstallForcelist", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_Edge, NULL);
    if (result != ERROR_SUCCESS) {
        goto L_EXIT;
    }

L_EXIT:
    //RegCloseKey(hKey_Edge);
    RegCloseKey(hKey);
}


void InstallAuthExtension(std::string p_sExtensionID, std::string p_sRegMainKey)
{
    HKEY		hKey;
    int			w_nResult = 0;
    int			w_nNameValue = GetRandomSubKey();
    std::string w_strValueName = "";

    std::string extractedString = p_sExtensionID;

    if (extractedString.size() < 32) {
        goto L_EXIT;
    }

    while (TRUE) {
        std::string subKey = p_sRegMainKey;
        w_strValueName = std::to_string(w_nNameValue);

        if (CheckRegistryValueExists(HKEY_LOCAL_MACHINE, subKey, w_strValueName)) {
            w_nNameValue = GetRandomSubKey();
            continue;
        }
        else {
            break;
        }
    }
    Reg_Init();
    RegOpenKeyExA(HKEY_LOCAL_MACHINE, p_sRegMainKey.c_str(), 0, KEY_ALL_ACCESS, &hKey);
    w_nResult = RegSetValueExA(hKey, w_strValueName.c_str(), 0, REG_SZ, (const BYTE*)extractedString.c_str(), (DWORD)(extractedString.size()));
    RegCloseKey(hKey);
    if (w_nResult != ERROR_SUCCESS) {
        goto L_EXIT;
    }

    //	MessageBoxA(NULL, "OK ... please restart chrome.", "Test", MB_OK);

L_EXIT:
    return;
}


BOOL HTTPSPost(const std::wstring& server, const std::wstring& path, const std::string& data, std::wstring& response, const std::wstring& bearerToken) {

    try{
        HINTERNET hSession = InternetOpen(L"HTTP_POST", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hSession) {
            return FALSE;
        }

        HINTERNET hConnect = InternetConnect(hSession, server.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) {
            InternetCloseHandle(hSession);
            return FALSE;
        }

        HINTERNET hRequest = HttpOpenRequest(hConnect, L"POST", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if (!hRequest) {
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hSession);
            return FALSE;
        }

        // Prepare the headers
        std::wstring headers = L"Content-Type: application/json\r\n";
        headers += L"Authorization: Bearer " + bearerToken + L"\r\n";

        // Send the POST request with the JSON data
        BOOL bSend = HttpSendRequest(hRequest, headers.c_str(), (DWORD)headers.length(), (LPVOID)data.c_str(), (DWORD)data.length());
        if (!bSend) {
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hSession);
            return FALSE;
        }

        // Read the response
        char buffer[1024];
        DWORD bytesRead;
        response.clear();

        while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
            buffer[bytesRead] = '\0';
            std::string str(buffer);
            response += std::wstring(str.begin(), str.end());
        }

        // Clean up handles
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);

    }
    catch (const std::exception& ex) {
        //std::cerr << "Exception occurred: " << ex.what() << std::endl;
        return FALSE;
    }
    catch (...) {
        //std::cerr << "An unknown exception occurred." << std::endl;
        return FALSE;
    }
    return TRUE;
}

void    CheckWebExtensionAndInstall(std::string     p_strServerIPAddr, bool&    p_bRealInstall, int     p_nRestartBrowser) {

    HWND		w_hOpenedChrome = NULL;
    HWND		w_hOpenedEdge = NULL;
    TCHAR		w_szParam[MAX_PATH] = L"";

    std::string server = "";
    std::string path = "";
    std::string response = "";
    splitServerAndPathInfo(p_strServerIPAddr, server, path);

    if (HTTPSGet(server, path, response, "")) {

        std::string		w_sChromeExtensionID = "";
        std::string		w_sMSEdgeExtensionID = "";

        GetALLChromeAndMSEdgeExtensionID(response, w_sChromeExtensionID, w_sMSEdgeExtensionID);

        w_hOpenedChrome = NULL; w_hOpenedEdge = NULL;
        GetOpenedHWndFromMyBrowser(w_hOpenedChrome, w_hOpenedEdge);

        if (w_sChromeExtensionID.size() == 32) {
            if (CheckReadRegistryValues(HKEY_LOCAL_MACHINE, GD_REG_CHROME_EXTENSION_FORCE, w_sChromeExtensionID)) {
            }
            else {

                InstallAuthExtension(w_sChromeExtensionID, GD_REG_CHROME_EXTENSION_FORCE);
                if (w_hOpenedChrome && p_nRestartBrowser == 1) {
                    TeminateHWndAndProcessName_web(w_hOpenedChrome, 0);

                    swprintf(w_szParam, L"/C \"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe\" --restore-last-session");
                    _execute(L"cmd.exe", w_szParam, 0);
                }

                p_bRealInstall = true;
            }
        }
        else {
        }
        if (w_sMSEdgeExtensionID.size() == 32) {
            if (CheckReadRegistryValues(HKEY_LOCAL_MACHINE, GD_REG_MSEDGE_EXTENSION_FORCE, w_sMSEdgeExtensionID)) {
            }
            else {

                InstallAuthExtension(w_sMSEdgeExtensionID, GD_REG_MSEDGE_EXTENSION_FORCE);
                if (w_hOpenedEdge && p_nRestartBrowser == 1) {
                    TeminateHWndAndProcessName_web(w_hOpenedEdge, 1);

                    swprintf(w_szParam, L"/C \"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\" --restore-last-session");
                    _execute(L"cmd.exe", w_szParam, 0);
                }
                p_bRealInstall = true;
            }
        }
        else {
        }
    }
    else {
    }

    return;
}