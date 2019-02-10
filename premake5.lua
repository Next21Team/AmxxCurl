-- Clean Function --
newaction {
  trigger     = "clean",
  description = "clean the software",
    execute     = function ()
      print("clean the build...")
      os.rmdir("./build")
      os.rmdir("./bin")
      os.rmdir("./obj")
      os.rmdir("./.vs")
      print("done.")
  end
}

-- workspace --
workspace "AmxxCurl"
  location ("build/" .. _ACTION)
  configurations { "ReleaseDLL", "DebugDLL" }
  staticruntime "On"
  architecture "x86"
  cppdialect "C++14"
  
  characterset("MBCS") -- use multibyte character set in msvs

  -- flags & options --
  flags { "No64BitChecks", "MultiProcessorCompile" }
  defines "NO_MSVC8_AUTO_COMPAT"
  
  filter "configurations:DebugDLL"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"

  filter "configurations:ReleaseDLL"
    defines { "NDEBUG" }
    optimize "Full"
    flags { "NoBufferSecurityCheck", "NoRuntimeChecks" }

  filter { "system:windows", "configurations:ReleaseDLL" }
    flags { "NoIncrementalLink", "LinkTimeOptimization" }
	
  filter { "system:windows" }
    defines "_CRT_SECURE_NO_WARNINGS"
    systemversion "latest"

  filter "configurations:*"
    defines {
      "HAVE_STDINT_H", -- prevent C2371 for int32_t in amxmodx
      "CURL_STATICLIB"
    }

project "AmxxCurl"
  targetname  "amxxcurl_amxx_i386"
  kind        "SharedLib"
  language    "C++"
  targetdir   "bin/%{cfg.buildcfg}"
  includedirs {
    "deps/halflife/dlls",
    "deps/halflife/engine",
    "deps/halflife/common",
    "deps/halflife/public",
    "deps/metamod",
  }
  
  -- src, includes & libs --
  files { "src/**.h", "src/**.cc", "src/sdk/**.cpp" }

  filter "configurations:ReleaseDLL"
    includedirs { "deps/Release/include" }
	libdirs { "deps/Release/lib" }

  filter "configurations:DebugDLL"
    includedirs { "deps/Release/include" }
	libdirs { "deps/Debug/lib" }

  --
  filter "system:windows"
    links { "Ws2_32", "Crypt32", "Wldap32", "Normaliz", "zlib_a", "libcurl_a" }

  filter "system:linux"
    defines "AMXXCURL_USE_PTHREADS_EXPLICITLY"
    links { "pthread" }
    toolset "gcc"
    linkgroups "On"
    buildoptions { "-fpermissive" }
    linkoptions { "-static-libgcc -static-libstdc++ -Wl,--no-as-needed" }
  
  filter { "system:linux", "configurations:ReleaseDLL" }
    linkoptions { "-Wl,--start-group " .. path.getabsolute("deps/Release/lib/libcrypto.a") .. " " .. path.getabsolute("deps/Release/lib/libssl.a") .. " " .. path.getabsolute("deps/Release/lib/libcurl.a") .. " " ..  path.getabsolute("deps/Release/lib/libz.a") .. " -Wl,--end-group" }
  
  filter { "system:linux", "configurations:DebugDLL" }
    linkoptions { "-Wl,--start-group " .. path.getabsolute("deps/Debug/lib/libcrypto.a") .. " " .. path.getabsolute("deps/Debug/lib/libssl.a") .. " " .. path.getabsolute("deps/Debug/lib/libcurl.a") .. " " ..  path.getabsolute("deps/Debug/lib/libz.a") .. " -Wl,--end-group" }

--[[ 
bild libcurl win:
1. run buildconf.bat
2. run developer command promt for vs
3. cd winbuild
4. nmake /f Makefile.vc mode=static VC=15 WITH_ZLIB=static ENABLE_SSPI=yes ENABLE_IPV6=yes ENABLE_IDN=yes ENABLE_WINSSL=yes GEN_PDB=no DEBUG=no RTLIBCFG=static MACHINE=x86 ZLIB_PATH=../../../deps

--]]
