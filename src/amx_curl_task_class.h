#ifndef _AMX_CURL_TASK_CLASS_H_
#define _AMX_CURL_TASK_CLASS_H_

#include "curl_class.h"
#include "amx_curl_callback_class.h"
#include "simple_multiplatform_execution_queue_class.h"

#if AMXXCURL_USE_PTHREADS_EXPLICITLY
#include <pthread.h>
#endif

class AmxCurlTask
{
    using AmxCallback = int;

public:
    AmxCurlTask(AMX* amx, ExecutionQueueInterface& execution_queue) :
        amx_(amx),
        execution_queue_(execution_queue),
        curl_callback_(std::make_shared<CurlCallbackAmx>(amx, execution_queue)),
        curl_(curl_callback_),
        thread_active_(false)
    { }

    void PerformTask(const char* complete_callback, int handle, cell* data, int data_len)
    {
        if (!thread_active_)
        {
            thread_active_ = true;

            if(data != nullptr)
                amx_callback_ = MF_RegisterSPForwardByName(amx_, complete_callback, FP_CELL /* handle */, FP_CELL /* CURLcode */, FP_ARRAY /* data */, FP_DONE);
            else
                amx_callback_ = MF_RegisterSPForwardByName(amx_, complete_callback, FP_CELL /* handle */, FP_CELL /* CURLcode */, FP_DONE);
            
            curl_manager_handle_ = handle;
            amx_callback_data_ = data;
            amx_callback_data_len_ = data_len;

#if AMXXCURL_USE_PTHREADS_EXPLICITLY
            pthread_create(&pthread_, nullptr, ThreadFunctionStatic, this);
            pthread_detach(pthread_);
#else
            auto thread = std::thread(ThreadFunctionStatic, this);
            thread.detach();
#endif
        }
        else
            throw std::runtime_error("Curl thread already started");
    }

    Curl& get_curl() { return curl_; }

    CurlCallbackAmx& get_curl_callback_amx() const { return *curl_callback_; }

    bool is_thread_active() const { return thread_active_; }

private:
    void ThreadFunction()
    {
        CURLcode result = curl_.Perform();

        SignalExecutor executor(execution_queue_);

        thread_active_ = false;

        if (amx_callback_data_ != nullptr)
        {
            MF_ExecuteForward(amx_callback_, curl_manager_handle_, result, MF_PrepareCellArray(amx_callback_data_, amx_callback_data_len_));
            delete[] amx_callback_data_;
        }
        else
            MF_ExecuteForward(amx_callback_, curl_manager_handle_, result);

        MF_UnregisterSPForward(amx_callback_);
    }
    
#if AMXXCURL_USE_PTHREADS_EXPLICITLY
    static void* ThreadFunctionStatic(void* threadData)
    {
        reinterpret_cast<AmxCurlTask*>(threadData)->ThreadFunction();
        pthread_exit(0);
    }
#else
    static void ThreadFunctionStatic(void* threadData)
    {
        reinterpret_cast<AmxCurlTask*>(threadData)->ThreadFunction();
    }
#endif

private:
    AMX* amx_;
    ExecutionQueueInterface& execution_queue_;
    std::shared_ptr<CurlCallbackAmx> curl_callback_;
    Curl curl_;
    bool thread_active_;

    AmxCallback amx_callback_;
    int curl_manager_handle_;
    cell* amx_callback_data_;
    int amx_callback_data_len_;


#if AMXXCURL_USE_PTHREADS_EXPLICITLY
    pthread_t pthread_;
#endif
};

#endif // _AMX_CURL_TASK_CLASS_H_
