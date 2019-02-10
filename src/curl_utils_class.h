#ifndef _CURL_UTILS_CLASS_H_
#define _CURL_UTILS_CLASS_H_

#include <curl\curl.h>

class CurlUtils
{
public:
    static int CurlUtils::GetInterruptCodeForCurlCallback(CURLoption callback_option);
    static CURLoption CurlUtils::GetDataOptionForCallbackOption(CURLoption callback_option);
    static bool IsDataOption(CURLoption option) noexcept;
    static bool IsCallbackOption(CURLoption option) noexcept;
};

#endif // _CURL_UTILS_CLASS_H_
