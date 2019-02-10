#include <curl/curl.h>
#include "sdk/amxxmodule.h"
#include "amx_curl_controller_class.h"
#include "asio_poller.h"

extern AMX_NATIVE_INFO g_amx_curl_natives[];

// amxmodx

void OnAmxxAttach()
{
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if(res != CURLE_OK)
        MF_PrintSrvConsole("[CURL] Cannot init curl: ", curl_easy_strerror(res));
    else
        MF_AddNatives(g_amx_curl_natives);
}

// metamod

void StartFrame()
{
    AmxCurlController::Instance().get_asio_poller().Poll();

    SET_META_RESULT(MRES_IGNORED);
}

void ServerDeactivate()
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    manager.TryInterruptAllTransfers();
    manager.WaitAllTransfers();
    manager.RemoveAllTasks();

    SET_META_RESULT(MRES_IGNORED);
}
