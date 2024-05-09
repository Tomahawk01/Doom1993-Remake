workspace "Doom-Remake"
    architecture "x64"
    startproject "Doom"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "vendor/glad/Build-Glad.lua"
group ""

include "Doom/Build_Doom.lua"