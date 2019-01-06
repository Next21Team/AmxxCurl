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
    os.getenv("HLSDK") .. "/dlls",
    os.getenv("HLSDK") .. "/engine",
    os.getenv("HLSDK") .. "/common",
    os.getenv("HLSDK") .. "/public",
    os.getenv("METAMOD"),
  }
  
  -- src, includes & libs --
  files { "src/**.h", "src/**.cc", "src/sdk/**.cpp" }
  includedirs { "deps/include" }

  libdirs { "deps/lib" }
  
  --
  filter "system:windows"
    links { "Ws2_32", "Crypt32", "Wldap32", "Normaliz", "zlib_a", "libcurl_a" }

  filter "system:linux"
    links { "pthread" }
    toolset "gcc"
    linkgroups "On"
    buildoptions { "-fpermissive" }
    linkoptions { "-static-libgcc -static-libstdc++ -Wl,--start-group " .. path.getabsolute("deps/lib/libcrypto.a") .. " " .. path.getabsolute("deps/lib/libssl.a") .. " " .. path.getabsolute("deps/lib/libcurl.a") .. " " ..  path.getabsolute("deps/lib/libz.a") .. " -Wl,--end-group" }

--[[ 
bild libcurl win:
1. run buildconf.bat
2. run developer command promt for vs
3. cd winbuild
4. nmake /f Makefile.vc mode=static VC=15 WITH_ZLIB=static ENABLE_SSPI=yes ENABLE_IPV6=yes ENABLE_IDN=yes ENABLE_WINSSL=yes GEN_PDB=no DEBUG=no RTLIBCFG=static MACHINE=x86 ZLIB_PATH=../../../deps

--]]
