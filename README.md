# Description

AmxxCurl module is a wrapper over libcurl easy interface for amxmodx. Module performs curl's in non-blocking mode.

Download latest version [here](https://github.com/Polarhigh/AmxxCurl/releases).

## Remarks for installation Linux version

This module dynamically linked with openssl, and therefore if it asent in system you'll get load failure.

And therefore:

-   You must have installed metamod plugin [SoLoader](https://github.com/Polarhigh/SoLoader/releases) (and it must be written on top addons/metamod/plugins.ini);
-   And then add lines in addons/soloader/libraries.cfg:


    libcrypto.so.1.0.0
    libssl.so.1.0.0

# Compilation

You must have installed the latest version premake5. ([Get it here](https://github.com/premake/premake-core))

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

All natives functions you can find in curl.inc, they have the same behaviour that [C functions](https://curl.haxx.se/libcurl/c/).

Except for some:

    native void:curl_easy_perform(const CURL:handle, const callbackComplite[], const data[] = {}, const data_len = 0)

The function starts executing in new thread, after, callbackComplite will be called. The third and fourth parameters set the addition data that will be sent in the third argument complete callback:
`public compliteCallback(CURL:curl, CURLcode:code, data[])`

# Other

If this description contains a strange phrases, please do a fix commit)
