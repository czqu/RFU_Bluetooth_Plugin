//
// Created by czq on 5/20/2024.
//

#ifndef BLUETOOTHEXT_BLUETOOTH_EXT_H
#define BLUETOOTHEXT_BLUETOOTH_EXT_H


#include <string>

DWORD WINAPI BluetoothAcceptThread(LPVOID lpParam);

DWORD WINAPI BluetoothReceiverThread(LPVOID lpParam);

#define  SUCCESS                     0

#define CXN_TRANSFER_DATA_LENGTH          8192
#define CXN_SUCCESS                       0
#define CXN_ERROR                         1
#define CXN_DEFAULT_LISTEN_BACKLOG        4

#define REGISTRY_PATH    L"SOFTWARE\\RemoteFingerUnlock\\BluetoothExt"

typedef int (*callback)(const wchar_t *, const wchar_t *);

DWORD handleData(std::string &json);

#endif //BLUETOOTHEXT_BLUETOOTH_EXT_H
