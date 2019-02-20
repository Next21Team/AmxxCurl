#include "asio_poller.h"

using namespace std;
using namespace asio::ip;

AsioPoller::AsioPoller() :
    timer_(io_context_)
{
}

AsioPoller::~AsioPoller()
{
}

tcp::socket AsioPoller::CreateTcpSocket()
{
    return tcp::socket(io_context_);
}

tcp::socket AsioPoller::WrapTcpSocket(const asio::detail::socket_type& native_socket, const tcp::socket::protocol_type protocol)
{
    return tcp::socket(io_context_, protocol, native_socket);
}

void AsioPoller::Poll()
{
    io_context_.poll();

    if (io_context_.stopped())
        io_context_.restart();
}
