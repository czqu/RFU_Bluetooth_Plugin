#pragma once
#include <set>
#include <string>
#define PLUGIN_INITIALIZE_PLUGIN_NAME "plugin_initialize"
#define PLUGIN_API_REGISTER_PLUGIN_NAME "plugin_api_register"
#define PLUGIN_RUN_PLUGIN_NAME "plugin_run"
#define PLUGIN_RELEASE_PLUGIN_NAME "plugin_release"

extern "C" __declspec(dllexport) int plugin_initialize(const wchar_t* configPath, int length);


extern "C" __declspec(dllexport) int plugin_api_register(char** apis, int* length);

extern "C" __declspec(dllexport) int plugin_run();

extern "C" __declspec(dllexport) int plugin_release();


#define REGISTER_UNLOCK_CALLBACK_NAME "register_unlock_callback"

typedef int (*unlockCallback)(const wchar_t*, const wchar_t*);

extern "C" __declspec(dllexport) int register_unlock_callback(unlockCallback c);

