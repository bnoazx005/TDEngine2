![TDEngine2](https://i.imgur.com/Q8QlaxD.png)

TDEngine 2 is a cross-platform game engine.

Build status (for all platforms): [![TDEngine2 SDK Build](https://github.com/bnoazx005/TDEngine2/actions/workflows/main.yml/badge.svg)](https://github.com/bnoazx005/TDEngine2/actions/workflows/main.yml)


## Table of contents

1. ### [Current Goals](#current-goals)
2. ### [Current Features](#current-features)
3. ### [How to Build](#how-to-build)

    ### 3.1 [Visual Studio (Win32)](#vs-win32)

    ### 3.2 [Make Utility (UNIX)](#make-unix)
    
4. ### [Getting Started](#getting-started)

***

### Current Goals:<a name="current-goals"></a>

* Android OS support;

* Effective memory management (in future);

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
CMake tools of 2.8 version or higher to continue. At the moment the engine supports Win32 and UNIX 
platforms.

#### Visual Studio (Win32)<a name="vs-win32"></a>

For now we support Visual Studio 2017 and higher versions.

To build all the projects use scripts that were specially prepared for that tasks. For Windows users there are
two batch scripts **prepare_build_generic_win.bat** and **prepare_build_vs2017_win32.bat**. The first one is common 
pipeline for any version of Visual Studio or a build type.

```console
prepare_build_generic_win.bat "<<Generator name>>" "<<Build Type>>"
:: or use this one to generate debug for VS2017
prepare_build_vs2017_win32.bat
```

Both arguments for the script is one of allowed by CMake tool. _**\<\<Build Type\>\>**_ could be **Debug** or **Release** values.
After execution all binaries and libraries will be available under /bin/%CONFIGURATION%/ directory.

#### Make utility (UNIX)<a name="make-unix"></a>

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

### Getting Started<a name="getting-started"></a>

The simplest application that creates an empty window consists of the following lines of code:
```cpp
#include <TDEngine2.h>

#if defined (TDE2_USE_WIN32PLATFORM)
    #pragma comment(lib, "TDEngine2.lib")
#endif

using namespace TDEngine2;


int main(int argc, char** argv)
{
    E_RESULT_CODE result = RC_OK;
    
    IEngineCoreBuilder* pEngineCoreBuilder = CreateConfigFileEngineCoreBuilder(CreateEngineCore, "settings.cfg", result);

    if (result != RC_OK)
    {
        return -1;
    }

    IEngineCore* pEngineCore = pEngineCoreBuilder->GetEngineCore();

    pEngineCoreBuilder->Free();
    
    pEngineCore->Run();
    
    pEngineCore->Free();

    return 0;
}
```
You can see that there is "settings.cfg" file used in the line where IEngineCoreBuilder* instance is retrieved. It's a configuration file that should have at least information about window's sizes, its title and graphics API that should be used. You can find the one inside **/Samples/TemplateApp/settings.zip** archive. In short it's just an INI file with all the syntax that corresponds to its notation.