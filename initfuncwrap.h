/**
 * @file initfuncwrap.h
 * @brief Wrappers for PrivateProfile functions to take strings.
 * @author Chrono Vortex#9916@Discord
 */

#pragma once

#include <windows.h>
#include <filesystem>

DWORD GetPrivateProfileString(const std::string& lpAppName, const std::string& lpKeyName, const std::string& lpDefault, char* lpReturnedString, const DWORD& nSize, const std::filesystem::path& lpFileName) {
	return GetPrivateProfileString(lpAppName.c_str(), lpKeyName.c_str(), lpDefault.c_str(), lpReturnedString, nSize, lpFileName.string().c_str());
}

bool WritePrivateProfileString(const std::string& lpAppName, const std::string& lpKeyName, const std::string& lpString, const std::filesystem::path& lpFileName) {
	return WritePrivateProfileStringA(lpAppName.c_str(), lpKeyName.c_str(), lpString.c_str(), lpFileName.string().c_str());
}

DWORD GetPrivateProfileSection(const std::string& lpAppName, char* lpReturnedString, DWORD nSize, const std::filesystem::path& lpFileName) {
	return GetPrivateProfileSection(lpAppName.c_str(), lpReturnedString, nSize, lpFileName.string().c_str());
}

bool WritePrivateProfileSection(const std::string& lpAppName, const char* lpString, const std::filesystem::path& lpFileName) {
	return WritePrivateProfileSectionA(lpAppName.c_str(), lpString, lpFileName.string().c_str());
}
