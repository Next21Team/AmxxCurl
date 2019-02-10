#include "amx_curl_callback_class.h"
#include "curl_utils_class.h"

CurlCallbackAmx::CurlCallbackAmx(AMX* amx) :
    amx_(amx),
    interrupt_(false)
{ }

CurlCallbackAmx::~CurlCallbackAmx()
{
    ResetAmxCallbacks();
}

void CurlCallbackAmx::SetData(CURLoption data_option, void* data)
{
    data_[data_option] = data;
}

void CurlCallbackAmx::TryInterrupt()
{
    interrupt_ = true;
}

void CurlCallbackAmx::ResetAmxCallbacks()
{
    std::for_each(registered_callbacks_.begin(), registered_callbacks_.end(), [](std::pair<const CURLoption, AmxForward>& pair) { MF_UnregisterSPForward(pair.second); });
}

void CurlCallbackAmx::SetupAmxCallback(CURLoption callback_option, const char* amx_function_name)
{
    if (registered_callbacks_.count(callback_option) > 0)
        MF_UnregisterSPForward(registered_callbacks_[callback_option]);

    switch (callback_option)
    {
    case CURLOPT_WRITEFUNCTION:
        // char *ptr, size_t size, size_t nmemb, void *userdata
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_ARRAY, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
        break;

    case CURLOPT_READFUNCTION:
        // char *buffer, size_t size, size_t nitems, void *instream
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_ARRAY, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
        break;

    case CURLOPT_IOCTLFUNCTION:
        // CURL *handle, int cmd, void *clientp
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
        break;

    case CURLOPT_SEEKFUNCTION:
        // void *userp, curl_off_t offset, int origin
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_CELL /* high offset*/, FP_CELL /* low offset */, FP_CELL, FP_DONE);
        break;

    case CURLOPT_SOCKOPTFUNCTION:
        // void *clientp, curl_socket_t curlfd, curlsocktype purpose
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
        break;

    case CURLOPT_PROGRESSFUNCTION:
        // void *clientp, double dltotal, double dlnow, double ultotal, double ulnow
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_FLOAT, FP_FLOAT, FP_FLOAT, FP_FLOAT, FP_DONE);
        break;

    case CURLOPT_HEADERFUNCTION:
        // char *buffer, size_t size, size_t nitems, void *userdata
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_ARRAY, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
        break;

    case CURLOPT_DEBUGFUNCTION:
        // CURL *handle, curl_infotype type, char *data, size_t size, void *userptr
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_CELL, FP_ARRAY, FP_CELL, FP_CELL, FP_DONE);
        break;

    case CURLOPT_SSL_CTX_FUNCTION:
        // CURL *curl, void *ssl_ctx, void *userptr
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
        break;

    case CURLOPT_INTERLEAVEFUNCTION:
        // void *ptr, size_t size, size_t nmemb, void *userdata
        registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_ARRAY, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
        break;

    default:
        throw std::runtime_error("Unsupported option");
    }
}


// protected
size_t CurlCallbackAmx::WriteCallback(char* ptr, size_t size, size_t nmemb)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_WRITEFUNCTION);
    }

    // char *ptr, size_t size, size_t nmemb, void *userdata
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_WRITEFUNCTION], MF_PrepareCharArray(ptr, size * nmemb), size, nmemb, data_[CURLOPT_WRITEDATA]);
}

size_t CurlCallbackAmx::ReadCallback(char* buffer, size_t size, size_t nitems)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_READFUNCTION);
    }

    // CURL *handle, int cmd, void *clientp
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_READFUNCTION], MF_PrepareCharArrayA(buffer, size * nitems, true), size, nitems, data_[CURLOPT_READDATA]);
}

curlioerr CurlCallbackAmx::IoctlCallback(CURL* handle, int cmd)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return static_cast<curlioerr>(CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_IOCTLFUNCTION));
    }

    // CURL *handle, int cmd, void *clientp
    return static_cast<curlioerr>(MF_ExecuteForward(registered_callbacks_[CURLOPT_IOCTLFUNCTION], handle, cmd, data_[CURLOPT_IOCTLDATA]));
}

int CurlCallbackAmx::SeekCallback(curl_off_t offset, int origin)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_SEEKFUNCTION);
    }

    // TODO если curl_off_t не 8 байт?
    // void *userp, curl_off_t offset, int origin
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_SEEKFUNCTION], data_[CURLOPT_SEEKDATA], offset >> 32, static_cast<int>(offset), origin);
}

int CurlCallbackAmx::SockoptCallback(curl_socket_t curlfd, curlsocktype purpose)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_SOCKOPTFUNCTION);
    }

    // void *clientp, curl_socket_t curlfd, curlsocktype purpose
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_SOCKOPTFUNCTION], data_[CURLOPT_SOCKOPTDATA], curlfd, purpose);
}

int CurlCallbackAmx::ProgressCallback(double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_PROGRESSFUNCTION);
    }

    // void *clientp, double dltotal, double dlnow, double ultotal, double ulnow
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_PROGRESSFUNCTION], data_[CURLOPT_PROGRESSDATA], dltotal, dlnow, ultotal, ulnow);
}

size_t CurlCallbackAmx::HeaderCallback(char* buffer, size_t size, size_t nitems)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_HEADERFUNCTION);
    }

    // char *buffer, size_t size, size_t nitems, void *userdata
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_HEADERFUNCTION], MF_PrepareCharArray(buffer, size * nitems), size, nitems, data_[CURLOPT_HEADERDATA]);
}

int CurlCallbackAmx::DebugCallback(CURL* handle, curl_infotype type, char* data, size_t size)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_DEBUGFUNCTION);
    }

    // CURL *handle, curl_infotype type, char *data, size_t size, void *userptr
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_DEBUGFUNCTION], handle, type, MF_PrepareCharArray(data, size), size, data_[CURLOPT_DEBUGDATA]);
}

CURLcode CurlCallbackAmx::SslCtxCallback(CURL* curl, void* ssl_ctx)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return static_cast<CURLcode>(CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_SSL_CTX_FUNCTION));
    }

    // CURL *curl, void *ssl_ctx, void *userptr
    return static_cast<CURLcode>(MF_ExecuteForward(registered_callbacks_[CURLOPT_SSL_CTX_FUNCTION], curl, ssl_ctx, data_[CURLOPT_SSL_CTX_DATA]));
}

size_t CurlCallbackAmx::InterleaveCallback(void* ptr, size_t size, size_t nmemb)
{
    if (interrupt_)
    {
        interrupt_ = false;
        return static_cast<size_t>(CurlUtils::GetInterruptCodeForCurlCallback(CURLOPT_INTERLEAVEFUNCTION));
    }

    // void *ptr, size_t size, size_t nmemb, void *userdata
    return MF_ExecuteForward(registered_callbacks_[CURLOPT_INTERLEAVEFUNCTION], MF_PrepareCharArray(static_cast<char*>(ptr), size * nmemb), size, nmemb, data_[CURLOPT_INTERLEAVEDATA]);
}
