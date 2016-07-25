#ifndef _CURLTASKSMANAGERH_
#define _CURLTASKSMANAGERH_

#include <unordered_map>
#include <queue>
#include <chrono>
#include <thread>
#include <algorithm>

#include "curl_class.h"
#include "amx_curl_callback_class.h"
#include "amx_curl_task_class.h"

class CurlAmxManagerInvalidHandleException : std::exception
{ };

// Менеджер amx curl задач, позволяет выполнять над ними действия через дескриптор
class AmxCurlTaskManager
{
public:
    using CurlTaskHandle = int;

private:
    using CurlMapPair = std::pair<const CurlTaskHandle, std::unique_ptr<AmxCurlTask>>;

    struct CurlTaskHandleHash
    {
        size_t operator()(const CurlTaskHandle& key) const
        {
            return static_cast<size_t>(key);
        }
    };

public:
    // execution_queue очередь по которой будет проходить синхронизация всех колбэков
    AmxCurlTaskManager(ExecutionQueueInterface& execution_queue) :
        execution_queue_(execution_queue)
    { }

    // -----------------------

    CurlTaskHandle CreateTaskForAmx(AMX* amx)
    {
        CurlTaskHandle handle = curl_tasks_.size() + 1;
        if (!free_handles_.empty())
        {
            handle = free_handles_.front();
            free_handles_.pop();
        }
        else
        {
            while (curl_tasks_.count(handle) > 1)
                handle++;
        }

        curl_tasks_.emplace(handle, std::make_unique<AmxCurlTask>(amx, execution_queue_));

        return handle;
    }

    // Удаляеет задачу из менеджера (удаляются все amx колбэки и вызывается curl_easy_cleanup())
    void RemoveTask(CurlTaskHandle handle)
    {
        CheckHandle(handle);
        CheckTaskStatus(handle);

        curl_tasks_.erase(handle);
        free_handles_.push(handle);
    }

    void RemoveAllTasks()
    {
        curl_tasks_.clear();
        
        std::queue<CurlTaskHandle> empty;
        std::swap(free_handles_, empty);
    }

    // --- Управление curl

    void CurlSetupAmxCallback(CurlTaskHandle handle, CURLoption callback_option, const char* function_name)
    {
        CheckHandle(handle);
        CheckTaskStatus(handle);

        curl_tasks_[handle]->get_curl_callback_amx().SetupAmxCallback(callback_option, function_name);
        curl_tasks_[handle]->get_curl().BindCallback(callback_option);
    }

    template<class T>
    CURLcode CurlSetOption(CurlTaskHandle handle, CURLoption option, T data)
    {
        CheckHandle(handle);
        CheckTaskStatus(handle);

        if(CurlCallback::IsDataOption(option))
        {
            curl_tasks_[handle]->get_curl_callback_amx().SetData(option, reinterpret_cast<void*>(data));
            return CURLE_OK;
        }

        return curl_tasks_[handle]->get_curl().SetOption(option, data);
    }

    // запускает на исполнение курл в новом потоке
    void CurlPerformTask(CurlTaskHandle handle, const char* complete_callback, cell* data, int data_len)
    {
        CheckHandle(handle);
        CheckTaskStatus(handle);

        curl_tasks_[handle]->PerformTask(complete_callback, handle, data, data_len);
    }

    template<class T>
    CURLcode CurlGetInfo(CurlTaskHandle handle, CURLINFO curl_info, T& out_data)
    {
        CheckHandle(handle);

        return curl_tasks_[handle]->get_curl().GetInfo(curl_info, out_data);
    }

    void CurlReset(CurlTaskHandle handle)
    {
        CheckHandle(handle);
        CheckTaskStatus(handle);

        curl_tasks_[handle]->get_curl_callback_amx().ResetAmxCallbacks();
        curl_tasks_[handle]->get_curl().Reset();
    }

    void CurlEscapeUrl(CurlTaskHandle handle, const char* url, std::string& out_escaped_url)
    {
        CheckHandle(handle);

        curl_tasks_[handle]->get_curl().EscapeUrl(url, out_escaped_url);
    }

    void CurlUnescapeUrl(CurlTaskHandle handle, const char* url, std::string& out_unescaped_url)
    {
        CheckHandle(handle);

        curl_tasks_[handle]->get_curl().UnescapeUrl(url, out_unescaped_url);
    }

    //---------------------------

    // вызывает TryInterrupt для всех AmxCurlTask
    void TryInterruptAllTransfers()
    {
        std::for_each(curl_tasks_.begin(), curl_tasks_.end(), [](CurlMapPair& pair)
        {
            pair.second->get_curl_callback_amx().TryInterrupt();
        });
    }

    // ставит поток в ожидаение, до тех пор пока все передачи не завершатся
    void WaitAllTransfers()
    {
        while (true)
        {
            execution_queue_.ExecuteAll();

            if (std::find_if(curl_tasks_.begin(), curl_tasks_.end(), [](CurlMapPair& pair) { return pair.second->is_thread_active(); }) == curl_tasks_.end())
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

private:
    void CheckHandle(CurlTaskHandle handle) const
    {
        if (curl_tasks_.count(handle) == 0)
            throw CurlAmxManagerInvalidHandleException();
    }

    void CheckTaskStatus(CurlTaskHandle handle)
    {
        if (curl_tasks_[handle]->is_thread_active())
            throw std::runtime_error("invalid operation while curl task is active");
    }

    std::unordered_map<CurlTaskHandle, std::unique_ptr<AmxCurlTask>, CurlTaskHandleHash> curl_tasks_;
    std::queue<CurlTaskHandle> free_handles_;
    ExecutionQueueInterface& execution_queue_;
};

#endif // _CURLTASKSMANAGERH_