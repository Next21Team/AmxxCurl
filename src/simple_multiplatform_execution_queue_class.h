#ifndef  _SIMPLE_MULTIPLATFORM_EXECUTION_QUEUE_CLASS_H_
#define  _SIMPLE_MULTIPLATFORM_EXECUTION_QUEUE_CLASS_H_

#include <mutex>
#include <queue>

#include "execution_queue_interface.h"

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

    void WaitSignal() override
    {
        std::condition_variable cv;
        bool notify = false;

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            threads_queue_.emplace(ThreadData(cv, notify));
        }

        std::unique_lock<std::mutex> lock(mutex_thread_signal_);
        while (!notify)
            cv.wait(lock);
    }

    void Executed() override
    {
        std::unique_lock<std::mutex> lock(mutex_thread_executed_);
        executed_ = true;
        cv_thread_executed_.notify_one();
    }


    void ExecuteAll() override
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        while (!threads_queue_.empty())
        {
            {
                std::unique_lock<std::mutex> lock(mutex_thread_signal_);

                ThreadData& thread_data = threads_queue_.front();
                thread_data.notify = true;
                thread_data.condition_variable.notify_one();

                threads_queue_.pop();
            }

            std::unique_lock<std::mutex> lock(mutex_thread_executed_);
            while (!executed_)
                cv_thread_executed_.wait(lock);
            executed_ = false;
        }
    }

private:
    std::queue<ThreadData> threads_queue_;

    bool executed_;
    std::mutex mutex_thread_signal_;
    std::mutex mutex_thread_executed_;
    std::condition_variable cv_thread_executed_;

    std::mutex queue_mutex_;
};

#endif // _SIMPLE_MULTIPLATFORM_EXECUTION_QUEUE_CLASS_H_