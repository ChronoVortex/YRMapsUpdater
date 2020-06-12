/**
 * @file initfuncwrap.h
 * @brief Wrappers for PrivateProfile functions to take strings.
 * @author Chrono Vortex#9916@Discord
 */

#ifndef INITFUNCWRAP_H
#define INITFUNCWRAP_H

#include <windows.h>

DWORD GetPrivateProfileString(const std::string& lpAppName, const std::string& lpKeyName, const std::string& lpDefault, char* lpReturnedString, const DWORD& nSize, const std::string& lpFileName) {
	return GetPrivateProfileString(lpAppName.c_str(), lpKeyName.c_str(), lpDefault.c_str(), lpReturnedString, nSize, lpFileName.c_str());
}

bool WritePrivateProfileString(const std::string& lpAppName, const std::string& lpKeyName, const std::string& lpString, const std::string& lpFileName) {
	return WritePrivateProfileStringA(lpAppName.c_str(), lpKeyName.c_str(), lpString.c_str(), lpFileName.c_str());
}

DWORD GetPrivateProfileSection(const std::string& lpAppName, char* lpReturnedString, DWORD nSize, const std::string& lpFileName) {
	return GetPrivateProfileSection(lpAppName.c_str(), lpReturnedString, nSize, lpFileName.c_str());
}

bool WritePrivateProfileSection(const std::string& lpAppName, const char* lpString, const std::string& lpFileName) {
	return WritePrivateProfileSectionA(lpAppName.c_str(), lpString, lpFileName.c_str());
}

#endif