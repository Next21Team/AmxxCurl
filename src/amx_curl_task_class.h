#ifndef _AMX_CURL_TASK_CLASS_H_
#define _AMX_CURL_TASK_CLASS_H_

#include "curl_class.h"
#include "amx_curl_callback_class.h"
#include "simple_multiplatform_execution_queue_class.h"

// Класс агрегирует Curl и CurlCallbackAmxProxy, позволяет запустить передачу курл в отдельном потоке
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

    // начинает передачу curl в новом потоке, по окончанию исполнения будет вызван amx колбэк complete_callback
    // аргумент handle - ссылка на curl в менеджере, будет передана в колбэк
    void PerformTask(const char* complete_callback, int handle, cell* data, int data_len)
    {
        if (!thread_active_)
        {
            thread_active_ = true;

            AmxCallback callback;
            if(data != nullptr)
                callback = MF_RegisterSPForwardByName(amx_, complete_callback, FP_CELL /* handle */, FP_CELL /* CURLcode */, FP_ARRAY /* data */, FP_DONE);
            else
                callback = MF_RegisterSPForwardByName(amx_, complete_callback, FP_CELL /* handle */, FP_CELL /* CURLcode */, FP_DONE);


            std::thread thread = std::thread([this, callback, handle, data, data_len]()
            {
                CURLcode result = curl_.Perform();

                // синхронизация с главным потоком
                SignalExecutor executor(execution_queue_);
                thread_active_ = false; // далее объект AmxCurlTask может быть уничтожен из amx колбэка

                if (data != nullptr)
                {
                    MF_ExecuteForward(callback, handle, result, MF_PrepareCellArray(data, data_len));
                    delete[] data;
                }
                else
                    MF_ExecuteForward(callback, handle, result);

                MF_UnregisterSPForward(callback);
            });
            thread.detach();
        }
        else
            throw std::runtime_error("Curl thread already started");
    }

    Curl& get_curl() { return curl_; }

    CurlCallbackAmx& get_curl_callback_amx() const { return *curl_callback_; }

    bool is_thread_active() const { return thread_active_; }

private:
    AMX* amx_;
    ExecutionQueueInterface& execution_queue_;
    std::shared_ptr<CurlCallbackAmx> curl_callback_;
    Curl curl_;

    bool thread_active_;
};

#endif // _AMX_CURL_TASK_CLASS_H_
