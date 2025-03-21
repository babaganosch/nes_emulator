solution "nesscape"
   configurations { "Debug", "Release" }
   location "build"
   targetdir "bin"
   debugdir "./runtime/"
   platforms { "x64" }


   -- nesscape
   -- ============
   project "nesscape"
      language "C++"
      files { "src/**.cpp" }
      includedirs { 
        "include/", 
        "thirdparty/minifb/include/", 
        "thirdparty/miniaudio/", 
        "thirdparty/rapidjson/include/" 
      }

      links { "minifb_internal" }

      buildoptions_cpp {
          "-std=c++11",
      }

      if os.get() == "windows" then
         buildoptions { "-Wall",
                        "-Werror", 
                      }
         links {
            "Opengl32",
            "winmm",
            "gdi32",
         }
      elseif (os.get() == "macosx") then
         buildoptions { "-Wall",
                        "-Werror",
                        "-fcolor-diagnostics"
                      }
         links {
           "Cocoa.framework",
           "QuartzCore.framework",
           "Metal.framework",
           "MetalKit.framework",
         }
      else -- linux
         buildoptions { "-Wall",
                       "-Werror",
                     }
         links {
           "X11",
           "dl",
           "pthread",
         }
      end

      configuration "Debug"
         kind "ConsoleApp"
         defines { "DEBUG", "WARNINGS" }
         flags { "Symbols", "StaticRuntime" }

      configuration "Release"
         kind "WindowedApp"
         defines { "NDEBUG" }
         flags { "Optimize", "StaticRuntime" }
         -- flags { "OptimizeSize", "StaticRuntime" }
   
   -- minifb lib
   -- ===========
   project "minifb_internal"
      targetname ("minifb_internal")
      kind "StaticLib"
      language "C++"

      buildoptions_cpp {
          "-std=c++11",
      }

      includedirs { "thirdparty/minifb/include/", "thirdparty/minifb/src/" }

      files {
        "thirdparty/minifb/src/MiniFB_common.c",
        "thirdparty/minifb/src/MiniFB_cpp.cpp",
        "thirdparty/minifb/src/MiniFB_internal.c",
        "thirdparty/minifb/src/MiniFB_internal.h",
        "thirdparty/minifb/src/MiniFB_timer.c",
        "thirdparty/minifb/src/WindowData.h",
      }

      if os.get() == "windows" then
         defines { "USE_WINDOWS_API" }
         files {
            "thirdparty/minifb/src/windows/WinMiniFB.c",
            "thirdparty/minifb/src/windows/WindowData_Win.h",
         }
         includedirs { "thirdparty/minifb/src/windows/" }
         links {
            "Opengl32",
            "winmm",
            "gdi32",
         }
      elseif (os.get() == "macosx") then
        defines { "USE_METAL_API" }
        files {
          "thirdparty/minifb/src/macosx/MacMiniFB.m",
          "thirdparty/minifb/src/macosx/OSXWindow.m",
          "thirdparty/minifb/src/macosx/OSXView.m",
          "thirdparty/minifb/src/macosx/OSXViewDelegate.m",
          "thirdparty/minifb/src/macosx/WindowData_OSX.h",
        }

        includedirs { "thirdparty/minifb/macosx/" }

        links {
          "Cocoa.framework",
          "QuartzCore.framework",
          "Metal.framework",
          "MetalKit.framework",
        }
      else -- linux
        defines { "USE_X11_API" }
        files {
          "thirdparty/minifb/src/gl/MiniFB_GL.c",
          "thirdparty/minifb/src/x11/X11MiniFB.c",
          "thirdparty/minifb/src/MiniFB_linux.c",
        }
        includedirs { "thirdparty/minifb/src/linux/" }
        links {
          "X11",
        }
      end

      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols", "StaticRuntime" }

      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize", "StaticRuntime" }
