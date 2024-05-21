//
// Created by czq on 5/20/2024.
//

#ifndef BLUETOOTHEXT_UTILS_H
#define BLUETOOTHEXT_UTILS_H

#include <cstdio>
#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include <any>

struct ValueData {
    DWORD type;
    std::vector<BYTE> data;
};

bool ReadRegistryValues(HKEY hKey, std::map<std::wstring, ValueData> &mapData);

bool ReadRegistryValues(HKEY hKey, std::map<std::wstring, std::any> &keyMap);

bool ReadRegistryKey(HKEY hKey, std::map<std::wstring, HKEY> &keyMap);

std::wstring ReadStringFromRegistry(HKEY hKey, const std::wstring &subKey, const std::wstring &valueName);

DWORD ReadIntFromRegistry(HKEY hKey, const std::wstring &subKey, const std::wstring &valueName);

bool DeleteRegistryKey(HKEY hKey, const wchar_t *keyPath);

int convertToLogLevel(int level);

FILE *openLogFile();

int initLog();

errno_t openFile(FILE **pFile);

std::map<std::string, std::string> parseJson(const std::string &jsonString);

#endif //BLUETOOTHEXT_UTILS_H
