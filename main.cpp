#include <iostream>
#include <windows.h>
#include "rfu_ext_api.h"
#include "rfu_error.h"
#include "bluetooth_ext.h"
int main() {

    int ret = ERROR_UNKNOWN;
    ret = plugin_run();
    if (ret != ERROR_SUCCESS) {
        exit(1);
    }

    char *api_string = nullptr;
    int length = 0;
    ret = plugin_api_register(&api_string, &length);
    if (ret != ERROR_SUCCESS && api_string == nullptr) {
        free(api_string);
        exit(1);
    }
    if (ret != ERROR_SUCCESS) {
        exit(1);
    }

    ret = register_unlock_callback([](const wchar_t *username, const wchar_t *password) -> int {

        std::wcout << L"Username: " << username << L", Password: " << password << std::endl;
        return ERROR_SUCCESS;
    });
    if (ret != ERROR_SUCCESS) {
        exit(1);
    }

    Sleep(100000000);
    ret = plugin_release();
    if (ret != ERROR_SUCCESS) {
        exit(1);
    }
    return 0;

}