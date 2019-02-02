#ifndef _AMX_CURL_TASK_CLASS_H_
#define _AMX_CURL_TASK_CLASS_H_

#include "curl_class.h"
#include "amx_curl_callback_class.h"
#include "simple_multiplatform_execution_queue_class.h"

#if AMXXCURL_USE_PTHREADS_EXPLICITLY
#include <pthread.h>
#endif

class CurlTaskCallbackNotFoundException : std::exception
{ };

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

    void PerformTask(const char* complete_callback, int task_handle, cell* data, int data_len)
    {
        if (!thread_active_)
        {
            if (MF_AmxFindPublic(amx_, complete_callback, &amx_callback_fun_) != AMX_ERR_NONE)
            {
                throw CurlTaskCallbackNotFoundException();
            }

            thread_active_ = true;
            amx_callback_data_ = data;
            amx_callback_data_len_ = data_len;
            task_handle_ = task_handle;

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

        // we need to store data here, because execution of amx callback can override object properties 
        cell* cb_data = amx_callback_data_;
        int cb_data_len = amx_callback_data_len_;
        int cb_id = amx_callback_fun_;
        int task_handle = task_handle_;

        int forward_id;
        if (cb_data != nullptr)
        {
            forward_id = MF_RegisterSPForward(amx_, cb_id, FP_CELL /* handle */, FP_CELL /* CURLcode */, FP_ARRAY /* data */, FP_DONE);
            MF_ExecuteForward(forward_id, task_handle, result, MF_PrepareCellArray(cb_data, cb_data_len));
            delete[] cb_data;
        }
        else
        {
            forward_id = MF_RegisterSPForward(amx_, cb_id, FP_CELL /* handle */, FP_CELL /* CURLcode */, FP_DONE);
            MF_ExecuteForward(forward_id, task_handle, result);
        }
		
        MF_UnregisterSPForward(forward_id);
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

    AmxCallback amx_callback_fun_;
    cell* amx_callback_data_;
    int task_handle_;
    int amx_callback_data_len_;


#if AMXXCURL_USE_PTHREADS_EXPLICITLY
    pthread_t pthread_;
#endif
};

#endif // _AMX_CURL_TASK_CLASS_H_
