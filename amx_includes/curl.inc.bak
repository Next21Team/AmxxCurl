#if defined _curl_included
	#endinput
#endif
#define _curl_included

#include <curl_consts>

#if AMXX_VERSION_NUM >= 175
 #pragma reqlib curl
 #if !defined AMXMODX_NOAUTOLOAD
  #pragma loadlib curl
 #endif
#else
 #pragma library curl
#endif

enum _:curl_off_t {
	curl_off_left,
	curl_off_right
}

native curl_easy_escape(const CURL:handle, const url[], escapedUrl[], const len)
native curl_easy_unescape(const CURL:handle, const url[], unescapedUrl[], const len)

native CURL:curl_easy_init()
native void:curl_easy_perform(const CURL:handle, const callbackComplite[], const data[] = {}, const data_len = 0)
native CURLcode:curl_easy_setopt(const CURL:handle, const CURLoption:option, any:...)
native void:curl_easy_cleanup(const CURL:handle)
native void:curl_easy_reset(const CURL:handle)
native CURLcode:curl_easy_getinfo(const CURL:handle, const CURLINFO:info, any:...)
native void:curl_easy_strerror(const CURLcode:code, errorBuf[], const len)

native CURLFORMcode:curl_formadd(&curl_httppost:first, &curl_httppost:last, any:...)
native void:curl_formfree(&curl_httppost:first)

native curl_slist:curl_slist_append(curl_slist:list, buffer[])
native void:curl_slist_free_all(curl_slist:list)

native void:curl_version(buf[], len)
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1049\\ f0\\ fs16 \n\\ par }
*/
