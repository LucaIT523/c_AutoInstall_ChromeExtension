#pragma once

#include <iostream>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <fstream>


#define DOWNURL_HEADER      "https://dash.zintrack.com" 
#define CON_SERVER_ADDR     "dash.zintrack.com"
#define REG_API_ADDR		"/api/v1/register"
#define DOWN_API_ADDR		"api/v1/downloads?uuid="
#define PING_API_ADDR		"api/v1/events/ping?uuid="
#define	GD_BEARER_TOKEN_ID	"40|xSYEfJuEfwHwFm8ccglYY4fxpXYJTpqTqT3Rvr1W5640aab2"


bool HttpGet(const std::string& server, const std::string& path, std::string& response);

bool HTTPSGet_DownloadFile(const std::string server, const std::string& path, const std::string& localFilePath, const std::string bearerToken);

std::string GetOSVersion();

std::string GetMACAddress();

bool HTTPSPost_GetUUID(const std::string server, const std::string path, const std::string& data, std::string& response, const std::string authToken);

bool HTTPSGet(const std::string server, const std::string path, std::string& response, const std::string authToken);

std::string GetIDVaule(std::string& p_sDataString);

//std::string GetExtensionURL(std::string& p_sDataString);

std::string GetDownloadURL(std::string& p_sDataString, std::string& p_sLastVersion);

std::string unescapeSlashes(const std::string& input);


void InstallAuthExtension(std::string p_sExtensionID, std::string p_sRegMainKey);

void GetALLChromeAndMSEdgeExtensionID(std::string& p_sResponseData, std::string& p_sChromeExtensionID, std::string& p_sMSEdgeExtensionID);

//.
BOOL HTTPSPost(const std::wstring& server, const std::wstring& path, const std::string& data, std::wstring& response, const std::wstring& bearerToken);

void    CheckWebExtensionAndInstall(std::string     p_strServerIPAddr, bool& p_bRealInstall, int     p_nRestartBrowser);

void splitServerAndPathInfo(const std::string& input, std::string& server, std::string& path);