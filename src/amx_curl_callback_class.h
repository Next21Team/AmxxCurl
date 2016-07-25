#ifndef _CURLCALLBACKAMXPROXYH_
#define _CURLCALLBACKAMXPROXYH_

#include <unordered_map>
#include <atomic>
#include <algorithm>

#include "sdk/amxxmodule.h"

#include "curl_callback_class.h"
#include "execution_queue_interface.h"
#include "signal_executor_class.h"

class CurlCallbackAmx : public CurlCallback
{
    using AmxForward = int;

    struct CurlOptionHash
    {
        size_t operator()(const CURLoption& key) const
        {
            return static_cast<size_t>(key);
        }
    };

public:
    CurlCallbackAmx(AMX* amx, ExecutionQueueInterface& execution_queue) :
        amx_(amx),
        execution_queue_(execution_queue),
        interrupt_(false)
    { }

    // Передача curl к моменту разрушения объекта обязательно должна быть завершена или прервана!
    virtual ~CurlCallbackAmx()
    {
        ResetAmxCallbacks();
    }

    void SetData(CURLoption data_option, void* data)
    {
        data_[data_option] = data;
    }

    // Прерывает выполнение curl путем возврата специального значения в колбэке (например для CURLOPT_READFUNCTION будет CURL_READFUNC_ABORT),
    // что приведет к немедленной остановке исполнения.
    // Однако, если курл находится в какой-то другой стадии исполнения, то это ни к чему не приведет.
    void TryInterrupt()
    {
        interrupt_ = true;
    }

    void ResetAmxCallbacks()
    {
        std::for_each(registered_callbacks_.begin(), registered_callbacks_.end(), [](std::pair<const CURLoption, AmxForward>& pair) { MF_UnregisterSPForward(pair.second); });
    }

    void SetupAmxCallback(CURLoption callback_option, const char* amx_function_name)
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

        case CURLOPT_OPENSOCKETFUNCTION:
            // void *clientp, curlsocktype purpose, struct curl_sockaddr *address
            registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_CELL, FP_CELL, FP_DONE);
            break;

        case CURLOPT_CLOSESOCKETFUNCTION:
            // void *clientp, curl_socket_t item
            registered_callbacks_[callback_option] = MF_RegisterSPForwardByName(amx_, amx_function_name, FP_CELL, FP_CELL, FP_DONE);
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


protected:
    size_t WriteCallback(char* ptr, size_t size, size_t nmemb) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_WRITEFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // char *ptr, size_t size, size_t nmemb, void *userdata
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_WRITEFUNCTION], MF_PrepareCharArray(ptr, size * nmemb), size, nmemb, data_[CURLOPT_WRITEDATA]);
    }

    size_t ReadCallback(char* buffer, size_t size, size_t nitems) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_READFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // CURL *handle, int cmd, void *clientp
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_READFUNCTION], MF_PrepareCharArrayA(buffer, size * nitems, true), size, nitems, data_[CURLOPT_READDATA]);
    }

    curlioerr IoctlCallback(CURL* handle, int cmd) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return static_cast<curlioerr>(GetInterruptCodeForCurlCallback(CURLOPT_IOCTLFUNCTION));
        }

        SignalExecutor executor(execution_queue_);

        // CURL *handle, int cmd, void *clientp
        return static_cast<curlioerr>(MF_ExecuteForward(registered_callbacks_[CURLOPT_IOCTLFUNCTION], handle, cmd, data_[CURLOPT_IOCTLDATA]));
    }

    int SeekCallback(curl_off_t offset, int origin) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_SEEKFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // TODO если curl_off_t не 8 байт?
        // void *userp, curl_off_t offset, int origin
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_SEEKFUNCTION], data_[CURLOPT_SEEKDATA], offset >> 32, static_cast<int>(offset), origin);
    }

    int SockoptCallback(curl_socket_t curlfd, curlsocktype purpose) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_SOCKOPTFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // void *clientp, curl_socket_t curlfd, curlsocktype purpose
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_SOCKOPTFUNCTION], data_[CURLOPT_SOCKOPTDATA], curlfd, purpose);
    }

    curl_socket_t OpensocketCallback(curlsocktype purpose, curl_sockaddr* address) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return static_cast<curl_socket_t>(GetInterruptCodeForCurlCallback(CURLOPT_OPENSOCKETFUNCTION));
        }

        SignalExecutor executor(execution_queue_);

        // void *clientp, curlsocktype purpose, struct curl_sockaddr *address
        return static_cast<curl_socket_t>(MF_ExecuteForward(registered_callbacks_[CURLOPT_OPENSOCKETFUNCTION], data_[CURLOPT_OPENSOCKETDATA], purpose, address));
    }

    int ClosesocketCallback(curl_socket_t item) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_CLOSESOCKETFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // void *clientp, curl_socket_t item
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_CLOSESOCKETFUNCTION], data_[CURLOPT_CLOSESOCKETDATA], item);
    }

    int ProgressCallback(double dltotal, double dlnow, double ultotal, double ulnow) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_PROGRESSFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // void *clientp, double dltotal, double dlnow, double ultotal, double ulnow
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_PROGRESSFUNCTION], data_[CURLOPT_PROGRESSDATA], dltotal, dlnow, ultotal, ulnow);
    }

    size_t HeaderCallback(char* buffer, size_t size, size_t nitems) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_HEADERFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // char *buffer, size_t size, size_t nitems, void *userdata
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_HEADERFUNCTION], MF_PrepareCharArray(buffer, size * nitems), size, nitems, data_[CURLOPT_HEADERDATA]);
    }

    int DebugCallback(CURL* handle, curl_infotype type, char* data, size_t size) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return GetInterruptCodeForCurlCallback(CURLOPT_DEBUGFUNCTION);
        }

        SignalExecutor executor(execution_queue_);

        // CURL *handle, curl_infotype type, char *data, size_t size, void *userptr
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_DEBUGFUNCTION], handle, type, MF_PrepareCharArray(data, size), size, data_[CURLOPT_DEBUGDATA]);
    }

    CURLcode SslCtxCallback(CURL* curl, void* ssl_ctx) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return static_cast<CURLcode>(GetInterruptCodeForCurlCallback(CURLOPT_SSL_CTX_FUNCTION));
        }

        SignalExecutor executor(execution_queue_);

        // CURL *curl, void *ssl_ctx, void *userptr
        return static_cast<CURLcode>(MF_ExecuteForward(registered_callbacks_[CURLOPT_SSL_CTX_FUNCTION], curl, ssl_ctx, data_[CURLOPT_SSL_CTX_DATA]));
    }

    size_t InterleaveCallback(void* ptr, size_t size, size_t nmemb) override
    {
        if (interrupt_)
        {
            interrupt_ = false;
            return static_cast<size_t>(GetInterruptCodeForCurlCallback(CURLOPT_INTERLEAVEFUNCTION));
        }

        SignalExecutor executor(execution_queue_);

        // void *ptr, size_t size, size_t nmemb, void *userdata
        return MF_ExecuteForward(registered_callbacks_[CURLOPT_INTERLEAVEFUNCTION], MF_PrepareCharArray(static_cast<char*>(ptr), size * nmemb), size, nmemb, data_[CURLOPT_INTERLEAVEDATA]);
    }

private:
    // функция возвращает значение, которое можно вернуть в соответствующем колбэке для вызова ошибки curl
    static int GetInterruptCodeForCurlCallback(CURLoption callback_option)
    {
        switch (callback_option) {
        case CURLOPT_WRITEFUNCTION:			return 0;
        case CURLOPT_READFUNCTION:			return CURL_READFUNC_ABORT;
        case CURLOPT_IOCTLFUNCTION:			return CURLIOE_UNKNOWNCMD;
        case CURLOPT_SEEKFUNCTION:			return 1;
        case CURLOPT_SOCKOPTFUNCTION:		return 1;
        case CURLOPT_OPENSOCKETFUNCTION:	return CURL_SOCKET_BAD;
        case CURLOPT_CLOSESOCKETFUNCTION:	return 1;
        case CURLOPT_PROGRESSFUNCTION:		return 1;
        case CURLOPT_XFERINFOFUNCTION:		return 1;
        case CURLOPT_HEADERFUNCTION:		return 0;
        case CURLOPT_DEBUGFUNCTION:			return 1; // ??
        case CURLOPT_SSL_CTX_FUNCTION:		return CURLE_GOT_NOTHING;
        case CURLOPT_INTERLEAVEFUNCTION:	return 0; // ??
        case CURLOPT_CHUNK_BGN_FUNCTION:	return CURL_CHUNK_BGN_FUNC_FAIL;
        case CURLOPT_CHUNK_END_FUNCTION:	return CURL_CHUNK_END_FUNC_FAIL;
        case CURLOPT_FNMATCH_FUNCTION:		return CURL_FNMATCHFUNC_FAIL;

        default:
            throw std::runtime_error("Invalid option");
        }
    }

    AMX* amx_;
    ExecutionQueueInterface& execution_queue_;
    std::unordered_map<CURLoption, AmxForward, CurlOptionHash> registered_callbacks_;
    std::unordered_map<CURLoption, void*, CurlOptionHash> data_;

    std::atomic<bool> interrupt_;
};

#endif // _CURLCALLBACKAMXPROXYH_