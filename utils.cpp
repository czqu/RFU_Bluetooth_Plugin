//
// Created by czq on 5/20/2024.
//

#include <sstream>
#include "utils.h"
#include "log.h"
#include "bluetooth_ext.h"
#include "rfu_error.h"
#include <algorithm>
errno_t openFile(FILE **pFile, wchar_t const *filePath) {
    auto err = _wfopen_s(pFile, filePath, L"a+");
    if (err == 0) {
        return err;
    }
    //todo Add logic to utilize temporary folder
    return err;
}

FILE *openLogFile() {
    auto logPath = ReadStringFromRegistry(HKEY_LOCAL_MACHINE, REGISTRY_PATH, L"log_path");
    FILE *logFile;

    errno_t err = openFile(&logFile, logPath.c_str());
    if (err == 0) {

        return logFile;
    } else {
        return nullptr;
    }
}

int convertToLogLevel(int level) {


    if (level <= 1) {
        return LOG_FATAL;
    }
    if (level <= 2) {
        return LOG_ERROR;
    }
    if (level <= 3) {
        return LOG_WARN;
    }
    if (level <= 115) {
        return LOG_INFO;
    }
    if (level <= 2083) {
        return LOG_DEBUG;
    }
    return LOG_TRACE;

}

int initLog() {

    int level = (int) ReadIntFromRegistry(HKEY_LOCAL_MACHINE, REGISTRY_PATH, L"log_level");

    level = convertToLogLevel(level);
    auto logFile = openLogFile();
    if (logFile == nullptr) {
        return ERROR_SYSTEM_INTERNAL_ERROR;
    }

    log_set_level(level);
    if (level > LOG_DEBUG) {
        log_set_quiet(true);

    } else {
        log_add_fp(stdout, LOG_DEBUG);
    }

    log_add_fp(logFile, level);

    return ERROR_SUCCESS;

}

bool ReadRegistryKey(HKEY hKey, std::map<std::wstring, HKEY> &keyMap) {

    DWORD subKeyCount = 0;
    DWORD maxSubKeyNameLen = 0;
    DWORD subKeyNameLen = 0;
    wchar_t subKeyName[256];
    HKEY subKey = NULL;


    if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &subKeyCount, &maxSubKeyNameLen, NULL, NULL, NULL, NULL, NULL, NULL) !=
        ERROR_SUCCESS) {
        return false;
    }


    for (DWORD i = 0; i < subKeyCount; i++) {

        subKeyNameLen = maxSubKeyNameLen + 1;

        if (RegEnumKeyExW(hKey, i, subKeyName, &subKeyNameLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
            return false;
        }

        if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &subKey) != ERROR_SUCCESS) {
            return false;
        }

        keyMap[std::wstring(subKeyName)] = subKey;
    }


    return true;
}


bool ReadRegistryValues(HKEY hKey, std::map<std::wstring, ValueData> &mapData) {

    DWORD dwValueCount = 0, dwMaxValueNameLen = 0, dwMaxValueDataLen = 0;
    LONG lRet = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
                                &dwValueCount, &dwMaxValueNameLen, &dwMaxValueDataLen, NULL, NULL);
    if (lRet != ERROR_SUCCESS) {
        return false;
    }

    for (DWORD i = 0; i < dwValueCount; i++) {

        std::vector<wchar_t> valueName(dwMaxValueNameLen + 1);
        std::vector<BYTE> valueData(dwMaxValueDataLen);
        DWORD nameLen = dwMaxValueNameLen + 1;
        DWORD dataLen = dwMaxValueDataLen;
        DWORD type = 0;

        lRet = RegEnumValueW(hKey, i, valueName.data(), &nameLen,
                             NULL, &type, valueData.data(), &dataLen);
        if (lRet != ERROR_SUCCESS) {
            return false;
        }

        if (type == REG_SZ || type == REG_DWORD || type == REG_QWORD || type == REG_BINARY) {

            ValueData vd;
            vd.type = type;
            vd.data.assign(valueData.begin(), valueData.begin() + dataLen);

            mapData.insert({valueName.data(), vd});
        }
    }
    return true;
}


bool ReadRegistryValues(HKEY hKey, std::map<std::wstring, std::any> &keyMap) {
    std::map<std::wstring, ValueData> mapData;
    bool result = ReadRegistryValues(hKey, mapData);
    if (!result) {
        return result;
    }
    for (const auto &[key, value]: mapData) {
        switch (value.type) {
            case REG_SZ:
                keyMap[key] = std::wstring(reinterpret_cast<const wchar_t *>(value.data.data()));
                break;
            case REG_DWORD:
                keyMap[key] = *reinterpret_cast<const DWORD *>(value.data.data());
                break;
            case REG_QWORD:
                keyMap[key] = *reinterpret_cast<const ULONGLONG *>(value.data.data());
                break;
            case REG_BINARY:
                keyMap[key] = value.data;
                break;
        }
    }
    return result;
}

std::wstring ReadStringFromRegistry(HKEY hKey, const std::wstring &subKey, const std::wstring &valueName) {
    HKEY hSubKey;
    if (RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
        DWORD dataSize = 0;
        if (RegQueryValueExW(hSubKey, valueName.c_str(), nullptr, nullptr, nullptr, &dataSize) == ERROR_SUCCESS) {
            std::wstring result;
            result.resize(dataSize / sizeof(wchar_t));

            if (RegQueryValueExW(hSubKey, valueName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(&result[0]),
                                 &dataSize) == ERROR_SUCCESS) {
                RegCloseKey(hSubKey);
                return result;
            }
        }

        RegCloseKey(hSubKey);
    }

    return L"";
}


DWORD ReadIntFromRegistry(HKEY hKey, const std::wstring &subKey, const std::wstring &valueName) {
    HKEY hSubKey;
    if (RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
        DWORD result = 0;
        DWORD dataSize = sizeof(DWORD);

        if (RegQueryValueExW(hSubKey, valueName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(&result),
                             &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hSubKey);
            return result;
        }

        RegCloseKey(hSubKey);
    }

    return 0;
}

std::map<std::string, std::string> parseJson(const std::string &jsonString) {
    //todo have bug
    std::map<std::string, std::string> dataMap;


    std::string trimmedJsonString;
    for (char c: jsonString) {
        if (c != ' ' && c != '\n') {
            trimmedJsonString += c;
        }
    }


    if (trimmedJsonString.front() == '{' && trimmedJsonString.back() == '}') {
        trimmedJsonString = trimmedJsonString.substr(1, trimmedJsonString.length() - 2);
    }


    std::stringstream ss(trimmedJsonString);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        std::stringstream pairStream(pair);
        std::string key, value;
        std::getline(pairStream, key, ':');
        std::getline(pairStream, value);
        key.erase(0, key.find_first_not_of('\"'));
        key.erase(key.find_last_not_of('\"') + 1);
        value.erase(0, value.find_first_not_of('\"'));
        value.erase(value.find_last_not_of('\"') + 1);
        size_t found = value.find("\\");
        while (found != std::string::npos) {
            value.replace(found, 2, "\\");
            found = value.find("\\", found + 1);
        }
        dataMap[key] = value;
    }

    return dataMap;
}