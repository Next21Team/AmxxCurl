#ifndef  _SIMPLE_MULTIPLATFORM_EXECUTION_QUEUE_CLASS_H_
#define  _SIMPLE_MULTIPLATFORM_EXECUTION_QUEUE_CLASS_H_

#include <mutex>
#include <queue>

#include "execution_queue_interface.h"

// Стандартная мультиплатформенная реализация (и скорее всего медленная :p) очереди потоков
class SimpleMultiplatformExecutionQueue : public ExecutionQueueInterface
{
    struct ThreadData
    {
        ThreadData(std::condition_variable& condition_variable, bool& notify)
            : condition_variable(condition_variable),
              notify(notify)
        { }

        std::condition_variable& condition_variable;
        bool& notify;
    };

public:
    SimpleMultiplatformExecutionQueue() :
        executed_(false)
    { }

    void WaitSignal(std::condition_variable& condition_variable, bool& notify) override
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        threads_queue_.emplace(ThreadData(condition_variable, notify));
    }

    void Executed() override
    {
        executed_ = true;
        cv_executed_.notify_one();
    }


    void ExecuteAll() override
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        while (!threads_queue_.empty())
        {
            std::unique_lock<std::mutex> lock_exec(mutex_executed_);

            ThreadData data = threads_queue_.front();
            data.notify = true;
            data.condition_variable.notify_one();

            threads_queue_.pop();

            // ожидание вызова Executed() из потока
            while (!executed_)
                cv_executed_.wait(lock_exec);
            executed_ = false;
        }
    }

private:
    std::queue<ThreadData> threads_queue_;

    bool executed_;
    std::mutex mutex_executed_;
    std::condition_variable cv_executed_;

    std::mutex queue_mutex_;
};

#endif // _SIMPLE_MULTIPLATFORM_EXECUTION_QUEUE_CLASS_H_