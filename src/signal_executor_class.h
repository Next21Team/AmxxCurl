#ifndef _SIGNAL_EXECUTOR_H_
#define _SIGNAL_EXECUTOR_H_

#include <condition_variable>

#include "execution_queue_interface.h"

// RAII helper for ExecutionQueueInterface.
class SignalExecutor
{
public:
    SignalExecutor(ExecutionQueueInterface& execution_queue) :
        execution_queue_(execution_queue)
    {
        execution_queue_.WaitSignal();
    }

    ~SignalExecutor()
    {
        execution_queue_.Executed();
    }

private:
    ExecutionQueueInterface& execution_queue_;
};

#endif // !_SIGNAL_EXECUTOR_H_
