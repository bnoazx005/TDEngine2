# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.5.27] 2022-01-25

### Changed

- **CU8String** class is now just a namespace for helper functions to operate with UTF-8 encoded strings. **U8C** alias was
replaced with strongly typed version called **TUtf8CodePoint**.

- Now **CFileInputStream** and **CFileOuptuStream** support UTF-8 paths.

## [0.5.26] 2022-01-22

### Added

- A new versioning was introduced into engine's CMakeLists.txt and Config.h files.

- A git hook for pre-push was added to automatically update the project version.

- API of **IBinaryFileReader** was extended with a bunch of new methods **ReadX** that's endian independent.

- API of **IBinaryFileWriter** was extended with a bunch of new methods **WriteX** that's endian independent.

- A new type of file readers was introduced which is **ICsvFileReader**.

- **CU8String** now supports construction from **wchar_t**. Also **_U8Str** suffix was introduced for the type.

### Changed

- The functions SwapXBytes were replaced with a template SwapBytes. Now the engine assumes that all numeric data is stored in little endian manner.
If it's run on big endian machines everything will be swapped into a little endian memory layout.

- Now **CBinaryArchiveWriter** and **CBinaryArchiveReader** are endian independent.

## [0.5.25] 2022-01-16

### Added

- Now **default-profile.camera_profile** is a part of common resources and can be loaded via _DefaultConfigs/default-profile.camera_profile_ identifier.

- Also all configuration assets are available using the following _DefaultConfigs/_ virtual path.

- **CreateComponentsLocalSlice** was added into API of **IWorld**.

### Changed 

- The implementation of **CFramePostProcessor** was refactored and cleaned up

- The common performance of builtin systems was improved.

- The update of bounds for scene's instances was temporary disabled.

### Fixed

- The performance of builtin systems was improved due to move to cache friendly code in their code.

## [0.5.24] 2021-12-31

### Added

- The API of **ISystemManager** was extended with new public methods **ForEachSystems** that allows to iterate over systems and **IsSystemActive**.

- The API of **ISystem** was extended with new public methods **IsActive**, **OnActivated** and **OnDeactivated**.

- A new editor window was added for changing project's settings on the fly. For now it supports to enable/disable game systems.

### Changed

- Now all components use their own pool allocators.

## [0.5.23] 2021-12-18

### Changed

- **Optick** library is not linked statically.

- Implementation of memory allocators was highly refactored.

- The common implementation of the memory manager as a subsystem was excluded. Now local usage of allocators and
memory arenas is more preffered way.

## [0.5.22] 2021-12-01

### Changed

- The implementation of component factories was totally refactored to simplify creation of new ones.

### Fixed

- Optick is now available only under MSVC compiler.

## [0.5.21] 2021-12-08

### Added

- Now **IWorld** supports unique components which can be assumed as singletons replacement.

- Add implementation of separate editor's camera

### Fixed

- The behaviour within developer's mode was fixed.

- The dereferencing of nullptr within CObjectSelectionSystem was fixed when the developer's menu is invoked too soon after the start up

- The implementation of editor's camera was fixed and refactored

## [0.5.20] 2021-11-30

### Added

- Shadow mapping's configuration was implemented for the forward renderer

### Fixed

- The input was fixed for ImGUI's interfaces.

- (Fixed issue #72) Processing of window's resize was fixed under **CWin32WindowSystem**.

## [0.5.19] 2021-11-23

### Added

- The configuration of texture's filtering type was implemented.

- **Optick** profiler was integrated into the engine's core.

### Changed

- **CFileLogger** was rewritten in asynchronous manner. Now it uses its own thread to write logs.

- **CUIElementsProcessSystem** was optimized. Now elements are updated only if their layout's changed.

## [0.5.18] 2021-11-14

### Changed

- **CTextureAtlas** now uses **CScopedPtr\<T\>** to store references to resources.

## [0.5.17] 2021-11-14

### Added

- The API of **IRenderTarget** was extended with a new public method **Resize(U32, U32)**.

### Fixed

- (Fixed issue #44) The bad memory access exception was fixed for the case a window was resized under D3D11 graphics context.

## [0.5.16] 2021-11-13

### Changed

- **tde2_introspector** utility was updated to a new version.

## [0.5.15] 2021-11-13

### Fixed

- (Fixed issue #65) The work of IDebugUtility::DrawSphere was fixed to correctly position spheres

## [0.5.14] 2021-11-12

### Added

- A new proxy type **CProxyGraphicsContext** was introduced to provide a null graphics context for the console mode.

### Fixed 

- **tde2_mesh_converter** utility's logic was fixed to correctly support work in console mode.

## [0.5.13] 2021-11-11

### Fixed

- The main loop in **CWin32WindowSystem** was updated.
- **CKeyboard** in **WindowsInputContext** now uses non-exclusive acquiring mode.
- The implementation of getting keyboard's input was fixed and improved in **CImGUIContext**.

## [0.5.12] 2021-11-07

### Added

- The growing of vertex/index buffers was added in dear imgui's code

### Fixed

- **CImGUIContext::GradientColorPicker**'s implementation was refactored.

## [0.5.11] 2021-11-06

### Added

- A new method **IsScissorTestEnabled()** was added for **IMaterial** interface

### Changed 

- **tde2_introspector** utility was updated.
- **ImGUIContext** now uses Dear ImGUI of 1.85 version.
- The API of **IGraphicsContext** was changed. The signature of **SetScissorRect** method was changed from (const TRectF32&) onto (const TRectU32&).

### Fixed

- The implemenation of **COGLGraphicsContext::SetScissorRect** was updated to be consistent when GAPI is switched.

## [0.5.10] 2021-11-01

### Changed

- **IDLLManager** moved from **IWindowSystem** into **IPluginManager** ownership

### Added

- A new implementation **CProxyWindowSystem** was added
- Silent mode was added for **CFileLogger**

## [0.5.9] 2021-10-31

### Changed

- **CGameUserSettings::mFlags** was moved into **CProjectSettings** class

## [0.5.8] 2021-10-30

### Changed

- All resources are now wrapped with **CScopedPtr** when returned from the resource manager

### Fixed 

- The loading of default instances was fixed for all types of resources when the specified ones cannot be loaded

## [0.5.7] 2021-10-28

### Changed

- **CResourceManager** is migrated onto CScopedPtr usage.

## [0.5.6] 2021-10-26

### Fixed 

- CD3D11GraphicsContext now doesn't assert in SafeReleaseCOM invocations.

### Changed

- The storage of depth-stencil and rasterizer states was optimized in CD3D11GraphicsObjectsManager.
- CMemoryProfiler's instance is thread-safe now.

## [0.5.5] 2021-10-17

### Fixed 

- A bucnh of Unix related classes were fixed due to last refactoring

## [0.5.4] 2021-10-16

### Changed

- Breaking changes. Now the engine and its toolset is targeted for x64 platforms. x86 isn't supported from this moment.

## [0.5.3] 2021-10-09

### Changed

- Now settings are splitted into application specific and project ones
- The localization manager now reads its settings from **CProjectSettings**

### Added

- Two new types **CGameUserSettings** and **CProjectSettings** were introduced

## [0.5.2]  2021-10-06

### Changed

- The logger now uses the smart pointer to control its lifetime

### Added

- A new type to process input program's console arguments was introduced which is **CProgramOptions**

## [0.5.1]  2021-09-29

### Changed

- **CDeferOperation** class is replaced with **Wrench::TDeferOperation**

- GraphicsContextTypeToString(E_GRAPHICS_CONTEXT_GAPI_TYPE) and StringToGraphicsContextType(const std::string&) were removed

## [0.5.0]  2021-08-02

### Changed

### Added

- An implementation of basic material functionality was introduced

- A pack of utilities were added including tde2_mesh_converter, tde2_resource_packer, TDE2FontEd, TDE2AnimationEd

- Scene manager with support of sync/async scene loading was added

- A few types of textures (2D / Cubemap) were supported

- Off-screen rendering support was also provided via RenderBuffer / DepthBuffer resources

- Basic functionality of save manager was added

- Support of UI system with flexible layout was implemented

- Particles with their own editor TDE2ParticlesEd

- 2D/3D physics

- Audio system based on FMOD library

- Post-processing camera stack

- Localization manager with corresponding resource to store locales data

- first iteration of 3D animation system

- Support of skinned meshes

### Fixed

## [0.4.22] - 2020-10-08

### Changed

- The internal type CResult<T, E> was replaced with Wrench::Result<T, E>

- CStringUtils API's was partially replaced with Wrench::StringUtils

## [0.4.0] - 2019-12-24

### Added

- An implementation of basic materials system with support

- Support of static 3d meshes

- Integration of box2d as 2D physics engine

- Font rendering system was added into the engine for debug purposes in this build

### Changed

- Implementation of ECS system was updated

- Now user can specify interfaces instead of particular classes for all methods of **IFileSystem**

## [0.3.63] - 2019-10-07

### Added

- Support of cubemaps for both D3D11 and OGL3.x graphics contexts

- Implement blending states for materials

- An asynchronous I/O operations were introduced for text and binary file readers / writers

- Now the engine supports reading and writing from/into YAML files

- Texture atlases support was introduced

- A new debug infrastructure, that's represented with IDebugUtility, was added

### Fixed

- Fixed existed memory leaks (issue #32)

- **CConfigFileReader**'s implementation was fixed (issue #23)

### Changed

- Now instead of concrete classes you should use their interfaces when work with the file system methods

# Template 

## [MAJOR.MINOR.BUILD] - YYYY-MM-DD

### Added
### Fixed
### Changed