#ifndef _ASIO_POLLER_CLASS_H_
#define _ASIO_POLLER_CLASS_H_

#include <asio.hpp>
#include <memory>

class AsioPoller
{
public:
    AsioPoller();
    ~AsioPoller();

    void Poll();

    asio::io_context& get_io_context() { return io_context_; }
    asio::steady_timer& get_timer() { return timer_; }

private:
    asio::io_context io_context_;
    asio::steady_timer timer_;
};

#endif // _ASIO_POLLER_CLASS_H_
