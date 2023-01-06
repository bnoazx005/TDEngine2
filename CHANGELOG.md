# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.6.14] 2023-01-03

### Added

### Changed

- The API of **IComponentManager** was changed. RemoveComponentImmediately and RemoveComponentsImmediately were removed from the class.

### Fixed

- The logic of **IWorld::CreateLocalComponentsSlice** was fixed to support hierarchies of entities.

## [0.6.13] 2022-12-31

### Added

- New interfaces were defined which are **IBoxCollisionObject3D**, **ISphereCollisionObject3D**, **IConvexHullCollisionObject3D**, **ICapsuleCollisionObject3D**.

- New overloadings of HasComponent method were implemented for **CEntity**, **CEntityManager** and **CComponentManager**.

- A new dependency library which is **Tina** was added.

- Two template classes **CBaseResourceLoader<T>** and **CBaseResourceFactory<T>** were introduced to simplify declaration of a new simple resources types.

- Now game entities have activation/deactivation ability to exclude them from a processing step.

### Changed

- **Wrench** library's version was updated

- The Catch2 was updated to 3rd version.

- The current implementation of the job manager was refactored to support better synchronization mechanisms and tasks' batches.

- The internal implementation of **CSceneManager** was refactored to fix loading issues.

- The APIs of **CEntityManager** and **IWorld** were changed. DestroyAllImediately, DestroyImmediately methods were removed.

- The signature of **IWorld::Destroy** was changed.

### Fixed

- The serialization/deserialization logic for prefabs links was refactored and fixed.

- TDE2_REGISTER_EVENT macro was updated for other namespaces usage.

- The crases caused with CFont's creation with empty font's settings were fixed.

- The incorrect loading/saving of scene's prefabs was fixed. 

- (Fixed issue #92) Duplicates of states that cause asserts were fixed.

## [0.6.12] 2022-11-02

### Added

- Now prefabs can be linked to any entity within scene's hierarchy.

- The functionality for saving hierarchies into prefabs was implemented.

- A new concept of runtime-only components that are not serialized was introduced.

- Now editor windows can be drawn in overlayed mode.

- A new editor window which is a statistics overlay window was added. 

- A new function ContainsAABB for **TAABB** type was defined.

- A new helper function Reflect was added for **TVector2**, **TVector3** math types.

- The API of **ISystem** was extended with two new public methods **AddDefferedCommand**, **ExecuteDefferedCommands** and **DebugOutput**. Now everything that should be executed after all updates
should be wrapped up with a deffered action's call.

- Prefabs linkage now supports overriding of a original position.

- Add 3d physics's settings into the project settings class.

- A new type of 3d physics collider **CCapsuleCollisionObject3D** was added.

### Changed

- 3D physics implementation via Bullet physics engine was moved into a separate plugin.

- The API of **IECSPlugin** was changed OnRegister methods was splitted into OnRegisterComponents and OnRegisterSystems.

- The inspectors for 3d physics components were updated.

### Fixed

- The implementation of transform's manipulators works more robust way.

- The incomplete material's initialization was fixed.

- The incorrect layout for multiple prefab items in "Load prefab window" was fixed.

- The serialization of prefabs collection resources was fixed.

- The implementation of auto-tests' registration system was fixed.

- The prefabs registry's implementation was fixed to support spawning entities with non-built in component types. 

- Intersection of kinematic/static 3d physics objects was fixed.

- Remove duplicated 3d collision events for static/kinematic objects.

- The serialization/deserialization of **CTransform** component was fixed. Now local coordinates are stored instead of global ones.

## [0.6.11] 2022-09-24

### Added

- The implementation of simple auto-testing system was introduced.

- A new project TDE2TestsRunner which is an environment for engine's autotests was added.

- A new type of file's writer was added which is **CTextFileWriter** type.

- A new proxy type for the input context was introduced.

- A new overloaded version of IImageFileWriter::Write was added that allows to write raw array of bytes into the image file.

- A new public method GetBackBufferData() was added into **IGraphicsContext**.

### Changed

- **TComponentTypeNameTrait** meta-function was removed and replaced with BaseComponent::GetClassId() static method which has same functionality.

- The latter statement infers another change which is the macro **TDE2_DECLARE_FLAG_COMPONENT** can be used within any namespace.

- Now the following macros **TDE2_TYPE_ID, TDE2_REGISTER_TYPE, TDE2_REGISTER_COMPONENT_TYPE, TDE2_SYSTEM** can be used within any namespace.

### Fixed

- Memory leaks caused by **IShaderCompiler** and **IStructuredBuffer** instances were fixed.

- The incorrect work of **CTransformSystem** was fixed.

- The incorrect computations of transforms that are changed via inspector was fixed.

- The incorrect work of **CImGUIContext::DrawGizmos** was fixed.

## [0.6.10] 2022-08-17

### Added

- Now the engine provides an implementation of application's entry point which is stay same for most projects

- A helper script for generation of new projects was added.

- A new type of a subsystem which provides a way to switch between game modes was introduced.

- Now Image component supports changing of its color.

- The configuration of the splash screen was added.

- The inspectors for all camera types were implemented.

### Changed

### Fixed

- (Fixed issue #86) Now a new added StaticMeshComponent or SkinnedMeshComponent don't make an application crash.

- (Fixed issue #88) Deserializing of StaticMeshContainer's state was fixed.

- (Fixed issue #87) Now a user can specify and load any scene as persistent without any problem. If the given one isn't exist a new default is created instead.

- A crash caused by ObjectSelectionSystem was fixed when a user tried to enter a material's name for a new created StaticMeshContainer component.

- (Fixed issue #89) Incorrect processing of multiple canvases in a scene was fixed.

## [0.6.9] 2022-07-30

### Added

- The API of **IFileSystem** was extended with a new public method GetParentPath.

- Now textures could be configured during development process and loaded without any code with defined parameters that come from the runtime manifest.

- A new type of a shader was introduced which is a compute shader.

- The API of **IGraphicsContext** was extended with a new public method DispatchCompute.

- Add support of usage of structured buffers as shaders resources.

### Changed

- A base resources path that was a hardcoded value now is stored in project settings file.

- The signatures of CD3D11Buffer::Init and COGLBuffer::Init was changed. Now it takes a structure of parameters of **TBufferInitParams** type.

### Fixed

- Warnings caused with render targets simultaneously used as SRV/RTV in D3D11 mode was fixed.

## [0.6.8] 2022-07-15

### Added

-  The basic draft implementation of a resources build manifest was added.

- A new utility which is **TDE2ResourcesEd** was added.

- A new interface **IResourcesRuntimeManifest** and an implementation **CResourcesRuntimeManifest** were added into the project.

### Changed

- **tde2_mesh_converter** now supports batched conversion based on resources manifest's information.

- **MountDirectories** function in CBaseEngineCoreBuilder.cpp was splitted into two stages.

### Fixed

- Horizontal layout alignment was fixed for images and selectable items.

## [0.6.7] 2022-06-17

### Added

- **IMaterial** now is cloneable entity.

- Add new functions **Serialize**, **Deserialize** with overloadings for mostly used engine types. 

- The serialization/deserialization of basic shader variables was implemented for **CBaseMaterial** type.

- The draft implementation of **TDEMaterialEd** editor was added.

- **IFileSystem::ResolveVirtualPath**'s got third parameter to retrieve absolute path.

- A new utility for materials editing was added which is called **TDE2MaterialEd**.

### Changed 

- Now project settings stores real paths to audio/renderer plugins instead of predefined set of values.

- A new version of **tde2_introspector** was added also a tagged-mode enabled to minimize library size.

### Fixed

- Now **IWindowSystem::OpenFileDialog/SaveFileDialog** don't change current working directory.

- The implementation of **CEntity::RemoveComponents** was fixed due to prevent deletion of CTransform components

- (Fixed issue #84) Now the shadow map is cleared even if there are no shadow casters in a scene.

- An incorrect offsets computation for material variables was fixed. 

## [0.6.6] 2022-06-02

### Added

- Drag & drop functionality was added for scene hierarchy's window.

- The API of **ISelectionManager** was extended with new public methods AddSelectionEntity, ClearSelection, GetSelectedEntities and IsEntityBeingSelected.

- The functionality of multi-selection of nodes in the hierarchy's window was implemented.

- Gizmos manipulations for multiple selected entities was added.

### Changed

- The computation of scene's boundaries was removed from **CBoundsUpdatingSystem**. 

### Fixed

- **CImGUIContext::GetDragAndDropData**'s implementation was fixed to return correct results.

## [0.6.5] 2022-05-28

### Added

- A new plugin **TDE2LevelStreamingUtils** was added to provide utilities (components/system) for basic level streaming based on bounding volumes.

- A new functionality of loading user defined plugins that are defined in project settings was implemented.

- A new system **CSceneChunksLoadingSystem** was added which implements logic of levels streaming.

- A new component which is **CSceneLoadingTriggerComponent** was added.

- The API of **IWorld** was extended with new methods RegisterComponentFactory and UnregisterComponentFactory.

- The API of **IEditorsManager** was extended with a new public method RegisterComponentInspector.

### Fixed

- **ContainsPoint** function was fixed.

- The implementation of **CSceneManager::LoadSceneAsync** was fixed for the case when there is no actual scene's data at given filepath.

## [0.6.4] 2022-05-25

### Added

- Now a list of all registered components identifiers can be retrieved using **IWorld::GetRegisteredComponentsIdentifiers**.

- Functionality of adding new components was implemented.

- "Save scene chunk" context menu item was added for scene hierarchy's window.

### Changed

- The existing implementation of components inspectors was refactored. Now they could be removed from their entities.

### Fixed

- The selection of entities in the hierarchy window was fixed.

- Open/Save file dialog's filters were added for Windows/UNIX platform.

## [0.6.3] 2022-05-22

### Added

- Add support of prefab's links within scene's assets.

- Add a new component **CPrefabLinkInfoComponent** to store in-editor information about instantiated prefabs. 

- A context menu for hierachy's window was added which for now supports deletion of entities.

- Functionality of creating new entities/loading prefabs was added into the hierarchy's window.

- Linkage of prefab's instances into scenes was added.

### Fixed

- Incorrect destruction of **CPrefabsRegistry**'s instance was fixed. 

## [0.6.2] 2022-05-18

### Added

- A new type of a resource was added which is **CPrefabsManifest**.

- Minimalistic implementation of prefabs' instantiation logic was introduced.

- Two new interfaces **ICloneable** and **INonAllocCloneable** were added to support deep copying of objects.

- The API of IScene was extended with a new public method Spawn.

### Changed

- Remove redundant default and project file paths mountings.

### Fixed

## [0.6.1] 2022-05-12

### Added

- Mounting of user's defined directories via **CProjectSettings** was added into the engine.

### Changed

- "Arial" font was replaced with "OpenSans" one.

- Now release builds will be deployed only in the main branch.

## [0.6.0] 2022-05-10

### Added

- The implementation of **CUnixInputContext::GetNormalizedMousePosition** was added.

- Processing of **TOnCharInputEvent** was added for **CUnixWindowSystem**.

- **CUnixFileSystem::GetUserDirectory** and **CUnixFileSystem::GetApplicationDataPath** have got their implementations.

- **CUnixWindowSystem::GetClientRect**'s got implementation.

- Default resources for SandboxGame project.

- The implementation of **CUnixWindowSystem::SaveFileDialog** and **CUnixWindowSystem::OpenFileDialog**.

- **CUnixInputContext::SetOnCharInputCallback**'s got implementation.

- **CParticleEmitter** component now supports a non-looped playback mode.

- The inspector for **CParticleEmitter** was implemented.

- Implementation of **tde2_mesh_converter** utility was extended to support LODs for meshes.

- A new type of component which is **CLODStrategy** was added. Also corresponding system **CLODMeshSwitchSystem** was defined to control LODs.

- The implementation of basic parallax mapping effect was added into **TDEngine2Lighting.inc** shader include file.

- Support of **sub-mesh-id** property was added into **CStaticMeshContainer** component.

- The rendering of sub-meshes for static and skinned meshes was implemented.

- Support of capturing entity's state was added for all existing type of tracks in TDEAnimationEd. 

- Add support of sampling modes' switching for animation tracks via its context menu. 

- Add support of switching between sampling modes in curve's editor.

- An experimental implementation of a cubic interpolation mode was added into animation tracks.

- The API of **TColor<T>** was extended with new operators overloadings.

- Serialization/Deserialization of **GridGroupLayout** component was added.

- Add support of scaling curves in track sheets' editor mode of the animation editor utility.

- A pallete of editing channels for TVector2/3 curves was added for AnimationEd utility

- A new component which is **GridGroupLayout** was added.

- The support of selection of UI elements in the developer mode was added. 

- The API of **IFont** was extended with a new public method GetDataVersionNumber.

- A new component which is **9SliceImage** was added into the engine.

- Load/Save methods of **C9SliceImage**, **CLayoutElement**, **CLabel**, **CCanvas**, **CImage**, **CInputReceiver** got their implementations.
- Load/Save methods of **CAnimationContainerComponent**, **CMeshAnimatorComponent** got their implementations.

- The identifier of a default skybox's material now can be configured from the project settings.

- A new library **stb_rect_pack** was added into the dependencies directory.

- Add **zlib** into dependencies directory. Also build scripts were updated due to the new library.

- Now **IPackageFileWriter** and **IPackageFileReader** supports compressed files within packages. All conversions are done implicitly.

- A new versioning was introduced into engine's CMakeLists.txt and Config.h files.

- A git hook for pre-push was added to automatically update the project version.

- API of **IBinaryFileReader** was extended with a bunch of new methods **ReadX** that's endian independent.

- API of **IBinaryFileWriter** was extended with a bunch of new methods **WriteX** that's endian independent.

- A new type of file readers was introduced which is **ICsvFileReader**.

- **CU8String** now supports construction from **wchar_t**. Also **_U8Str** suffix was introduced for the type.

- Now **default-profile.camera_profile** is a part of common resources and can be loaded via _DefaultConfigs/default-profile.camera_profile_ identifier.

- Also all configuration assets are available using the following _DefaultConfigs/_ virtual path.

- **CreateComponentsLocalSlice** was added into API of **IWorld**.

- The API of **ISystemManager** was extended with new public methods **ForEachSystems** that allows to iterate over systems and **IsSystemActive**.

- The API of **ISystem** was extended with new public methods **IsActive**, **OnActivated** and **OnDeactivated**.

- A new editor window was added for changing project's settings on the fly. For now it supports to enable/disable game systems.

- Now **IWorld** supports unique components which can be assumed as singletons replacement.

- Add implementation of separate editor's camera

- Shadow mapping's configuration was implemented for the forward renderer

- The configuration of texture's filtering type was implemented.

- **Optick** profiler was integrated into the engine's core.

- The API of **IRenderTarget** was extended with a new public method **Resize(U32, U32)**.

- A new proxy type **CProxyGraphicsContext** was introduced to provide a null graphics context for the console mode.

- The growing of vertex/index buffers was added in dear imgui's code

- A new method **IsScissorTestEnabled()** was added for **IMaterial** interface

- A new implementation **CProxyWindowSystem** was added

- Silent mode was added for **CFileLogger**

- Two new types **CGameUserSettings** and **CProjectSettings** were introduced

- A new type to process input program's console arguments was introduced which is **CProgramOptions**

### Fixed 

- (Fixed issue #82) The incorrect mapping of mouse buttons under UNIX was fixed for **CUnixInputContext**.

- The implementation of **CPhysics2DSystem::InjectBindings** was fixed.

- Segmentation fault caused by **IRaycastContext::Raycast2DClosest** was fixed.

- Utilities configurations were fixed.

- Invalid deletion of GL shader program's handle was fixed.

- (Fixed issue #83) Incorrect initialization of textures storage in **COGLShader** class was fixed.

- **FMOD** libraries for UNIX were removed from ~FMODAudioContext~ plugin's directory. **FMODAudioContext** plugin's compilation is disabled now for UNIX platforms.
You should build it manually with installed FMOD libraries earlier.

- An invalid order of timer's tick was fixed for **CUnixTimer**.

- The invalid implementation of **CUnixWindowSystem::GetClientRect** was fixed.

- Incorrect processing of window resizing was fixed for GLGraphicsContext.

- Some fixes were introduced for GLSL built-in shaders.

- Fix issues in CMakeLists.txt of **TDE2ResourcePacker** utility.

- Update **install.sh** file. Add zlib's dependencies installation command.

- Fix compilation under UNIX with gcc toolchain.

- **DefaultShader.shader**'s TBN matrix computation was fixed. 

- A typo in meshConverter.cpp was fixed. Indices count was written instead of faces' amount.

- (Fixed issue #77) The incorrect invocation of memcpy was fixed in **CImGUIContext::TextField**.

- (Fixed issue #74) Initialization of animation tracks was fixed.

- The incorrect restore of CYAMLArchiveWriter's state was fixed when an array is written.

- The GPU memory access failure in CImGUIContext plugin when buffers are reallocated was fixed. 

- (Fixed issue #75) Rendering of animation curves was fixed.

- (Fixed issue #76) Assertion caused by TDEAnimationEd was fixed.

- The issue when no camera isn't marked as active was fixed. Now if there is no an active game camera the editor's one is used for that role.

- Behaviour of **CImGUIContext::VerticalSeparator** method was fixed.

- Animation curve's points modification was refactored to improve UX.

- The editing of **LayoutElement** anchors and points was fixed.

- The implementation of **CU8String::MoveNext** was fixed.

- **CSkinnedMeshContainer::Save** implementation was fixed to support serialization/deserialization of skeleton's identifier.

- Now renderer's output under both D3D and GL graphics context looks the same and consistent.

- Receiving shadows by skinned meshes was fixed.

- The generation of texture atlas for glyphs in CRuntimeFont was fixed. Now it's constrained with lexicographical order.

- The performance of builtin systems was improved due to move to cache friendly code in their code.

- Optick is now available only under MSVC compiler.

- The behaviour within developer's mode was fixed.

- The dereferencing of nullptr within CObjectSelectionSystem was fixed when the developer's menu is invoked too soon after the start up

- The implementation of editor's camera was fixed and refactored

- The input was fixed for ImGUI's interfaces.

- (Fixed issue #72) Processing of window's resize was fixed under **CWin32WindowSystem**.

- (Fixed issue #44) The bad memory access exception was fixed for the case a window was resized under D3D11 graphics context.

- (Fixed issue #65) The work of IDebugUtility::DrawSphere was fixed to correctly position spheres

- **tde2_mesh_converter** utility's logic was fixed to correctly support work in console mode.

- The main loop in **CWin32WindowSystem** was updated.

- **CKeyboard** in **WindowsInputContext** now uses non-exclusive acquiring mode.

- The implementation of getting keyboard's input was fixed and improved in **CImGUIContext**.

- **CImGUIContext::GradientColorPicker**'s implementation was refactored.

- The implemenation of **COGLGraphicsContext::SetScissorRect** was updated to be consistent when GAPI is switched.

- The loading of default instances was fixed for all types of resources when the specified ones cannot be loaded

- CD3D11GraphicsContext now doesn't assert in SafeReleaseCOM invocations.

- A bunch of Unix related classes were fixed due to last refactoring

### Changed

- Now LUT texture for color grading post-effect is loaded into the shader even if it's not used. This was done to fix some graphical issues under GL graphics context.

- A new config macro was introduced which is **TDE2_BUILTIN_PERF_PROFILER_ENABLED**. **CPerfProfiler** is now disabled by default.

- **AssertImpl** was reimplemented to support messages output.

- (Fixed issue #81) Now all applications can use default parameters if there is no configuration file at working directory.

- **DrawGrid** function was moved into common editor utilities which is **EditorUtils.h**.

- A new iteration of binary mesh format was introduced.

- Current implementation of **CBinaryMeshFileReader** was reimplemented to support newer version of meshes.

- Now **CAnimationCurve**'s track uses tangents instead of control points to change its curvature.

- API of **IArchiveReader** was updated to support default values for getters.

- **ResolveBinding** was exposed to be a part of public API.

- **Wrench** extra submodule was removed from dependencies of the engine's toolset. 

- **TexturePacker**'s argparse dependency was replaced with internal **CProgramOptions** implementation.

- The build scrips were updated to support Microsoft Visual Studio 2022.

- **CObjectSelectionSystem**'s implementation was optimized.

- Now **zlib** library is linked statically.

- **ISceneManager::LoadSceneAsync**'s onResultCallback parameter now allows nullptr as an input.

- Now a resource can be unloaded and removed from the registry of the resource manager via **CBaseResource::Unload**.

- Fix creation of texture samples under OpenGL GAPI context.

- The implementation of **CTextureAtlas** was refactored and now it's updated on the fly.

- **CU8String** class is now just a namespace for helper functions to operate with UTF-8 encoded strings. **U8C** alias was
replaced with strongly typed version called **TUtf8CodePoint**.

- Now **CFileInputStream** and **CFileOuptuStream** support UTF-8 paths.

- The implementations of **CPackageFileReader** and **CPackageFileWriter** were refactored.

- Now **CFileLogger** adds date and time information for the log.

- **tde2_resource_packer** utility was updated to support compressed files within packages.

- The functions SwapXBytes were replaced with a template SwapBytes. Now the engine assumes that all numeric data is stored in little endian manner.
If it's run on big endian machines everything will be swapped into a little endian memory layout.

- Now **CBinaryArchiveWriter** and **CBinaryArchiveReader** are endian independent.

- The implementation of **CFramePostProcessor** was refactored and cleaned up

- The common performance of builtin systems was improved.

- The update of bounds for scene's instances was temporary disabled.

- Now all components use their own pool allocators.

- **Optick** library is not linked statically.

- Implementation of memory allocators was highly refactored.

- The common implementation of the memory manager as a subsystem was excluded. Now local usage of allocators and
memory arenas is more preffered way.

- The implementation of component factories was totally refactored to simplify creation of new ones.

- **CFileLogger** was rewritten in asynchronous manner. Now it uses its own thread to write logs.

- **CUIElementsProcessSystem** was optimized. Now elements are updated only if their layout's changed.

- **CTextureAtlas** now uses **CScopedPtr\<T\>** to store references to resources.

- **tde2_introspector** utility was updated to a new version.

- **ImGUIContext** now uses Dear ImGUI of 1.85 version.

- The API of **IGraphicsContext** was changed. The signature of **SetScissorRect** method was changed from (const TRectF32&) onto (const TRectU32&).

- **IDLLManager** moved from **IWindowSystem** into **IPluginManager** ownership

- **CGameUserSettings::mFlags** was moved into **CProjectSettings** class

- All resources are now wrapped with **CScopedPtr** when returned from the resource manager

- **CResourceManager** is migrated onto CScopedPtr usage.

- The storage of depth-stencil and rasterizer states was optimized in CD3D11GraphicsObjectsManager.

- CMemoryProfiler's instance is thread-safe now.

- Breaking changes. Now the engine and its toolset is targeted for x64 platforms. x86 isn't supported from this moment.

- Now settings are splitted into application specific and project ones

- The localization manager now reads its settings from **CProjectSettings**

- The logger now uses the smart pointer to control its lifetime

- **CDeferOperation** class is replaced with **Wrench::TDeferOperation**

- GraphicsContextTypeToString(E_GRAPHICS_CONTEXT_GAPI_TYPE) and StringToGraphicsContextType(const std::string&) were removed

***

## [0.5.40] 2022-05-10

### Fixed

- (Fixed issue #82) The incorrect mapping of mouse buttons under UNIX was fixed for **CUnixInputContext**.

- The implementation of **CPhysics2DSystem::InjectBindings** was fixed.

- Segmentation fault caused by **IRaycastContext::Raycast2DClosest** was fixed.

- Utilities configurations were fixed.

- Invalid deletion of GL shader program's handle was fixed.

- (Fixed issue #83) Incorrect initialization of textures storage in **COGLShader** class was fixed.

### Added

- The implementation of **CUnixInputContext::GetNormalizedMousePosition** was added.

- Processing of **TOnCharInputEvent** was added for **CUnixWindowSystem**.

### Changed

- Now LUT texture for color grading post-effect is loaded into the shader even if it's not used. This was done to fix some graphical issues under GL graphics context.

## [0.5.39] 2022-05-04

### Fixed

- **FMOD** libraries for UNIX were removed from ~FMODAudioContext~ plugin's directory. **FMODAudioContext** plugin's compilation is disabled now for UNIX platforms.
You should build it manually with installed FMOD libraries earlier.

- An invalid order of timer's tick was fixed for **CUnixTimer**.

- The invalid implementation of **CUnixWindowSystem::GetClientRect** was fixed.

- Incorrect processing of window resizing was fixed for GLGraphicsContext.

### Added

- **CUnixFileSystem::GetUserDirectory** and **CUnixFileSystem::GetApplicationDataPath** have got their implementations.

- **CUnixWindowSystem::GetClientRect**'s got implementation.

- Default resources for SandboxGame project.

- The implementation of **CUnixWindowSystem::SaveFileDialog** and **CUnixWindowSystem::OpenFileDialog**.

- **CUnixInputContext::SetOnCharInputCallback**'s got implementation.

## [0.5.38] 2022-04-28

### Fixed

- Some fixes were introduced for GLSL built-in shaders.

- Fix issues in CMakeLists.txt of **TDE2ResourcePacker** utility.

- Update **install.sh** file. Add zlib's dependencies installation command.

- Fix compilation under UNIX with gcc toolchain.

### Changed

- A new config macro was introduced which is **TDE2_BUILTIN_PERF_PROFILER_ENABLED**. **CPerfProfiler** is now disabled by default.

- **AssertImpl** was reimplemented to support messages output.

- (Fixed issue #81) Now all applications can use default parameters if there is no configuration file at working directory.

## [0.5.37] 2022-04-19

### Added

- **CParticleEmitter** component now supports a non-looped playback mode.

- The inspector for **CParticleEmitter** was implemented.

### Changed

- **DrawGrid** function was moved into common editor utilities which is **EditorUtils.h**.

## [0.5.36] 2022-04-13

### Added

- Implementation of **tde2_mesh_converter** utility was extended to support LODs for meshes.

- A new type of component which is **CLODStrategy** was added. Also corresponding system **CLODMeshSwitchSystem** was defined to control LODs.

- The implementation of basic parallax mapping effect was added into **TDEngine2Lighting.inc** shader include file.

### Fixed

- **DefaultShader.shader**'s TBN matrix computation was fixed. 

## [0.5.35] 2022-04-06

### Added

- Support of **sub-mesh-id** property was added into **CStaticMeshContainer** component.

- The rendering of sub-meshes for static and skinned meshes was implemented.

### Changed

- A new iteration of binary mesh format was introduced.

- Current implementation of **CBinaryMeshFileReader** was reimplemented to support newer version of meshes.

### Fixed

- A typo in meshConverter.cpp was fixed. Indices count was written instead of faces' amount.

- (Fixed issue #77) The incorrect invocation of memcpy was fixed in **CImGUIContext::TextField**.

## [0.5.34] 2022-03-31

### Added

- Support of capturing entity's state was added for all existing type of tracks in TDEAnimationEd. 

## [0.5.33] 2022-03-30

### Added

- Add support of sampling modes' switching for animation tracks via its context menu. 

- Add support of switching between sampling modes in curve's editor.

- An experimental implementation of a cubic interpolation mode was added into animation tracks.

- The API of **TColor<T>** was extended with new operators overloadings.

### Fixed

- (Fixed issue #74) Initialization of animation tracks was fixed.

- The incorrect restore of CYAMLArchiveWriter's state was fixed when an array is written.

- The GPU memory access failure in CImGUIContext plugin when buffers are reallocated was fixed. 

- (Fixed issue #75) Rendering of animation curves was fixed.

- (Fixed issue #76) Assertion caused by TDEAnimationEd was fixed.

### Changed

- Now **CAnimationCurve**'s track uses tangents instead of control points to change its curvature.

- API of **IArchiveReader** was updated to support default values for getters.

- **ResolveBinding** was exposed to be a part of public API.

## [0.5.32] 2022-03-16

### Added

- Serialization/Deserialization of **GridGroupLayout** component was added.

- Add support of scaling curves in track sheets' editor mode of the animation editor utility.

- A pallete of editing channels for TVector2/3 curves was added for AnimationEd utility

### Fixed

- The issue when no camera isn't marked as active was fixed. Now if there is no an active game camera the editor's one is used for that role.

- Behaviour of **CImGUIContext::VerticalSeparator** method was fixed.

- Animation curve's points modification was refactored to improve UX.

### Changed

- **Wrench** extra submodule was removed from dependencies of the engine's toolset. 

- **TexturePacker**'s argparse dependency was replaced with internal **CProgramOptions** implementation.

## [0.5.31] 2022-03-08

### Added

- A new component which is **GridGroupLayout** was added.

### Changed

- The build scrips were updated to support Microsoft Visual Studio 2022.

### Fixed

## [0.5.30] 2022-02-21

### Added

- The support of selection of UI elements in the developer mode was added. 

### Changed

- **CObjectSelectionSystem**'s implementation was optimized.

### Fixed

- The editing of **LayoutElement** anchors and points was fixed.

## [0.5.29] 2022-02-20

### Added

- The API of **IFont** was extended with a new public method GetDataVersionNumber.

- A new component which is **9SliceImage** was added into the engine.

- Load/Save methods of **C9SliceImage**, **CLayoutElement**, **CLabel**, **CCanvas**, **CImage**, **CInputReceiver** got their implementations.
- Load/Save methods of **CAnimationContainerComponent**, **CMeshAnimatorComponent** got their implementations.

- The identifier of a default skybox's material now can be configured from the project settings.

### Changed

- Now **zlib** library is linked statically.

- **ISceneManager::LoadSceneAsync**'s onResultCallback parameter now allows nullptr as an input.

### Fixed

- The implementation of **CU8String::MoveNext** was fixed.

- **CSkinnedMeshContainer::Save** implementation was fixed to support serialization/deserialization of skeleton's identifier.

## [0.5.28] 2022-02-06

### Added

- A new library **stb_rect_pack** was added into the dependencies directory.

### Changed

- Now a resource can be unloaded and removed from the registry of the resource manager via **CBaseResource::Unload**.

- Fix creation of texture samples under OpenGL GAPI context.

- The implementation of **CTextureAtlas** was refactored and now it's updated on the fly.

### Fixed

- Now renderer's output under both D3D and GL graphics context looks the same and consistent.

- Receiving shadows by skinned meshes was fixed.

- The generation of texture atlas for glyphs in CRuntimeFont was fixed. Now it's constrained with lexicographical order.

## [0.5.27] 2022-02-01

### Added

- Add **zlib** into dependencies directory. Also build scripts were updated due to the new library.

- Now **IPackageFileWriter** and **IPackageFileReader** supports compressed files within packages. All conversions are done implicitly.

### Changed

- **CU8String** class is now just a namespace for helper functions to operate with UTF-8 encoded strings. **U8C** alias was
replaced with strongly typed version called **TUtf8CodePoint**.

- Now **CFileInputStream** and **CFileOuptuStream** support UTF-8 paths.

- The implementations of **CPackageFileReader** and **CPackageFileWriter** were refactored.

- Now **CFileLogger** adds date and time information for the log.

- **tde2_resource_packer** utility was updated to support compressed files within packages.

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

***

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