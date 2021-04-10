#include "curl_multi_class.h"
#include <functional>

#ifdef AMXXCURL_DEBUG_LOG_ENABLE
#include <cstdio>
#define DEBUG_LOG(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define DEBUG_LOG(f_, ...) 
#endif

const char* whatstr[] = { "none", "IN", "OUT", "INOUT", "REMOVE" };

int CurlSocketCallbackStatic(CURL* easy, curl_socket_t s, int what, void* userp, void* socketp)
{
    return static_cast<CurlMulti*>(userp)->CurlSocketCallback(easy, s, what, socketp);
}

int CurlTimerCallbackStatic(CURLM* multi, long timeout_ms, void* userp)
{
    return static_cast<CurlMulti*>(userp)->CurlTimerCallback(multi, timeout_ms);
}

curl_socket_t CurlOpenSocketCallbackStatic(void* clientp, curlsocktype purpose, curl_sockaddr* address)
{
    return static_cast<CurlMulti*>(clientp)->CurlOpenSocketCallback(purpose, address);
}

int CurlCloseSocketCallbackStatic(void* clientp, curl_socket_t item)
{
    return static_cast<CurlMulti*>(clientp)->CurlCloseSocketCallback(item);
}

using namespace std::placeholders;

CurlMulti::CurlMulti(AsioPoller& asio_poller) :
    asio_poller_(asio_poller),
    running_handles_(0)
{
    curl_multi_ = curl_multi_init();
    curl_multi_setopt(curl_multi_, CURLMOPT_SOCKETFUNCTION, &CurlSocketCallbackStatic);
    curl_multi_setopt(curl_multi_, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(curl_multi_, CURLMOPT_TIMERFUNCTION, &CurlTimerCallbackStatic);
    curl_multi_setopt(curl_multi_, CURLMOPT_TIMERDATA, this);
}

CurlMulti::~CurlMulti()
{
    curl_multi_cleanup(curl_multi_);
}

void CurlMulti::AddCurl(Curl& curl, CurlMulti::CurlPerformComplete&& callback)
{
    curl_map_.emplace(curl.get_handle(), callback);

    curl.SetOption(CURLOPT_OPENSOCKETFUNCTION, &CurlOpenSocketCallbackStatic);
    curl.SetOption(CURLOPT_OPENSOCKETDATA, this);
    curl.SetOption(CURLOPT_CLOSESOCKETFUNCTION, &CurlCloseSocketCallbackStatic);
    curl.SetOption(CURLOPT_CLOSESOCKETDATA, this);

    CURLMcode code = curl_multi_add_handle(curl_multi_, curl.get_handle());
}

void CurlMulti::RemoveCurl(Curl& curl)
{
    curl_map_.erase(curl.get_handle());
    curl_multi_remove_handle(curl_multi_, curl.get_handle());
}

curl_socket_t CurlMulti::CurlOpenSocketCallback(curlsocktype purpose, curl_sockaddr* address)
{
    curl_socket_t sockfd = CURL_SOCKET_BAD;

    if (purpose == CURLSOCKTYPE_IPCXN && (address->family == AF_INET || address->family == AF_INET6))
    {
        asio::ip::tcp::socket tcp_socket = asio_poller_.CreateTcpSocket();

        asio::error_code ec;
        tcp_socket.open(address->family == AF_INET ? asio::ip::tcp::v4() : asio::ip::tcp::v6(), ec);

        if (ec)
        {
            DEBUG_LOG("Curl | open socket error: %d\n", ec);
            return CURL_SOCKET_BAD;
        }

        sockfd = tcp_socket.native_handle();

        socket_map_.emplace(sockfd, std::move(tcp_socket));
    }
    else
        DEBUG_LOG("Curl | can't open socket, invalid condition. purpose=%d; address->family=%d\n", purpose, address->family);

    DEBUG_LOG("Curl | open socket %s %d\n", (address->family == AF_INET ? "ipv4" : "ipv6"), sockfd);

    return sockfd;
}

int CurlMulti::CurlCloseSocketCallback(curl_socket_t item)
{
    if (socket_map_.find(item) != socket_map_.end())
    {
        socket_map_.erase(item);
    }

    DEBUG_LOG("Curl | close socket %d\n", item);

    return 0;
}

int CurlMulti::CurlSocketCallback(CURL* easy, curl_socket_t s, int what, void* socketp)
{
    DEBUG_LOG("Curl | CurlSocketCallback: socket=%d; what=%s; sockp=%d\n", s, whatstr[what], socketp);

    SocketData* socketData = (SocketData*)socketp;

    if (what == CURL_POLL_REMOVE)
    {
        if (socketData != nullptr)
        {
            if (socket_map_.find(s) != socket_map_.end())
            {
                if (socketData->is_ares_socket)
                {
                    socket_map_.at(s).release();
                    socket_map_.erase(s);
                }
            }

            removed_sockets_.emplace(s);
            delete socketData;
        }
    }
    else
    {
        if (socketData == nullptr)
        {
            socketData = new SocketData;

            if (socket_map_.find(s) == socket_map_.end())
            {
                socketData->is_ares_socket = true;

                asio::ip::tcp::socket tcp_socket = asio_poller_.WrapTcpSocket(s, asio::ip::tcp::v4());
                socket_map_.emplace(s, std::move(tcp_socket));
            }

            DEBUG_LOG("       Adding SocketData: what=%s; is ares socket: %i\n", whatstr[socketData->previous_action], socketData->is_ares_socket);
            curl_multi_assign(curl_multi_, s, socketData);
        }

        if (what != CURL_POLL_NONE)
        {
            DEBUG_LOG("       Changing action from %s to %s\n", whatstr[socketData->previous_action], whatstr[what]);
            SetSock(what, s, socketData);
        }
    }

    return 0;
}

int CurlMulti::CurlTimerCallback(CURLM* multi, long timeout_ms)
{
    auto& timer = asio_poller_.get_timer();

    DEBUG_LOG("Curl | Set timer: %ld\n", timeout_ms);
    timer.cancel();

    if (timeout_ms > 0)
    {
        timer.expires_from_now(std::chrono::milliseconds(timeout_ms));
        timer.async_wait(std::bind(&CurlMulti::AsioTimerCallback, this, _1));
    }
    else if (timeout_ms == 0)
    {
        curl_multi_socket_action(curl_multi_, CURL_SOCKET_TIMEOUT, 0, &running_handles_);
        CheckMultiInfo();
    }

    return 0;
}

// calls by asio when data r/w is ready
void CurlMulti::AsioSocketActionCallback(int action, curl_socket_t s, SocketData* socket_data, const asio::error_code& error)
{
    if (socket_map_.count(s) == 0)
    {
        return;
    }

    DEBUG_LOG("Asio cb | socket triggered; socket: %d; is_ares_socket: %d; action: %s; prev_action: %s\n", s, socket_data->is_ares_socket, whatstr[action], whatstr[socket_data->previous_action]);
    //DEBUG_LOG("Asio cb | socket triggered; socket: %d; is_ares_socket: %d\n", s, socket_data->is_ares_socket);

    if (error || action == socket_data->previous_action || socket_data->previous_action == CURL_POLL_INOUT)
    {
        if (error)
            action = CURL_CSELECT_ERR;

        removed_sockets_.clear();
        CURLMcode rc = curl_multi_socket_action(curl_multi_, s, action, &running_handles_);

        CheckMultiInfo();

        if (running_handles_ <= 0)
        {
            asio_poller_.get_timer().cancel();
            DEBUG_LOG("          cancel timer (no more handles)\n");
        }
        
        /* keep on watching.
         * the socket may have been closed and/or socket_data may have been changed
         * in curl_multi_socket_action(), so check them both */
        if (   !error
            && !removed_sockets_.count(s)
            && socket_map_.find(s) != socket_map_.end()
            && (action == socket_data->previous_action || socket_data->previous_action == CURL_POLL_INOUT))
        {
            auto& tcp_socket = socket_map_.find(s)->second;

            if (action & CURL_POLL_IN)
                tcp_socket.async_wait(asio::socket_base::wait_type::wait_read, std::bind(&CurlMulti::AsioSocketActionCallback, this, action, s, socket_data, _1));

            if (action & CURL_POLL_OUT)
                tcp_socket.async_wait(asio::socket_base::wait_type::wait_write, std::bind(&CurlMulti::AsioSocketActionCallback, this, action, s, socket_data, _1));
        }
    }
}

// calls by asio when need update curl
void CurlMulti::AsioTimerCallback(const asio::error_code& error)
{
    if (error)
    {
        //std::cout << "asio timer cb error: " << error.value() << " text: " << error.message() << std::endl;
        return;
    }

    DEBUG_LOG("Asio cb | timer triggered\n");

    curl_multi_socket_action(curl_multi_, CURL_SOCKET_TIMEOUT, 0, &running_handles_);
    CheckMultiInfo();
}

void CurlMulti::CheckMultiInfo()
{
    CURLMsg* msg;
    int msgs_left;

    CURL* easy;
    CURLcode res;

    while ((msg = curl_multi_info_read(curl_multi_, &msgs_left)))
    {
        if (msg->msg == CURLMSG_DONE)
        {
            easy = msg->easy_handle;
            res = msg->data.result;

            if (curl_map_.count(easy))
                curl_map_[easy](res);
        }
    }

    return;
}

void CurlMulti::SetSock(int act, curl_socket_t s, SocketData* socket_data)
{
    auto it = socket_map_.find(s);
    if (it == socket_map_.end())
        throw std::runtime_error("Invalid socket " + s);

    asio::ip::tcp::socket& tcp_socket = it->second;

    if (act == CURL_POLL_IN)
    {
        if (socket_data->previous_action != CURL_POLL_IN && socket_data->previous_action != CURL_POLL_INOUT)
        {
            tcp_socket.async_wait(asio::socket_base::wait_type::wait_read, std::bind(&CurlMulti::AsioSocketActionCallback, this, act, s, socket_data, _1));
        }
    }
    else if (act == CURL_POLL_OUT)
    {
        if (socket_data->previous_action != CURL_POLL_OUT && socket_data->previous_action != CURL_POLL_INOUT)
        {
            tcp_socket.async_wait(asio::socket_base::wait_type::wait_write, std::bind(&CurlMulti::AsioSocketActionCallback, this, act, s, socket_data, _1));
        }
    }
    else if (act == CURL_POLL_INOUT)
    {
        if (socket_data->previous_action != CURL_POLL_IN && socket_data->previous_action != CURL_POLL_INOUT)
        {
            tcp_socket.async_wait(asio::socket_base::wait_type::wait_read, std::bind(&CurlMulti::AsioSocketActionCallback, this, act, s, socket_data, _1));
        }
        if (socket_data->previous_action != CURL_POLL_OUT && socket_data->previous_action != CURL_POLL_INOUT)
        {
            tcp_socket.async_wait(asio::socket_base::wait_type::wait_write, std::bind(&CurlMulti::AsioSocketActionCallback, this, act, s, socket_data, _1));
        }
    }

    socket_data->previous_action = act;
}
