//
// Created by czq on 5/20/2024.
//
#include <winsock2.h>
#include "bluetooth_ext.h"
#include <ws2bth.h>
#include <windows.h>
#include <strsafe.h>
#include <locale>
#include <chrono>
#include <codecvt>
#include <mutex>
#include "log.h"
#include "utils.h"
#include "rfu_error.h"

extern "C" __declspec(dllexport) int plugin_initialize(const wchar_t *configPath, int length);


extern "C" __declspec(dllexport) int plugin_api_register(char **apis, int *length);

extern "C" __declspec(dllexport) int plugin_run();

extern "C" __declspec(dllexport) int plugin_release();


typedef int (*unlockCallback)(const wchar_t *, const wchar_t *);

extern "C" __declspec(dllexport) int register_unlock_callback(unlockCallback c);


extern "C" const GUID __declspec(selectany) g_guidServiceClass = {0x4E5877C0, 0x8297, 0x4AAE,
                                                                  {0xB7, 0xBD, 0x73, 0xA8, 0xCB, 0xC1, 0xED, 0xAF}};
SOCKET g_btSocket = INVALID_SOCKET;

HANDLE g_hAcceptThread = NULL;


int g_ulMaxCxnCycles = 10L;//only receive 10 times every 10 sec
int lock_time = 10 * 1000;
char *g_apis_string = nullptr;
callback g_callback = nullptr;
bool volatile g_allow_run = false;
using std::string;

DWORD WINAPI BluetoothReceiverThread(LPVOID lpParam) {
    log_info(L"recive start");
    auto start = std::chrono::high_resolution_clock::now();
    char *pszDataBufferIndex = NULL;
    char pszDataBuffer[CXN_TRANSFER_DATA_LENGTH] = {0};
    ULONG ulRetCode = CXN_SUCCESS;
    auto clientSocket = (SOCKET) lpParam;
    if (INVALID_SOCKET == clientSocket) {
        ulRetCode = CXN_ERROR;
        log_warn(L"socket close:%d", WSAGetLastError());
        return ulRetCode;
    }

    pszDataBufferIndex = pszDataBuffer;

    int uiTotalLengthReceived = 0;
    BOOL bContinue = TRUE;
    while (bContinue && (uiTotalLengthReceived < CXN_TRANSFER_DATA_LENGTH)) {

        auto iLengthReceived = recv(clientSocket,
                                    (char *) pszDataBufferIndex,
                                    (CXN_TRANSFER_DATA_LENGTH - uiTotalLengthReceived),
                                    0);

        switch (iLengthReceived) {
            case 0:
                bContinue = FALSE;
                break;

            case SOCKET_ERROR:

                log_warn(L"socket close", WSAGetLastError());
                bContinue = FALSE;
                ulRetCode = CXN_ERROR;
                break;

            default:

                if (iLengthReceived > (CXN_TRANSFER_DATA_LENGTH - uiTotalLengthReceived)) {

                    bContinue = FALSE;
                    ulRetCode = CXN_ERROR;
                    break;

                }
                pszDataBufferIndex += iLengthReceived;
                uiTotalLengthReceived += iLengthReceived;
                break;
        }
    }
    closesocket(clientSocket);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    log_info(L"recive over,cost %lld nanoseconds\n", duration.count());
    std::string json = string(pszDataBuffer);
    handleData(json);
    log_info(L"handle over");
    return ulRetCode;
}

DWORD handleData(string &json) {
    std::map<std::string, std::string> keyValuePairs = parseJson(json);
    string username;
    string password;
    for (const auto &pair: keyValuePairs) {
        if (pair.first == "username") {
            username = pair.second;
        }
        if (pair.first == "passwd") {
            password = pair.second;
        }
    }
    log_debug(L"usr: %d\n",username.length());
    log_debug(L"pwd: %d\n",password.length());
    if (username.empty() || password.empty()) {
        return ERROR_USER_LOGIN_FAILURE;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::wstring wide_username = converter.from_bytes(username);
    std::wstring wide_password = converter.from_bytes(password);

    if (g_callback != nullptr) {
        g_callback(wide_username.c_str(), wide_password.c_str());
    }

}

DWORD WINAPI BluetoothAcceptThread(LPVOID lpParam) {

    int iCxnCount = 0;
    ULONG ulRetCode = CXN_SUCCESS;

    if (SOCKET_ERROR == listen(g_btSocket, CXN_DEFAULT_LISTEN_BACKLOG)) {
        log_error(L"%d", WSAGetLastError());
        ulRetCode = CXN_ERROR;
        return ulRetCode;
    }


    unsigned long iMaxCxnCycles = g_ulMaxCxnCycles;
    if (CXN_SUCCESS == ulRetCode) {

        for (iCxnCount = 0;
             (CXN_SUCCESS == ulRetCode) && ((iCxnCount < iMaxCxnCycles) || (iMaxCxnCycles == 0));
             iCxnCount++) {

            auto clientSocket = accept(g_btSocket, NULL, NULL);
            if (!g_allow_run) {
                closesocket(clientSocket);
                return CXN_ERROR;
            }
            auto hRecvThread = CreateThread(NULL, 0, BluetoothReceiverThread, (LPVOID) clientSocket, 0, NULL);
            if (hRecvThread == NULL) {
                log_error(L"create thread err", WSAGetLastError());
                closesocket(clientSocket);
                ulRetCode = CXN_ERROR;
                break;
            }
            log_info(L"create thread succ");
            CloseHandle(hRecvThread);
            if (iCxnCount == iMaxCxnCycles - 1) {
                Sleep(lock_time);
                iCxnCount = -1;
            }
        }

    }

    log_info(L"listen exit %d", ulRetCode);
    return ulRetCode;
}

int plugin_initialize(const wchar_t *configPath, int length) {
    log_info(L"ble ext log init");
    return initLog();


};

int plugin_run() {
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    log_info(L"ble ext run");

    g_allow_run = true;
    WSADATA WSAData = {0};
    ULONG ulRetCode = CXN_SUCCESS;

    ulRetCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (ulRetCode == CXN_ERROR) {
        return ERROR_SYSTEM_BLUETOOTH_INIT_FAILURE;
    }

    int iAddrLen = sizeof(SOCKADDR_BTH);


    WSAQUERYSETW wsaQuerySet = {0};
    SOCKADDR_BTH SockAddrBthLocal = {0};
    LPCSADDR_INFO lpCSAddrInfo = NULL;


    lpCSAddrInfo = (LPCSADDR_INFO) HeapAlloc(GetProcessHeap(),
                                             HEAP_ZERO_MEMORY,
                                             sizeof(CSADDR_INFO));
    if (NULL == lpCSAddrInfo) {

        ulRetCode = CXN_ERROR;
        return ulRetCode;
    }


    if (CXN_SUCCESS == ulRetCode) {
        g_btSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        if (INVALID_SOCKET == g_btSocket) {
            ulRetCode = CXN_ERROR;
            return ERROR_SYSTEM_BLUETOOTH_INIT_FAILURE;
        }
    }

    if (CXN_SUCCESS == ulRetCode) {
        SockAddrBthLocal.addressFamily = AF_BTH;
        SockAddrBthLocal.port = BT_PORT_ANY;

        if (SOCKET_ERROR == bind(g_btSocket,
                                 (struct sockaddr *) &SockAddrBthLocal,
                                 sizeof(SOCKADDR_BTH))) {

            ulRetCode = CXN_ERROR;
            return ERROR_SYSTEM_BLUETOOTH_INIT_FAILURE;
        }
    }

    if (CXN_SUCCESS == ulRetCode) {

        ulRetCode = getsockname(g_btSocket,
                                (struct sockaddr *) &SockAddrBthLocal,
                                &iAddrLen);
        if (SOCKET_ERROR == ulRetCode) {

            ulRetCode = CXN_ERROR;
            return ERROR_SYSTEM_BLUETOOTH_INIT_FAILURE;
        }
    }
    if (CXN_SUCCESS == ulRetCode) {
        lpCSAddrInfo[0].LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
        lpCSAddrInfo[0].LocalAddr.lpSockaddr = (LPSOCKADDR) &SockAddrBthLocal;
        lpCSAddrInfo[0].RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
        lpCSAddrInfo[0].RemoteAddr.lpSockaddr = (LPSOCKADDR) &SockAddrBthLocal;
        lpCSAddrInfo[0].iSocketType = SOCK_STREAM;
        lpCSAddrInfo[0].iProtocol = BTHPROTO_RFCOMM;

        ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSETW));
        wsaQuerySet.dwSize = sizeof(WSAQUERYSETW);
        wsaQuerySet.lpServiceClassId = (LPGUID) &g_guidServiceClass;


    }


    if (CXN_SUCCESS == ulRetCode) {

        wsaQuerySet.lpszServiceInstanceName = (LPWSTR) L"RemoteFingerprint Service ";
        wsaQuerySet.lpszComment = (LPWSTR) L"RemoteFingerprint Service ";
        wsaQuerySet.dwNameSpace = NS_BTH;
        wsaQuerySet.dwNumberOfCsAddrs = 1;
        wsaQuerySet.lpcsaBuffer = lpCSAddrInfo;

        if (SOCKET_ERROR == WSASetServiceW(&wsaQuerySet, RNRSERVICE_REGISTER, 0)) {
            ulRetCode = CXN_ERROR;
            return ERROR_SYSTEM_BLUETOOTH_INIT_FAILURE;
        }
    }
    if (!g_allow_run) {
        return CXN_ERROR;
    }
    g_hAcceptThread = CreateThread(NULL, 0, BluetoothAcceptThread, NULL, 0, NULL);

    if (g_hAcceptThread == NULL) {
        log_error(L"CreateThread for accept failed: %d\n", GetLastError());
        closesocket(g_btSocket);
        WSACleanup();

        return ERROR_SYSTEM_BLUETOOTH_INIT_FAILURE;
    }
    log_info(L"create accept thread succ");

    CloseHandle(g_hAcceptThread);
    return ulRetCode == CXN_SUCCESS ? SUCCESS : ERROR_SYSTEM_BLUETOOTH_INIT_FAILURE;
}

int plugin_release() {
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    g_allow_run = false;
    log_info(L"ble ext stop");

    closesocket(g_btSocket);

    WSACleanup();
    if (g_apis_string != nullptr) {
        free(g_apis_string);
    }
    log_info(L"ble ext stop");
    return SUCCESS;
}

int plugin_api_register(char **apis_string, int *length) {
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    const char *api_string_const = "plugin_initialize,plugin_api_register,plugin_run,plugin_release,register_unlock_callback";
    int len = (int) strlen(api_string_const);
    if (length != nullptr) {
        *length = len;
    }

    *apis_string = (char *) malloc((len + 1) * sizeof(char));

    if (*apis_string == nullptr) {
        return ERROR_SYSTEM_INSUFFICIENT_MEMORY;
    }

    if (strcpy_s(*apis_string, len + 1, api_string_const) != 0) {
        free(*apis_string);
        return ERROR_SYSTEM_INSUFFICIENT_MEMORY;
    }
    g_apis_string = *apis_string;
    return ERROR_SUCCESS;
}

typedef int (*callback)(const wchar_t *, const wchar_t *);

int register_unlock_callback(unlockCallback p) {
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    if (p == nullptr) {
        return ERROR_UNKNOWN;
    }
    g_callback = p;
    return ERROR_SUCCESS;
}