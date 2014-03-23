-- some quick helper functions to reduce typing :)
function prepend(s, t)
    for k, v in pairs(t) do t[k] = s .. v end
    return t
end

function cf_global()
    includedirs { "Libs", "external" }
end

function cf_global_Libs()
    includedirs { ".", "../Libs", "../external" }
end

-- where teh solution files are output to
BUILD_PATH = "build/" .. (_ACTION or "")

-- The GUT!
solution "Katana"
    configurations { "Debug", "Release" }
    location(BUILD_PATH)

    -- Global settings.
	-- various platform-specific build flags
	if os.is("windows") then
		defines { "_CRT_SECURE_NO_DEPRECATE", "_CRT_NONSTDC_NO_DEPRECATE", "WIN32" }
		flags { "StaticRuntime", "No64BitChecks" }
	else
		-- *nix
		buildoptions {
			"-Wunused-parameter",
			"-Wredundant-decls",	-- (useful for finding some multiply-included header files)
			"-Wundef",				-- (useful for finding macro name typos)

			-- enable security features (stack checking etc) that shouldn't have
			-- a significant effect on performance and can catch bugs
			"-fstack-protector-all",

			-- always enable strict aliasing (useful in debug builds because of the warnings)
			"-fstrict-aliasing",

			-- do something (?) so that ccache can handle compilation with PCH enabled
			"-fpch-preprocess",

			-- enable SSE intrinsics
			"-msse",

			-- don't omit frame pointers (for now), because performance will be impacted
			-- negatively by the way this breaks profilers more than it will be impacted
			-- positively by the optimisation
			"-fno-omit-frame-pointer" 
		}

		if os.is("linux") then
			defines { "LINUX" }
			linkoptions { "-Wl,--no-undefined", "-Wl,--as-needed" }
		end

		-- To support intrinsics like __sync_bool_compare_and_swap on x86
		-- we need to set -march to something that supports them
		if arch == "x86" then
			buildoptions { "-march=i686" }
		end
	end

    configuration "Debug"
        flags { "Symbols" }
        defines { "DEBUG" }
        targetdir(BUILD_PATH .. "/../..")
		targetname ("katana_debug")

    configuration "Release"
        flags { "OptimizeSpeed", "NoEditAndContinue" }
        defines { "NDEBUG" }
        targetdir(BUILD_PATH .. "/../..")
		targetname ("katana")


--[[
################################################
			Build Engine libraries
################################################
--]]


project "Menu"
    kind "SharedLib"
    language "C"
    cf_global_Libs()
    files { "menu/*.c" }


project "Platform"
    kind "StaticLib"
    language "C"
    cf_global_Libs()
    files { "platform/*.c" }



--[[
###############################################
			Build the Game
###############################################
--]]	



project "OpenKatana"
    kind "SharedLib"
    language "C"
    cf_global_Libs()
	includedirs { "game" }
	defines { "OPENKATANA", "GAME_MODULE" }
    files { "game/client/*.c", "game/server/OK/*.c", "game/server/*.c", "game/shared/*.c", }



--[[
###############################################
			Build the Engine
###############################################
--]]

project "Launcher"
	kind "WindowedApp"
	language "C++"
	files { "launcher/*.cpp", "launcher/*.h" }

project "Engine"
    kind "SharedLib"
    language "C"
    cf_global()
    files { "*.c", "*h", "engine/*.c","engine/*.h", "shared/*.c" }
    links { "ogg", "vorbis", "vorbisfile", "Menu", "Platform", "Game" }

	-- Platform Specifics
	if os.is("windows") then
		links { "ws2_32", "winmm", "openal32", "sdl2", "sdl2main" }
		linkoptions { "/NODEFAULTLIB:msvcrt.lib" }
	elseif os.is("linux") then
		links {	"m", "dl", "pthread" }
	elseif os.is("macosx") then
		links { "pthread" }
		buildoptions { "-mmacosx-version-min=10.5" }
		linkoptions { "-lstdc++-static", "-mmacosx-version-min=10.5" }	
	end


-- The Clean "Action" (tm)
if _ACTION == "clean" then
    for a in premake.action.each() do   -- action trigger is "vs2008", "gmake", etc.
        dir = "build/" .. a.trigger
        if os.isdir(dir) then
            local result, msg=os.rmdir(dir)
            print("Removing directory " .. dir .. "...")
        end
    end
end
