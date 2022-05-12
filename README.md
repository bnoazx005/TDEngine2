![TDEngine2](https://i.imgur.com/Q8QlaxD.png)

TDEngine 2 is a cross-platform game engine.

Build status (for all platforms): [![TDEngine2 SDK Build](https://github.com/bnoazx005/TDEngine2/actions/workflows/main.yml/badge.svg)](https://github.com/bnoazx005/TDEngine2/actions/workflows/main.yml)


## Table of contents

1. ### [Current Goals](#current-goals)
2. ### [Current Features](#current-features)
3. ### [How to Build](#how-to-build)

    ### 3.1 [Visual Studio (Win64)](#vs-win64)

    ### 3.2 [Make Utility (UNIX)](#make-unix)
 
4. ### [Dependencies](#dependencies)

5. ### [Getting Started](#getting-started)

***

### Current Goals:<a name="current-goals"></a>

* Android OS support;

***

### Current Features:<a name="current-features"></a>

* Entity-Component-System architecture with support of events, reactive systems and etc;

* Windows (7 and higher), UNIX systems support;

* Support of D3D11 and OGL 3.x GAPIs;

* 2D sprites rendering with instancing support;

* 3D static and skinned meshes rendering with different configurable materials;

* Support of particle effects, animations with custom editors;

* Scene management with support of asynchronous loading of scenes and assets;

* Toolchain that includes mesh converter, packages/fonts/atlases/particles editors;

***

### How to Build<a name="how-to-build"></a>

The project actively uses CMake for the building process. So firstly, you should be sure you have 
CMake tools of 3.8 version or higher to continue. At the moment the engine supports Win64 and UNIX 
platforms.

**IMPORTANT NOTE!** After you get compiled binaries (precompiled from "Releases" or did it yourself) you also need to prepare resources. This step could be done with **tde2_mesh_converter** utility.

#### Visual Studio (Win64)<a name="vs-win64"></a>

For now we support Visual Studio 2017 and higher versions.

To build all the projects use scripts that were specially prepared for that tasks. For Windows users there are
two batch scripts **prepare_build_generic_win.bat** and **prepare_build_vs2017_win64.bat**. The first one is common 
pipeline for any version of Visual Studio or a build type.

```console
prepare_build_generic_win.bat "<<Generator name>>" "<<Build Type>>"

:: or use this one to generate debug for VS2017
prepare_build_vs2017_win64.bat

:: or use this one to generate debug for VS2022
prepare_build_vs2022_win64.bat
```

Both arguments for the script is one of allowed by CMake tool. _**\<\<Build Type\>\>**_ could be **Debug** or **Release** values.
After execution all binaries and libraries will be available under /bin/%CONFIGURATION%/ directory.

#### Make utility (UNIX)<a name="make-unix"></a>

**IMPORTANT NOTE!** Starting from 0.5.39 version of the engine FMOD's binaries for UNIX platforms is excluded. This also means that **FMODAudioContext**'s compilation is disabled by default for now. So if you already have installed FMOD binaries you can turn this back on via fixing **prepare_build_generic_unix.sh** file.
```console
# replace this argument
-DBUILD_FMOD_AUDIO_CTX_PLUGIN=OFF
# with this one
-DBUILD_FMOD_AUDIO_CTX_PLUGIN=ON
```

To install FMOD libraries you can follow instructions in great article for Debian OS https://wiki.debian.org/FMOD. For Ubuntu it works the same but be sure you copy correct versions of the binaries.

Under UNIX platform you have an option to build the project using make utility. As described above you
need firstly generate Makefile with CMake. The process looks the same, but another CMake's generator
is used:
```console
$ mkdir build
$ cd build
$ cmake .. -G "Unix Makefiles"
```

Another approach to build the engine is to use shell script **prepare_build_generic_unix.sh** from **build/** directory. 

```bash
$ ./prepare_build_generic_unix.sh "<<Build Type>>"
```

_**\<\<Build Type\>\>**_ argument can equal either to **Debug** or **Release**.

Note. There are a few requirements for successfull compilation of the engine under UNIX. The first is
a support of GLX 1.4. It could be done if you have proprietary video drivers installed on your system.
So your video card should support at least OpenGL 3.0. The second is installed X11 and GLEW libraries.

***

### Dependencies<a name="dependencies"></a>

* Argparse (https://github.com/cofyc/argparse)

* Backward-cpp (https://github.com/bombela/backward-cpp)

* Box2D (https://github.com/erincatto)

* Bullet3 (https://github.com/bulletphysics/bullet3)

* Debugbreak (https://github.com/scottt/debugbreak)

* GLEW (http://glew.sourceforge.net/)

* Dear ImGUI (https://github.com/ocornut/imgui)

* Optick (https://github.com/bombomby/optick)

* Rapidcsv (https://github.com/d99kris/rapidcsv)

* STB libraries (https://github.com/nothings/stb)

* tcpp (https://github.com/bnoazx005/tcpp)

* Wrench (https://github.com/bnoazx005/wrench)

* mini-yaml (https://github.com/jimmiebergmann/mini-yaml)

* zlib (https://zlib.net/)

* Assimp (used only by tde2_mesh_converter utility) (https://github.com/assimp/assimp)

***

### Getting Started<a name="getting-started"></a>

The simplest application that creates an empty window consists of the following lines of code:
```cpp
#include <TDEngine2.h>

using namespace TDEngine2;

int main(int argc, char** argv)
{
    E_RESULT_CODE result = RC_OK;
    
    auto pEngineCoreBuilder = TPtr<IEngineCoreBuilder>(CreateConfigFileEngineCoreBuilder({ CreateEngineCore, "Default.project" }, result));

    if (result != RC_OK)
    {
        return -1;
    }

    if (auto pEngineCore = TPtr<IEngineCore>(pEngineCoreBuilder->GetEngineCore()))
    {
        pEngineCore->RegisterListener(std::make_unique<CCustomEngineListener>());
        pEngineCore->Run();
    } 

    return 0;
}
```
You can see that there is "settings.cfg" file used in the line where IEngineCoreBuilder* instance is retrieved. It's a configuration file that should have at least information about window's sizes, its title and graphics API that should be used. You can find the one inside **/Samples/TemplateApp/settings.zip** archive. In short it's just an INI file with all the syntax that corresponds to its notation.