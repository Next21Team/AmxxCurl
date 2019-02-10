#ifndef _AMX_CURL_CONTROLLER_H_
#define _AMX_CURL_CONTROLLER_H_

#include "amx_curl_manager_class.h"

class AmxCurlController
{
public:
    AmxCurlManager& get_curl_manager() { return curl_manager_; }
    AsioPoller& get_asio_poller() { return asio_poller_; }

    static AmxCurlController& Instance()
    {
        static AmxCurlController instance;
        return instance;
    }

private:
    AmxCurlController() :
        curl_manager_(asio_poller_)
    { }

    AmxCurlController(const AmxCurlController& root);
    AmxCurlController& operator=(const AmxCurlController&);

    AmxCurlManager curl_manager_;
    AsioPoller asio_poller_;
};

#endif // _AMX_CURL_CONTROLLER_H_
