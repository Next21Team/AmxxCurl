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
};

enum curl_slist
{
    SList_Empty
};

/**
 * This function converts the given input string to a URL encoded string.
 * All input characters that are not a-z, A-Z, 0-9, '-', '.', '_' or '~' are converted to their "URL escaped" version 
 * (%NN where NN is a two-digit hexadecimal number).
 * see also https://curl.haxx.se/libcurl/c/curl_easy_escape.html
 *
 * @param handle            Curl handle
 * @param url               URL for encoding
 * @param buffer            Buffer to copy encoded url
 * @param maxlen            Maximum size of the buffer
 *
 * @noreturn
 * @error                   If passed curl handle is not a valid
 */
native curl_easy_escape(const CURL:handle, const url[], buffer[], const maxlen);

/**
 * This function converts the given URL encoded input string to a "plain string".
 * All input characters that are URL encoded (%XX where XX is a two-digit hexadecimal number) are converted to their binary versions.
 * see also https://curl.haxx.se/libcurl/c/curl_easy_unescape.html
 *
 * @param handle            Curl handle
 * @param url               URL for decoding
 * @param buffer            Buffer to copy decoded url
 * @param maxlen            Maximum size of the buffer
 *
 * @noreturn
 * @error                   If passed curl handle is not a valid
 */
native curl_easy_unescape(const CURL: handle, const url[], buffer[], const maxlen);

/**
 * This function must be the first function to call, and it returns a CURL easy handle that you must
 * use as input to other functions in the easy interface. This call MUST have a corresponding call to curl_easy_cleanup
 * when the operation is complete.
 * see also https://curl.haxx.se/libcurl/c/curl_easy_init.html 
 *
 * @return                  Curl handle
 */
native CURL: curl_easy_init();

/**
 * Invoke this function after curl_easy_init and all the curl_easy_setopt calls are made, and will perform the transfer as described in the options.
 * It must be called with the same curl handle as input as the curl_easy_init call returned.
 * You can do any amount of calls to curl_easy_perform while using the same curl handle. If you intend to transfer more than one file, you are even 
 * encouraged to do so. libcurl will then attempt to re-use the same connection for the following transfers, thus making the operations faster, 
 * less CPU intense and using less network resources. Just note that you will have to use curl_easy_setopt between the invokes to set options 
 * for the following curl_easy_perform.
 * see also https://curl.haxx.se/libcurl/c/curl_easy_perform.html
 *
 * @param handle            Curl handle
 * @param callback          The forward to call after request completed
 * @param data              Any data to pass to the callback forward
 * @param len               Maximum size of the data
 *
 * @noreturn
 * @error                   If passed curl handle is not a valid or or undefined callback function
 */
native curl_easy_perform(const CURL: handle, const callback[], const data[] = {}, const len = 0);

/**
 * This function is used to tell libcurl how to behave. By setting the appropriate options, the application can change libcurl's behavior.
 * All options are set with an option followed by a parameter. That parameter can be a long, a function pointer, an object pointer or a curl_off_t, 
 * depending on what the specific option expects. Read this manual carefully as bad input values may cause libcurl to behave badly! 
 * You can only set one option in each function call. A typical application uses many curl_easy_setopt calls in the setup phase.
 * Options set with this function call are valid for all forthcoming transfers performed using this handle. The options are not in any way reset between
 * transfers, so if you want subsequent transfers with different options, you must change them between the transfers. You can optionally reset all options
 * back to internal default with curl_easy_reset.
 * see also https://curl.haxx.se/libcurl/c/curl_easy_setopt.html
 *
 * @param handle            Curl handle
 * @param option            Necessary option (see CURLoption enum)
 *
 * @return                  If the operation was successful, CURLE_OK is returned. Otherwise an appropriate error code will be returned.
 * @error                   If passed curl handle is not a valid or or undefined option
 */
native CURLcode: curl_easy_setopt(const CURL: handle, const CURLoption: option, any: ...);

/**
 * This function must be the last function to call for an easy session. It is the opposite of the curl_easy_init function and must be called
 * with the same handle as input that a curl_easy_init call returned.
 * This might close all connections this handle has used and possibly has kept open until now - unless it was attached to a multi handle while
 * doing the transfers. Don't call this function if you intend to transfer more files, re-using handles is a key to good performance with libcurl.
 * see also https://curl.haxx.se/libcurl/c/curl_easy_cleanup.html
 *
 * @param handle            Curl handle
 *
 * @noreturn
 * @error                   If passed curl handle is not a valid
 */
native curl_easy_cleanup(const CURL: handle);

/**
 * Re-initializes all options previously set on a specified CURL handle to the default values. This puts back the handle to the same state as it
 * was in when it was just created with curl_easy_init.
 * It does not change the following information kept in the handle: live connections, the Session ID cache, the DNS cache, the cookies and shares.
 * see also https://curl.haxx.se/libcurl/c/curl_easy_reset.html
 *
 * @param handle            Curl handle
 *
 * @noreturn
 * @error                   If passed curl handle is not a valid
 */
native curl_easy_reset(const CURL: handle);

/**
 * Request internal information from the curl session with this function. The third argument MUST be a buffer for value (num, string, float) 
 * The data pointed-to will be filled in accordingly and can be relied upon only if the function returns CURLE_OK. Use this function AFTER 
 * a performed transfer if you want to get transfer related data.
 * see also https://curl.haxx.se/libcurl/c/curl_easy_getinfo.html
 *
 * @param handle            Curl handle
 * @param info              Necessary info (see CURLINFO enum)
 *
 * @return                  If the operation was successful, CURLE_OK is returned. Otherwise an appropriate error code will be returned.
 * @error                   If passed curl handle is not a valid or or undefined info
 */
native CURLcode: curl_easy_getinfo(const CURL: handle, const CURLINFO: info, any: ...);

/**
 * The function receives a text description of the specified error code.
 *
 * @param code              Error code (see CURLcode enum)
 * @param buffer            Buffer to copy error description
 * @param maxlen            Maximum size of the buffer
 *
 * @noreturn
 */
native curl_easy_strerror(const CURLcode: code, buffer[], const maxlen);

/**
* @deprecated               This function does not catch all cases.
*/
#pragma deprecated This function is deprecated. Do not use!
native CURLFORMcode: curl_formadd(&curl_httppost: first, &curl_httppost: last, any: ...);

/**
* @deprecated               This function does not catch all cases.
*/
#pragma deprecated This function is deprecated. Do not use!
native curl_formfree(&curl_httppost: first);

/**
 * This function appends a string to a linked list of strings.
 * The existing list should be passed as the first argument and the new list is returned from this function.
 * Pass in SList_Empty (0) in the list argument to create a new list. The specified string has been appended when this function returns.
 * see also https://curl.haxx.se/libcurl/c/curl_slist_append.html
 *
 * @param list              Existing list
 * @param string            Some string
 *
 * @return                  A null pointer is returned if anything went wrong, otherwise the new list pointer is returned. 
 *                          To avoid overwriting an existing non-empty list on failure, the new list should be returned to a 
 *                          temporary variable which can be tested for SList_Empty (0) before updating the original list pointer
 */
native curl_slist: curl_slist_append(curl_slist: list, string[]);

/**
 * This function removes all traces of a previously built curl_slist linked list.
 * see also https://curl.haxx.se/libcurl/c/curl_slist_free_all.html 
 *
 * @param list              Existing list
 *
 * @noreturn
 */
native curl_slist_free_all(curl_slist: list);

/**
 * Returns a human readable string with the version number of libcurl and some of its important components (like OpenSSL version).
 * see also https://curl.haxx.se/libcurl/c/curl_version.html
 *
 * @param buffer            Buffer to copy string with the version number
 * @param maxlen            Maximum size of the buffer
 *
 * @noreturn
 */
native curl_version(buffer[], const maxlen);
