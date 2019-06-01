# Description

AmxxCurl module is a wrapper over libcurl (with ssl support) easy interface for amxmodx. Module performs curl's in non-blocking mode.

Download latest version [here](https://github.com/Polarhigh/AmxxCurl/releases).

# Roadmap

1. ~~Improving performance: using asio, curl multi interface and c-areas.~~
2. Adding simple interfaces for most common cases.
3. Improving build scripts.

# Compilation

You must have installed the latest version premake5. ([Get it here](https://github.com/premake/premake-core))

## Windows

Generate Visual Studio project:

    premake5 vs2017

_You may try generate project for another Visual Studio version._

Open solution and compile it)

## Linux

Generate Makefile:

    premake5 gmake

And compile it:

    cd build/gmake
    make

# Natives

All natives functions you can find in [curl.inc](https://github.com/Polarhigh/AmxxCurl/blob/master/amx_includes/curl.inc), they have the same behaviour that [C functions](https://curl.haxx.se/libcurl/c/).

Except for some:

1)

    native void:curl_easy_perform(const CURL:handle, const callbackComplite[], const data[] = {}, const data_len = 0)

The function starts curl perform, and upon completion of the transfer will be call callback callbackComplite. In third parameter you can specify an array of user data, wich will be transferred to callbackComplite, and fourth parameter is array length.

If user data set, callback should have the signature:

    public compliteCallback(CURL:curl, CURLcode:code, data[])

else:

    public compliteCallback(CURL:curl, CURLcode:code)

2)

    native CURLcode:curl_easy_getinfo(const CURL:handle, const CURLINFO:info, any:...)

For string type of CURLINFO option, you must set fourth argument - string size:

    new url[64]
    curl_easy_getinfo(curl,CURLINFO_EFFECTIVE_URL, url, charsmax(url))

# Other

If this description contains a strange phrases, please do a fix commit)
