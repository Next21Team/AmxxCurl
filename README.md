# Description

AmxxCurl module is a wrapper over libcurl (with ssl support) easy interface for amxmodx. Module performs curl's in non-blocking mode.

Download latest version [here](https://github.com/Polarhigh/AmxxCurl/releases).

# Compilation

You must have installed the latest version premake5. ([Get it here](https://github.com/premake/premake-core))

Also you should put all dependent libraries in deps/Release/lib derictory, for windows it zlib_a.lib and libcurl_a.lib. For linux libcrypro.a, libssl.a, libcurl.a and libz.a. And related includes in deps/Release/include.

## Windows

First, set the environment variables HLSDK and METAMOD.

Then, generate Visual Studio project:

    premake5 vs2015

_You may try generate project for another Visual Studio version, but only vs2015 was tested._

Now open solution and compile it)

## Linux

First, set the environment variables:

    export HLSDK=~/path_to/halflife
    export METAMOD=~/path_to/metamod-hl1/metamod

Then, call premake5 for generate Makefile:

    premake5 gmake

And now compile:

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
