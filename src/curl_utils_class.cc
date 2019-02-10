#include "curl_utils_class.h"
#include <stdexcept>

int CurlUtils::GetInterruptCodeForCurlCallback(CURLoption callback_option)
{
    switch (callback_option) {
    case CURLOPT_WRITEFUNCTION:         return 0;
    case CURLOPT_READFUNCTION:          return CURL_READFUNC_ABORT;
    case CURLOPT_IOCTLFUNCTION:         return CURLIOE_UNKNOWNCMD;
    case CURLOPT_SEEKFUNCTION:          return 1;
    case CURLOPT_SOCKOPTFUNCTION:       return 1;
    case CURLOPT_OPENSOCKETFUNCTION:    return CURL_SOCKET_BAD;
    case CURLOPT_CLOSESOCKETFUNCTION:   return 1;
    case CURLOPT_PROGRESSFUNCTION:      return 1;
    case CURLOPT_XFERINFOFUNCTION:      return 1;
    case CURLOPT_HEADERFUNCTION:        return 0;
    case CURLOPT_DEBUGFUNCTION:         return 1; // ??
    case CURLOPT_SSL_CTX_FUNCTION:      return CURLE_GOT_NOTHING;
    case CURLOPT_INTERLEAVEFUNCTION:    return 0; // ??
    case CURLOPT_CHUNK_BGN_FUNCTION:    return CURL_CHUNK_BGN_FUNC_FAIL;
    case CURLOPT_CHUNK_END_FUNCTION:    return CURL_CHUNK_END_FUNC_FAIL;
    case CURLOPT_FNMATCH_FUNCTION:      return CURL_FNMATCHFUNC_FAIL;

    default:
        throw std::runtime_error("Invalid option");
    }
}

CURLoption CurlUtils::GetDataOptionForCallbackOption(CURLoption callback_option)
{
    switch (callback_option) {
    case CURLOPT_WRITEFUNCTION:         return CURLOPT_WRITEDATA;
    case CURLOPT_READFUNCTION:          return CURLOPT_READDATA;
    case CURLOPT_IOCTLFUNCTION:         return CURLOPT_IOCTLDATA;
    case CURLOPT_SEEKFUNCTION:          return CURLOPT_SEEKDATA;
    case CURLOPT_SOCKOPTFUNCTION:       return CURLOPT_SOCKOPTDATA;
    case CURLOPT_OPENSOCKETFUNCTION:    return CURLOPT_OPENSOCKETDATA;
    case CURLOPT_CLOSESOCKETFUNCTION:   return CURLOPT_CLOSESOCKETDATA;
    case CURLOPT_PROGRESSFUNCTION:      return CURLOPT_PROGRESSDATA;
    case CURLOPT_XFERINFOFUNCTION:      return CURLOPT_XFERINFODATA;
    case CURLOPT_HEADERFUNCTION:        return CURLOPT_HEADERDATA;
    case CURLOPT_DEBUGFUNCTION:         return CURLOPT_DEBUGDATA;
    case CURLOPT_SSL_CTX_FUNCTION:      return CURLOPT_SSL_CTX_DATA;
    case CURLOPT_INTERLEAVEFUNCTION:    return CURLOPT_INTERLEAVEDATA;
    case CURLOPT_CHUNK_BGN_FUNCTION:    return CURLOPT_CHUNK_DATA;
    case CURLOPT_CHUNK_END_FUNCTION:    return CURLOPT_CHUNK_DATA;
    case CURLOPT_FNMATCH_FUNCTION:      return CURLOPT_FNMATCH_DATA;
    default:
        throw std::runtime_error("Unsupported callback option");
    }
}

bool CurlUtils::IsDataOption(CURLoption option) noexcept
{
    switch (option)
    {
    case CURLOPT_WRITEDATA:
    case CURLOPT_READDATA:
    case CURLOPT_IOCTLDATA:
    case CURLOPT_SEEKDATA:
    case CURLOPT_SOCKOPTDATA:
    case CURLOPT_OPENSOCKETDATA:
    case CURLOPT_CLOSESOCKETDATA:
    case CURLOPT_PROGRESSDATA:
        //case CURLOPT_XFERINFODATA:
    case CURLOPT_HEADERDATA:
    case CURLOPT_DEBUGDATA:
    case CURLOPT_SSL_CTX_DATA:
    case CURLOPT_INTERLEAVEDATA:
    case CURLOPT_CHUNK_DATA:
    case CURLOPT_FNMATCH_DATA:
        return true;

    default:
        return false;
    }
}

bool CurlUtils::IsCallbackOption(CURLoption option) noexcept
{
    switch (option) {
    case CURLOPT_WRITEFUNCTION:
    case CURLOPT_READFUNCTION:
    case CURLOPT_IOCTLFUNCTION:
    case CURLOPT_SEEKFUNCTION:
    case CURLOPT_SOCKOPTFUNCTION:
    case CURLOPT_OPENSOCKETFUNCTION:
    case CURLOPT_CLOSESOCKETFUNCTION:
    case CURLOPT_PROGRESSFUNCTION:
    case CURLOPT_XFERINFOFUNCTION:
    case CURLOPT_HEADERFUNCTION:
    case CURLOPT_DEBUGFUNCTION:
    case CURLOPT_SSL_CTX_FUNCTION:
    case CURLOPT_INTERLEAVEFUNCTION:
    case CURLOPT_CHUNK_BGN_FUNCTION:
    case CURLOPT_CHUNK_END_FUNCTION:
    case CURLOPT_FNMATCH_FUNCTION:
        return true;

    default:
        return false;
    }
}
