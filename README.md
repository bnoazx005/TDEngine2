# TDEngine 2

[![Build status](https://ci.appveyor.com/api/projects/status/dyp34r05yfxii09m?svg=true)](https://ci.appveyor.com/project/bnoazx005/tdengine2)

TDEngine 2 is a cross-platform game engine.

### Current goals:

* Entity-Component-System architecture;

* Windows (7 and higher), UNIX systems and Android OS support;

* 2D and 3D graphics support via Direct3D 11 and OpenGL 3.3 (with extensions) GAPIs;

* Effective memory management (in future);

* Modularity and plugins support;

* Toolset (including resource editor, scene editor, exporters and etc) (in future).

### How to Build

The project actively uses CMake for the building process. So firstly, you should be sure you have 
CMake tools of 2.8 version or higher to continue. By now the project  was built using Visual 
Studio 2017, but it will be successfully built with 2015 version as well (in plans to include 
a support of g++ compiler). 

To generate the basic Visual Studio's solution of the engine's SDK you should run the following 
command in a console:
cmake -G "Visual Studio 201X"
where X is 5 or 7.

If everything went ok you've got a solution of the entire SDK (including sample project, 
which is called SandboxGame). For now you can compile it either using Visual Studio IDE,
or a console's command:
cmake --build .
All executables can be found within /bin directory. Note. To speed up building process you can
exclude a part of built-in plugins, which 
aren't needed you. 