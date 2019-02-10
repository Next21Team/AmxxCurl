#include "asio_poller.h"

using namespace std;

AsioPoller::AsioPoller() :
    timer_(io_context_)
{
}

AsioPoller::~AsioPoller()
{
    int a = 1;
}

void AsioPoller::Poll()
{
    io_context_.poll();

    if (io_context_.stopped())
        io_context_.restart();
}
