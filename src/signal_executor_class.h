#ifndef _SIGNAL_EXECUTOR_H_
#define _SIGNAL_EXECUTOR_H_

#include <condition_variable>

#include "execution_queue_interface.h"

// RAII хелпер для очереди потоков. 
// При создании ставит текущий поток в ожидание, при разрушении оповещает очередь о завершении выполнения критического участка.
class SignalExecutor
{
public:
    SignalExecutor(ExecutionQueueInterface& execution_queue) :
        execution_queue_(execution_queue),
        notified_(false)
    {
        execution_queue_.WaitSignal(condition_variable_, notified_);

        std::unique_lock<std::mutex> lock(mutex_);
        while (!notified_)
            condition_variable_.wait(lock);
    }

    ~SignalExecutor()
    {
        execution_queue_.Executed();
    }

private:
    ExecutionQueueInterface& execution_queue_;

    bool notified_;
    std::mutex mutex_;
    std::condition_variable condition_variable_;
};

#endif // !_SIGNAL_EXECUTOR_H_