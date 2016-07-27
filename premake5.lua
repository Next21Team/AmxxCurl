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

  flags { "No64BitChecks", "StaticRuntime", "MultiProcessorCompile", "C++14" }
  characterset("MBCS") -- use multibyte character set in msvs

  -- flags & options --

  filter "configurations:DebugDLL"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"

  filter "configurations:ReleaseDLL"
    defines { "NDEBUG" }
    optimize "Full"
    flags { "NoBufferSecurityCheck", "NoRuntimeChecks" }

  filter { "system:windows", "configurations:ReleaseDLL" }
    flags { "NoIncrementalLink", "LinkTimeOptimization" }

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

  links { "zlib-lib", "curl-lib" }
  includedirs { "contrib/zlib", "contrib/curl/include" }

  files { "src/**.h", "src/**.cc", "src/sdk/**.cpp" }
  excludes { "contrib/**.*" }

  --
  filter "system:windows"
    links { "Ws2_32" }

  filter "system:linux"
    toolset "gcc"
    linkgroups "On"
    buildoptions { "-fpermissive" } -- and therefore clang not supported
    linkoptions { "-static-libstdc++" }

-- 3rd party
group "contrib"
  include   "contrib/zlib"
  include   "contrib/curl"
