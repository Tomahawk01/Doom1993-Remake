project "Doom"
    kind "ConsoleApp"
    language "C"
    staticruntime "off"

    files
    {
        "**.h",
        "**.c"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "src",
        "%{wks.location}/vendor/glfw/include"
    }

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib"
    }

    links
    {
        "glfw3.lib"
    }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"