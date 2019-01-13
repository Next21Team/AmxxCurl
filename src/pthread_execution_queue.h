#if AMXXCURL_USE_PTHREADS_EXPLICITLY

#ifndef _PTHREAD_EXECUTION_QUEUE_H_
#define _PTHREAD_EXECUTION_QUEUE_H_

#include <pthread.h>
#include <queue>
#include "execution_queue_interface.h"

class PthreadExecutionQueue : public ExecutionQueueInterface
{
    struct ThreadData
    {
        ThreadData(pthread_cond_t& condition_variable, bool& notify)
            : condition_variable(condition_variable),
            notify(notify)
        { }

        pthread_cond_t& condition_variable;
        bool& notify;
    };

public:
    PthreadExecutionQueue() :
        executed_(false)
    {
        pthread_mutex_init(&mutex_thread_signal_, NULL);
        pthread_mutex_init(&mutex_thread_executed_, NULL);
        pthread_mutex_init(&queue_mutex_, NULL);
        pthread_cond_init(&cv_thread_executed_, NULL);
    }

    void WaitSignal() override
    {
        pthread_cond_t cv;
        pthread_cond_init(&cv, NULL);

        bool notify = false;

        pthread_mutex_lock(&queue_mutex_);
        threads_queue_.emplace(cv, notify);
        pthread_mutex_unlock(&queue_mutex_);

        pthread_mutex_lock(&mutex_thread_signal_);
        while (!notify)
            pthread_cond_wait(&cv, &mutex_thread_signal_);
        pthread_mutex_unlock(&mutex_thread_signal_);
    }

    void Executed() override
    {
        pthread_mutex_lock(&mutex_thread_executed_);
        executed_ = true;
        pthread_cond_signal(&cv_thread_executed_);
        pthread_mutex_unlock(&mutex_thread_executed_);
    }


    void ExecuteAll() override
    {
        pthread_mutex_lock(&queue_mutex_);

        while (!threads_queue_.empty())
        {
            pthread_mutex_lock(&mutex_thread_signal_);

            ThreadData& thread_data = threads_queue_.front();
            thread_data.notify = true;
            pthread_cond_signal(&thread_data.condition_variable);

            threads_queue_.pop();
            pthread_mutex_unlock(&mutex_thread_signal_);

            pthread_mutex_lock(&mutex_thread_executed_);
            while (!executed_)
                pthread_cond_wait(&cv_thread_executed_, &mutex_thread_executed_);
            executed_ = false;
            pthread_mutex_unlock(&mutex_thread_executed_);
        }

        pthread_mutex_unlock(&queue_mutex_);
    }

private:
    std::queue<ThreadData> threads_queue_;

    bool executed_;
    pthread_mutex_t mutex_thread_signal_;
    pthread_mutex_t mutex_thread_executed_;
    pthread_cond_t cv_thread_executed_;

    pthread_mutex_t queue_mutex_;
};

#endif // _PTHREAD_EXECUTION_QUEUE_H_

#endif
