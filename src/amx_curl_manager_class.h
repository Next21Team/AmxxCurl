#ifndef _AMX_CURL_MANAGER_CLASS_H_
#define _AMX_CURL_MANAGER_CLASS_H_

#include <unordered_map>
#include <map>
#include <queue>
#include <chrono>
#include <algorithm>
#include <utility>

#include "curl_class.h"
#include "curl_multi_class.h"
#include "amx_curl_class.h"
#include "amx_curl_callback_class.h"
#include "curl_utils_class.h"

class CurlAmxManagerInvalidHandleException : std::exception
{ };

class AmxCurlManager
{
public:
    using AmxCurlHandle = int;

private:
    using CurlMapPair = std::pair<const AmxCurlHandle, std::unique_ptr<AmxCurl>>;

    struct CurlHandleHash
    {
        size_t operator()(const AmxCurlHandle& key) const
        {
            return static_cast<size_t>(key);
        }
    };

public:
    AmxCurlManager(AsioPoller& asio_poller) :
        curl_multi_(asio_poller)
    { }

    AmxCurlHandle CreateCurl(AMX* amx)
    {
        AmxCurlHandle handle = GetCurlHandle();
        amx_curl_.emplace(handle, AmxCurl(amx, curl_multi_));
        
        return handle;
    }

    void RemoveTask(AmxCurlHandle handle)
    {
        CheckHandle(handle);

        amx_curl_.erase(handle);
        FreeCurlHandle(handle);
    }

    void RemoveAllTasks()
    {
        amx_curl_.clear();

        std::queue<AmxCurlHandle> empty;
        std::swap(free_handles_, empty);
    }

    void CurlSetupAmxCallback(AmxCurlHandle handle, CURLoption callback_option, const char* function_name)
    {
        CheckHandle(handle);

        amx_curl_.at(handle).get_curl_callback_amx().SetupAmxCallback(callback_option, function_name);
        amx_curl_.at(handle).get_curl().BindCallback(callback_option);
    }

    template<class T>
    CURLcode CurlSetOption(AmxCurlHandle handle, CURLoption option, T data)
    {
        CheckHandle(handle);

        if (CurlUtils::IsDataOption(option))
        {
            amx_curl_.at(handle).get_curl_callback_amx().SetData(option, reinterpret_cast<void*>(data));
            return CURLE_OK;
        }

        return amx_curl_.at(handle).get_curl().SetOption(option, data);
    }

    void CurlPerformTask(AmxCurlHandle handle, const char* complete_callback, cell* data, int data_len)
    {
        CheckHandle(handle);

        amx_curl_.at(handle).Perform(complete_callback, handle, data, data_len);
    }

    template<class T>
    CURLcode CurlGetInfo(AmxCurlHandle handle, CURLINFO curl_info, T& out_data)
    {
        CheckHandle(handle);

        return amx_curl_.at(handle).get_curl().GetInfo(curl_info, out_data);
    }

    void CurlReset(AmxCurlHandle handle)
    {
        CheckHandle(handle);

        amx_curl_.at(handle).get_curl_callback_amx().ResetAmxCallbacks();
        amx_curl_.at(handle).get_curl().Reset();
    }

    void CurlEscapeUrl(AmxCurlHandle handle, const char* url, std::string& out_escaped_url)
    {
        CheckHandle(handle);

        amx_curl_.at(handle).get_curl().EscapeUrl(url, out_escaped_url);
    }

    void CurlUnescapeUrl(AmxCurlHandle handle, const char* url, std::string& out_unescaped_url)
    {
        CheckHandle(handle);

        amx_curl_.at(handle).get_curl().UnescapeUrl(url, out_unescaped_url);
    }

    //---------------------------

    void TryInterruptAllTransfers()
    {
        std::for_each(amx_curl_.begin(), amx_curl_.end(), [](auto& pair)
        {
            pair.second.get_curl_callback_amx().TryInterrupt();
        });
    }

    bool IsAllTransfersCompleted()
    {
        return std::all_of(amx_curl_.begin(), amx_curl_.end(), [](auto& pair)
        {
            return !pair.second.get_is_transfer_in_progress();
        });
    }

private:
    AmxCurlHandle GetCurlHandle()
    {
        AmxCurlHandle handle = amx_curl_.size() + 1;
        if (!free_handles_.empty())
        {
            handle = free_handles_.front();
            free_handles_.pop();
        }
        else
        {
            while (amx_curl_.count(handle) > 1)
                handle++;
        }

        return handle;
    }
    
    void FreeCurlHandle(AmxCurlHandle handle)
    {
        free_handles_.push(handle);
    }

    void CheckHandle(AmxCurlHandle handle) const
    {
        if (amx_curl_.count(handle) == 0)
            throw CurlAmxManagerInvalidHandleException();

        /*if (curl_tasks_[handle]->is_thread_active())
            throw std::runtime_error("invalid operation while curl task is active");*/
    }

    CurlMulti curl_multi_;
    std::unordered_map<AmxCurlHandle, AmxCurl, CurlHandleHash> amx_curl_;
    std::queue<AmxCurlHandle> free_handles_;
};

#endif // _AMX_CURL_MANAGER_CLASS_H_
