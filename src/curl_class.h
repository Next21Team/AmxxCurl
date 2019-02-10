#ifndef _CURL_CLASS_H_
#define _CURL_CLASS_H_

#include <string>
#include <memory>

#include <curl/curl.h>
#include "curl_callback_class.h"
#include "curl_utils_class.h"

class CurlInvalidOptionException : public std::exception
{
public:
    CurlInvalidOptionException(CURLoption option, const char* message) :
        message_(message), option_(option)
    {  }

    const char* what() const noexcept override { return message_.c_str(); }
    CURLoption get_option() const { return option_; }

private:
    std::string message_;
    CURLoption option_;
};

class CurlInitFailtureException : public std::exception
{
};

// wrapper for easy interface
class Curl
{
public:
    // curl_callback - класс в котором реализованы фунции обратного вызова для курла
    // Исключения: CurlInitFailtureException
    Curl(std::shared_ptr<CurlCallback> curl_callback) :
        curl_callback_(curl_callback)
    {
        InitCurl();
    }

    Curl(Curl&& other) :
        curl_(other.curl_),
        curl_callback_(other.curl_callback_)
    {
        other.curl_ = nullptr;
    }

    ~Curl()
    {
        if (curl_ != nullptr)
            curl_easy_cleanup(curl_);
    }

    // Установить значение опциии
    // Исключения: CurlInvalidOptionException
    template<class T>
    CURLcode SetOption(CURLoption option, T data) const
    {
        if(CurlUtils::IsDataOption(option) && option != CURLOPT_OPENSOCKETDATA && option != CURLOPT_CLOSESOCKETDATA)
            throw CurlInvalidOptionException(option, "can't set data option, use CurlCallback derived class to store data");

        if(CurlUtils::IsCallbackOption(option) && option != CURLOPT_OPENSOCKETFUNCTION && option != CURLOPT_CLOSESOCKETFUNCTION)
            throw CurlInvalidOptionException(option, "can't set callback option, use Curl::BindCallback to enable callbacks");

        return curl_easy_setopt(curl_, option, data);
    }

    // По умолчанию колбэки переданные в объекте curl_callback через конструктор
    // не связаны с дескриптором CURL*. Для связи используется этот метод.
    // Исключения: CurlInvalidOptionException
    void BindCallback(CURLoption option) const
    {
        if (!CurlUtils::IsCallbackOption(option))
            throw CurlInvalidOptionException(option, "callback option needed");

        CURLcode code;
        
        code = curl_easy_setopt(curl_, CurlUtils::GetDataOptionForCallbackOption(option), curl_callback_.get());
        if (code != CURLE_OK)
            throw CurlInvalidOptionException(option, "failture with code " + code);

        code = curl_easy_setopt(curl_, option, CurlCallback::GetMethodPointerForCallbackOption(option));
        if (code != CURLE_OK)
            throw CurlInvalidOptionException(option, "failture with code " + code);
    }

    CURLcode Perform() const noexcept
    {
        return curl_easy_perform(curl_);
    }

    template<class T>
    CURLcode GetInfo(CURLINFO curl_info, T& out_data) const noexcept
    {
        // TODO проеврка маски curl_info & CURLINFO_TYPEMASK ?

        return curl_easy_getinfo(curl_, curl_info, &out_data);
    }

    void Reset() const noexcept
    {
        curl_easy_reset(curl_);
    }

    void EscapeUrl(const char* url, std::string& out_escaped_url) const noexcept
    {
        char* escaped_url = curl_easy_escape(curl_, url, 0);
        out_escaped_url.assign(escaped_url);
        
        curl_free(escaped_url);
    }

    void UnescapeUrl(const char* url, std::string& out_unescaped_url) const noexcept
    {
        int outlen;
        char* unescaped_url = curl_easy_unescape(curl_, url, 0, &outlen);
        out_unescaped_url.assign(unescaped_url, outlen);

        curl_free(unescaped_url);
    }

    CURL* get_handle() const noexcept { return curl_; }
    std::shared_ptr<CurlCallback> get_curl_callback() const noexcept { return curl_callback_; }

private:
    void InitCurl()
    {
        curl_ = curl_easy_init();

        if (curl_ == nullptr)
            throw CurlInitFailtureException();
    }

    CURL* curl_;
    std::shared_ptr<CurlCallback> curl_callback_;
};

#endif // _CURL_CLASS_H_
