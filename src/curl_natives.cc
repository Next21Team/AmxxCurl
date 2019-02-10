#include "sdk/amxxmodule.h"

#include "curl_class.h"
#include "amx_curl_manager_class.h"
#include "amx_curl_controller_class.h"

int g_len;

// params[1]		CURL handle     cell
// params[2]		char* url		str
// params[3]						str ref | escaped url
// params[4]		int len			cell | размер дестенейшн массива
static cell AMX_NATIVE_CALL amx_curl_easy_escape(AMX* amx, cell* params)
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    AmxCurlManager::AmxCurlHandle curl_handle = params[1];

    try
    {
        std::string escaped_str;
        char* str_to_escape = MF_GetAmxString(amx, params[2], 0, &g_len);
        manager.CurlEscapeUrl(curl_handle, str_to_escape, escaped_str);

        MF_SetAmxString(amx, params[3], escaped_str.c_str(), params[4]);
    }
    catch(const CurlAmxManagerInvalidHandleException&)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid curl handle");
    }

    return 0;
}

// params[1]		CURL handle		    cell
// params[2]		char* url			str
// params[3]							str ref | escaped url
// params[4]		int len				cell | размер дестенейшн массива
// ret				int new_len			cell | реальный размер нового массива
static cell AMX_NATIVE_CALL amx_curl_easy_unescape(AMX* amx, cell* params)
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    AmxCurlManager::AmxCurlHandle curl_handle = params[1];

    try
    {
        std::string unescaped_str;
        char* str_to_unescape = MF_GetAmxString(amx, params[2], 0, &g_len);
        manager.CurlEscapeUrl(curl_handle, str_to_unescape, unescaped_str);

        MF_SetAmxString(amx, params[3], unescaped_str.c_str(), params[4]);

        return unescaped_str.size();
    }
    catch (const CurlAmxManagerInvalidHandleException&)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid curl handle");
    }

    return 0;
}

// ret              CURL                cell
static cell AMX_NATIVE_CALL amx_curl_easy_init(AMX* amx, cell* params)
{
    try
    {
        return static_cast<cell>(AmxCurlController::Instance().get_curl_manager().CreateCurl(amx));
    }
    catch(const CurlInitFailtureException&)
    {
        return 0;
    }
}

// params[1]		CURL  handle		cell
// params[2]		CURLoption option	cell
// params[3]		...					str,cell
// ret				CURLcode			cell
static cell AMX_NATIVE_CALL amx_curl_easy_setopt(AMX* amx, cell* params)
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    AmxCurlManager::AmxCurlHandle curl_handle = params[1];
    CURLoption option = static_cast<CURLoption>(params[2]);

    try
    {
        // CURLOPTTYPE_LONG
        if (option < CURLOPTTYPE_OBJECTPOINT)
            return manager.CurlSetOption(curl_handle, option, static_cast<long>(*MF_GetAmxAddr(amx, params[3])));

        // CURLOPTTYPE_OBJECTPOINT
        if (option < CURLOPT_WRITEFUNCTION)
        {
            if (CurlUtils::IsDataOption(option)) {
                return manager.CurlSetOption(curl_handle, option, reinterpret_cast<void*>(*MF_GetAmxAddr(amx, params[3])));
            }

            switch (option) {
            case CURLOPT_HTTPPOST:
                return manager.CurlSetOption(curl_handle, option, reinterpret_cast<curl_httppost*>(*MF_GetAmxAddr(amx, params[3])));
            case CURLOPT_HTTPHEADER:
            case CURLOPT_PROXYHEADER:
            case CURLOPT_HTTP200ALIASES:
            case CURLOPT_MAIL_RCPT:
            case CURLOPT_QUOTE:
            case CURLOPT_POSTQUOTE:
            case CURLOPT_RESOLVE:
            case CURLOPT_TELNETOPTIONS:
                return manager.CurlSetOption(curl_handle, option, reinterpret_cast<curl_slist*>(*MF_GetAmxAddr(amx, params[3])));

            default:
                return manager.CurlSetOption(curl_handle, option, MF_GetAmxString(amx, params[3], 0, &g_len));
            }
        }

        // CURLOPT_WRITEFUNCTION
        if (option < CURLOPTTYPE_OFF_T)
        {
            manager.CurlSetupAmxCallback(curl_handle, option, MF_GetAmxString(amx, params[3], 0, &g_len)); 
            return CURLE_OK;
        }

        // CURLOPTTYPE_OFF_T
        curl_off_t val = 0;
        cell* p = MF_GetAmxAddr(amx, params[3]);

        val = p[0];
        val <<= 32;
        val |= p[1];

        return manager.CurlSetOption(curl_handle, option, val);
    }
    catch (const CurlInvalidOptionException& ex)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "%s. Option: %d", ex.what(), ex.get_option());
    }
    catch(const CurlAmxManagerInvalidHandleException&)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid curl handle");
    }

    return -1;
}

// params[1]		CURL handle		cell
// params[2]		CURLINFO info	cell
// params[3]		...				cell,str,float ref
// params[4]		int				len | for string in third param
// ret				CURLcode		cell
static cell AMX_NATIVE_CALL amx_curl_easy_getinfo(AMX* amx, cell* params)
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    AmxCurlManager::AmxCurlHandle curl_handle = params[1];
    CURLINFO curl_info = static_cast<CURLINFO>(params[2]);

    CURLcode ret_code;

    int curlinfo_mask = curl_info & CURLINFO_TYPEMASK;

    try
    {
        if (curlinfo_mask == CURLINFO_STRING)
        {
            char* str;
            ret_code = manager.CurlGetInfo(curl_handle, curl_info, str);

            if (ret_code == CURLE_OK)
                MF_SetAmxString(amx, params[3], str, params[4]);
        }
        else if (curlinfo_mask == CURLINFO_LONG || curlinfo_mask == CURLINFO_SOCKET)
        {
            long num;
            ret_code = manager.CurlGetInfo(curl_handle, curl_info, num);

            cell* ret = MF_GetAmxAddr(amx, params[3]);
            *ret = num;
        }
        else if (curlinfo_mask == CURLINFO_DOUBLE)
        {
            double num;
            ret_code = manager.CurlGetInfo(curl_handle, curl_info, num);

            cell* ret = MF_GetAmxAddr(amx, params[3]);
            *ret = amx_ftoc(num);
        }
        else if (curlinfo_mask == CURLINFO_SLIST) {
            curl_slist* csl;
            ret_code = manager.CurlGetInfo(curl_handle, curl_info, csl);

            cell* ret = MF_GetAmxAddr(amx, params[3]);
            *ret = reinterpret_cast<cell>(csl);
        }
        else
        {
            MF_LogError(amx, AMX_ERR_NATIVE, "Invalid CURLINFO");
            return 0;
        }

        return static_cast<cell>(ret_code);
    }
    catch(const CurlAmxManagerInvalidHandleException&)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid curl handle");
    }

    return 0;
}

// params[1]		CURL  handle		cell
// params[2]							str  | complite callback  public callback(CURL:curl, CURLcode:code, data);
// params[3]              data[]        cells arr
// params[3]              data len      cell
static cell AMX_NATIVE_CALL amx_curl_easy_perform(AMX* amx, cell* params)
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    AmxCurlManager::AmxCurlHandle curl_handle = params[1];
    cell* data = nullptr;
    int data_len = static_cast<int>(params[4]);

    if (data_len > 0)
    {
        data = new cell[data_len];
        MF_CopyAmxMemory(data, MF_GetAmxAddr(amx, params[3]), data_len);
    }

    try
    {
        manager.CurlPerformTask(curl_handle, MF_GetAmxString(amx, params[2], 0, &g_len), data, data_len);
    }
    catch (const CurlAmxManagerInvalidHandleException&)
    {
        delete[] data;

        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid curl handle");
    }

    return 0;
}

// params[1]		CURL  handle	   cell
static cell AMX_NATIVE_CALL amx_curl_easy_cleanup(AMX* amx, cell* params)
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    AmxCurlManager::AmxCurlHandle curl_handle = params[1];

    try
    {
        manager.RemoveTask(curl_handle);
    }
    catch (const CurlAmxManagerInvalidHandleException&)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid curl handle");
    }

    return 0;
}

// params[1]		CURL  handle	    cell
static cell AMX_NATIVE_CALL amx_curl_easy_reset(AMX* amx, cell* params)
{
    AmxCurlManager& manager = AmxCurlController::Instance().get_curl_manager();

    AmxCurlManager::AmxCurlHandle curl_handle = params[1];

    try
    {
        manager.CurlReset(curl_handle);
    }
    catch (const CurlAmxManagerInvalidHandleException&)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid curl handle");
    }

    return 0;
}

// params[1]		CURLcode err		cell
// params[2]		char* strerr		str
// params[3]		int len				cell
// ret
static cell AMX_NATIVE_CALL amx_curl_easy_strerror(AMX* amx, cell* params)
{
    MF_SetAmxString(amx, params[2], curl_easy_strerror(static_cast<CURLcode>(params[1])), params[3]);

    return 0;
}

// params[1]		curl_httppost*first	cell
// params[2]		curl_httppost*last	cell
// ...
// ret				CURLFORMcode		cell
static cell AMX_NATIVE_CALL amx_curl_formadd(AMX* amx, cell* params)
{
    curl_httppost** first = reinterpret_cast<curl_httppost**>(MF_GetAmxAddr(amx, params[1]));
    curl_httppost** last = reinterpret_cast<curl_httppost**>(MF_GetAmxAddr(amx, params[2]));

    int i = 3, pairs = 0;
    while (static_cast<CURLformoption>(*MF_GetAmxAddr(amx, params[i])) != CURLFORM_END) { i += 2; pairs++; }

    if (pairs == 0 || pairs > 14)
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Invalid params count, get %d, must be in range 0 - 14", pairs);

        return -1;
    }

    char strings[14][16384];

    curl_forms* forms = new curl_forms[pairs + 1];
    for (i = 0; i < pairs; i++)
    {
        strcpy(strings[i], MF_GetAmxString(amx, params[i * 2 + 4], 0, &g_len));

        forms[i].option = static_cast<CURLformoption>(*MF_GetAmxAddr(amx, params[i * 2 + 3]));
        forms[i].value = strings[i];
    }
    forms[pairs].option = CURLFORM_END;

    CURLFORMcode code = curl_formadd(first, last, CURLFORM_ARRAY, forms, CURLFORM_END);

    delete[] forms; // pairs + 1

    return code;
}

// params[1]		curl_httppost*first	cell
static cell AMX_NATIVE_CALL amx_curl_formfree(AMX* amx, cell* params)
{
    curl_formfree(reinterpret_cast<curl_httppost*>(*MF_GetAmxAddr(amx, params[1])));

    return 0;
}

// params[1]		curl_slist* list	cell
// params[2]		char* string		arr
// ret				curl_slist* list	cell
static cell AMX_NATIVE_CALL amx_curl_slist_append(AMX* amx, cell* params)
{
    return reinterpret_cast<cell>(curl_slist_append(reinterpret_cast<curl_slist*>(params[1]), MF_GetAmxString(amx, params[2], 0, &g_len)));
}

// params[1]		curl_slist* list	cell
static cell AMX_NATIVE_CALL amx_curl_slist_free_all(AMX* amx, cell* params)
{
    curl_slist_free_all(reinterpret_cast<curl_slist*>(params[1]));

    return 0;
}

// params[1]		char* buf			str
// params[2]		int len				cell
// ret
static cell AMX_NATIVE_CALL amx_curl_version(AMX* amx, cell* params)
{
    MF_SetAmxString(amx, params[1], curl_version(), params[2]);

    return 0;
}

AMX_NATIVE_INFO g_amx_curl_natives[] =
{
    { "curl_easy_escape",		amx_curl_easy_escape },
    { "curl_easy_unescape",		amx_curl_easy_unescape },
    { "curl_easy_init",			amx_curl_easy_init },
    { "curl_easy_perform",		amx_curl_easy_perform },
    { "curl_easy_setopt",		amx_curl_easy_setopt },
    { "curl_easy_cleanup",      amx_curl_easy_cleanup },
    { "curl_easy_reset",        amx_curl_easy_reset },
    { "curl_easy_getinfo",      amx_curl_easy_getinfo },
    { "curl_easy_strerror",		amx_curl_easy_strerror },
    { "curl_formadd",			amx_curl_formadd },
    { "curl_formfree",			amx_curl_formfree },
    { "curl_slist_append",		amx_curl_slist_append },
    { "curl_slist_free_all",	amx_curl_slist_free_all },
    { "curl_version",			amx_curl_version },
    { NULL,						NULL },
};
