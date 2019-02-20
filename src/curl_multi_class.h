#ifndef _CURL_MULTI_CLASS_H_
#define _CURL_MULTI_CLASS_H_

#include <map>
#include <ares.h>
#include "asio_poller.h"
#include "curl_class.h"

struct SocketData
{
    bool is_ares_socket = false;
    int previous_action = CURL_POLL_NONE; // CURL_POLL_IN, CURL_POLL_OUT, etc
};

class CurlMulti
{
public:
    using CurlPerformComplete = std::function<void(CURLcode)>;

    CurlMulti(AsioPoller& asio_poller);
    ~CurlMulti();

    void AddCurl(Curl& curl, CurlMulti::CurlPerformComplete&& callback);
    void RemoveCurl(Curl& curl);

    curl_socket_t CurlOpenSocketCallback(curlsocktype purpose, struct curl_sockaddr *address);
    int CurlCloseSocketCallback(curl_socket_t item);

    int CurlSocketCallback(CURL *easy, curl_socket_t s, int what, void *socketp);
    int CurlTimerCallback(CURLM *multi, long timeout_ms);

private:
    CurlMulti(const CurlMulti& curl_multi);
    void CheckMultiInfo();
    void SetSock(int act, curl_socket_t s, SocketData* socketData);
    void AsioSocketActionCallback(int act, curl_socket_t s, SocketData* socketData, const asio::error_code& error);
    void AsioTimerCallback(const asio::error_code& error);

private:
    CURLM* curl_multi_;
    AsioPoller& asio_poller_;
    std::map<CURL*, CurlMulti::CurlPerformComplete> curl_map_;
    std::map<curl_socket_t, asio::ip::tcp::socket> socket_map_;
    int running_handles_;
};

#endif // _CURL_MULTI_CLASS_H_
