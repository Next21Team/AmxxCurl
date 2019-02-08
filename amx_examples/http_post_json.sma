#include <amxmodx>
#include <curl>

#pragma semicolon 1
#pragma ctrlchar '\'

public plugin_init()
{
    register_plugin("HTTP: Post request", "1.0.0", "gamingEx aka. Kaido Ren");
    @requestBegin();
}

enum dataStruct { curl_slist: linkedList };

@requestBegin()
{
    new CURL: pCurl, curl_slist: pHeaders, sData[dataStruct];

    pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");
    pHeaders = curl_slist_append(pHeaders, "User-Agent: curl");

    sData[linkedList] = pHeaders;

    if ((pCurl = curl_easy_init())) {
        curl_easy_setopt(pCurl, CURLOPT_URL, "http://jsonplaceholder.typicode.com/posts");
        curl_easy_setopt(pCurl, CURLOPT_COPYPOSTFIELDS, "{\"title\": \"foo\", \"body\": \"bar\", \"userId\": 1}");
        curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, "@responseWrite");

        curl_easy_perform(pCurl, "@requestComplete", sData, dataStruct);
    }
}

@responseWrite(const data[], const size, const nmemb)
{
    server_print("Response body: \n%s", data);
 
    return size * nmemb; // tell curl how many bytes we handled
}

@requestComplete(CURL: curl, CURLcode: code, const data[dataStruct])
{
    if (code != CURLE_OK) {
        new szError[128];
        curl_easy_strerror(code, szError, charsmax(szError));
        server_print("CURL: %s", szError);
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(data[linkedList]);
}