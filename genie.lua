solution "nes_emulator"
   configurations { "Debug", "Release" }
   location "build"
   targetdir "bin"
   debugdir "./runtime/"
   platforms { "x64" }


   -- nes_emulator
   -- ============
   project "nes_emulator"
      language "C++"
      files { "src/*.cpp" }
      includedirs { "include/" }

      buildoptions_cpp {
          "-std=c++11",
      }

      if os.get() == "windows" then

      elseif (os.get() == "macosx") then
         buildoptions { "-Wall",
                        "-Werror",
                        "-fcolor-diagnostics"
                      }
         links
         {
           "Cocoa.framework",
           "QuartzCore.framework",
           "Metal.framework",
           "MetalKit.framework",
         }
      end

      configuration "Debug"
         kind "ConsoleApp"
         defines { "DEBUG" }
         flags { "Symbols", "StaticRuntime" }

      configuration "Release"
         kind "WindowedApp"
         defines { "NDEBUG" }
         flags { "Optimize", "StaticRuntime" }
         -- flags { "OptimizeSize", "StaticRuntime" }
