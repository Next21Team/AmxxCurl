#ifndef _CURL_CALLBACK_CLASS_H_
#define _CURL_CALLBACK_CLASS_H_

#include <exception>
#include <string>

#include <curl/curl.h>


class CurlUnhandledCallbackException : public std::exception
{
public:
    CurlUnhandledCallbackException(const char* callback_name) :
        callback_name_(callback_name)
    {
        message_.append("Derived class not implements callaback ").append(callback_name_);
    }

    const char* what() const noexcept override { return message_.c_str(); }
    const char* get_callback_name() const { return callback_name_.c_str(); }

private:
    std::string callback_name_;
    std::string message_;
};

class CurlCallback
{

public:
    virtual ~CurlCallback() { }


protected:
    virtual size_t WriteCallback(char *ptr, size_t size, size_t nmemb)
    {
        throw CurlUnhandledCallbackException("WriteCallback");
    }

    virtual size_t ReadCallback(char *buffer, size_t size, size_t nitems)
    {
        throw CurlUnhandledCallbackException("ReadCallback");
    }

    virtual curlioerr IoctlCallback(CURL *handle, int cmd)
    {
        throw CurlUnhandledCallbackException("IoctlCallback");
    }

    virtual int SeekCallback(curl_off_t offset, int origin)
    {
        throw CurlUnhandledCallbackException("SeekCallback");
    }

    virtual int SockoptCallback(curl_socket_t curlfd, curlsocktype purpose)
    {
        throw CurlUnhandledCallbackException("SockoptCallback");
    }

    virtual int ProgressCallback(double dltotal, double dlnow, double ultotal, double ulnow)
    {
        throw CurlUnhandledCallbackException("ProgressCallback");
    }

    virtual int ProgressCallbackx(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        throw CurlUnhandledCallbackException("ProgressCallbackx");
    }

    virtual size_t HeaderCallback(char *buffer, size_t size, size_t nitems)
    {
        throw CurlUnhandledCallbackException("HeaderCallback");
    }

    virtual int DebugCallback(CURL *handle, curl_infotype type, char *data, size_t size)
    {
        throw CurlUnhandledCallbackException("DebugCallback");
    }

    virtual CURLcode SslCtxCallback(CURL *curl, void *ssl_ctx)
    {
        throw CurlUnhandledCallbackException("SslCtxCallback");
    }

    virtual size_t InterleaveCallback(void *ptr, size_t size, size_t nmemb)
    {
        throw CurlUnhandledCallbackException("InterleaveCallback");
    }

    virtual long ChunkBgnCallback(const void *transfer_info, int remains)
    {
        throw CurlUnhandledCallbackException("ChunkBgnCallback");
    }

    virtual long ChunkEndCallback()
    {
        throw CurlUnhandledCallbackException("ChunkEndCallback");
    }

    virtual int FnmatchCallback(const char *pattern, const char *string)
    {
        throw CurlUnhandledCallbackException("FnmatchCallback");
    }

public:
    // Static callbacks

    static size_t WriteCallbackStatic(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        return static_cast<CurlCallback*>(userdata)->WriteCallback(ptr, size, nmemb);
    }

    static size_t ReadCallbackStatic(char *buffer, size_t size, size_t nitems, void *instream)
    {
        return static_cast<CurlCallback*>(instream)->ReadCallback(buffer, size, nitems);
    }

    static curlioerr IoctlCallbackStatic(CURL *handle, int cmd, void *clientp)
    {
        return static_cast<CurlCallback*>(clientp)->IoctlCallback(handle, cmd);
    }

    static int SeekCallbackStatic(void *userp, curl_off_t offset, int origin)
    {
        return static_cast<CurlCallback*>(userp)->SeekCallback(offset, origin);
    }

    static int SockoptCallbackStatic(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
    {
        return static_cast<CurlCallback*>(clientp)->SockoptCallback(curlfd, purpose);
    }

    static int ProgressCallbackStatic(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        return static_cast<CurlCallback*>(clientp)->ProgressCallback(dltotal, dlnow, ultotal, ulnow);
    }

    static int ProgressCallbackxStatic(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        return static_cast<CurlCallback*>(clientp)->ProgressCallbackx(dltotal, dlnow, ultotal, ulnow);
    }

    static size_t HeaderCallbackStatic(char *buffer, size_t size, size_t nitems, void *userdata)
    {
        return static_cast<CurlCallback*>(userdata)->HeaderCallback(buffer, size, nitems);
    }

    static int DebugCallbackStatic(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr)
    {
        return static_cast<CurlCallback*>(userptr)->DebugCallback(handle, type, data, size);
    }

    static CURLcode SslCtxCallbackStatic(CURL *curl, void *ssl_ctx, void *userptr)
    {
        return static_cast<CurlCallback*>(userptr)->SslCtxCallback(curl, ssl_ctx);
    }

    static size_t InterleaveCallbackStatic(void *ptr, size_t size, size_t nmemb, void *userdata)
    {
        return static_cast<CurlCallback*>(userdata)->InterleaveCallback(ptr, size, nmemb);
    }

    static long ChunkBgnCallbackStatic(const void *transfer_info, void *ptr, int remains)
    {
        return static_cast<CurlCallback*>(ptr)->ChunkBgnCallback(transfer_info, remains);
    }

    static long ChunkEndCallbackStatic(void *ptr)
    {
        return static_cast<CurlCallback*>(ptr)->ChunkEndCallback();
    }

    static int FnmatchCallbackStatic(void *ptr, const char *pattern, const char *string)
    {
        return static_cast<CurlCallback*>(ptr)->FnmatchCallback(pattern, string);
    }

    static void* GetMethodPointerForCallbackOption(CURLoption callback_option)
    {
        switch (callback_option) {
        case CURLOPT_WRITEFUNCTION:         return reinterpret_cast<void*>(WriteCallbackStatic);
        case CURLOPT_READFUNCTION:          return reinterpret_cast<void*>(ReadCallbackStatic);
        case CURLOPT_IOCTLFUNCTION:         return reinterpret_cast<void*>(IoctlCallbackStatic);
        case CURLOPT_SEEKFUNCTION:          return reinterpret_cast<void*>(SeekCallbackStatic);
        case CURLOPT_SOCKOPTFUNCTION:       return reinterpret_cast<void*>(SockoptCallbackStatic);
        case CURLOPT_PROGRESSFUNCTION:      return reinterpret_cast<void*>(ProgressCallbackStatic);
        case CURLOPT_XFERINFOFUNCTION:      return reinterpret_cast<void*>(ProgressCallbackxStatic);
        case CURLOPT_HEADERFUNCTION:        return reinterpret_cast<void*>(HeaderCallbackStatic);
        case CURLOPT_DEBUGFUNCTION:         return reinterpret_cast<void*>(DebugCallbackStatic);
        case CURLOPT_SSL_CTX_FUNCTION:      return reinterpret_cast<void*>(SslCtxCallbackStatic);
        case CURLOPT_INTERLEAVEFUNCTION:    return reinterpret_cast<void*>(InterleaveCallbackStatic);
        case CURLOPT_CHUNK_BGN_FUNCTION:    return reinterpret_cast<void*>(ChunkBgnCallbackStatic);
        case CURLOPT_CHUNK_END_FUNCTION:    return reinterpret_cast<void*>(ChunkEndCallbackStatic);
        case CURLOPT_FNMATCH_FUNCTION:      return reinterpret_cast<void*>(FnmatchCallbackStatic);
        default:
            throw std::runtime_error("Unsupported callback option");
        }
    }
};

#endif // _CURL_CALLBACK_CLASS_H_
