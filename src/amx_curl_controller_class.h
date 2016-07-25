#ifndef _AMX_CURL_CONTROLLER_H_
#define _AMX_CURL_CONTROLLER_H_

#include "amx_curl_task_manager_class.h"

class AmxCurlController
{
public:
    AmxCurlTaskManager& get_curl_tasks_manager()
    {
        return curl_task_manager_;
    }

    // Возвращает очередь по которой происходит синхронизация всех колбэков, которые передаются в amxmodx
    ExecutionQueueInterface& get_execution_queue()
    {
        return *execution_queue_;
    }

    static AmxCurlController& Instance()
    {
        static AmxCurlController instance;
        return instance;
    }

private:
    AmxCurlController() :
        execution_queue_(std::make_unique<SimpleMultiplatformExecutionQueue>()),
        curl_task_manager_(*execution_queue_)
    {
    }

    AmxCurlController(const AmxCurlController& root);
    AmxCurlController& operator=(const AmxCurlController&);

    std::unique_ptr<ExecutionQueueInterface> execution_queue_;
    AmxCurlTaskManager curl_task_manager_;
};

#endif // _AMX_CURL_CONTROLLER_H_