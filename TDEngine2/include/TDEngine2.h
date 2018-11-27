/*!
	/file TDEngine.h
	/date 14.09.2018
	/authors Kasimov Ildar
*/

#pragma once

///core
#include "core/IEngineCore.h"
#include "core/IEngineSubsystem.h"
#include "core/IWindowSystem.h"
#include "core/IGraphicsContext.h"
#include "core/CEngineCore.h"
#include "core/IEngineCoreBuilder.h"
#include "core/CDefaultEngineCoreBuilder.h"
#include "core/IEngineListener.h"
#include "core/IBaseObject.h"
#include "core/CBaseObject.h"
#include "core/IPlugin.h"
#include "core/IDLLManager.h"
#include "core/IFileSystem.h"
#include "core/IFile.h"
#include "core/CBaseFileSystem.h"
#include "core/IResource.h"
#include "core/CBaseResource.h"
#include "core/IResourceManager.h"
#include "core/IResourceLoader.h"
#include "core/CResourceManager.h"
#include "core/IResourceHandler.h"
#include "core/IJobManager.h"
#include "core/CBaseJobManager.h"
#include "core/IPluginManager.h"
#include "core/CBasePluginManager.h"
#include "core/IResourceFactory.h"
#include "core/memory/IAllocator.h"
#include "core/memory/CBaseAllocator.h"
#include "core/memory/CLinearAllocator.h"
#include "core/memory/CStackAllocator.h"
#include "core/memory/CPoolAllocator.h"
#include "core/Event.h"
#include "core/IEventManager.h"
#include "core/CEventManager.h"

///ecs
#include "ecs/CEntity.h"
#include "ecs/CEntityManager.h"
#include "ecs/CComponentManager.h"
#include "ecs/IComponent.h"
#include "ecs/CBaseComponent.h"
#include "ecs/ISystem.h"
#include "ecs/ISystemManager.h"
#include "ecs/CSystemManager.h"
#include "ecs/IWorld.h"
#include "ecs/CWorld.h"
#include "ecs/ITransform.h"
#include "ecs/CTransform.h"
#include "ecs/IComponentFactory.h"
#include "ecs/IComponentManager.h"

///graphics
#include "graphics/IBuffer.h"
#include "graphics/IVertexBuffer.h"
#include "graphics/IIndexBuffer.h"
#include "graphics/IConstantBuffer.h"
#include "graphics/IVertexDeclaration.h"
#include "graphics/IShader.h"
#include "graphics/CBaseShader.h"
#include "graphics/IShaderCompiler.h"
#include "graphics/CBaseShaderLoader.h"
#include "graphics/ISprite.h"
#include "graphics/CQuadSprite.h"
#include "graphics/ITexture.h"
#include "graphics/ITexture2D.h"
#include "graphics/ITextureCube.h"
#include "graphics/CBaseTexture2D.h"
#include "graphics/ICamera.h"
#include "graphics/CBaseCamera.h"
#include "graphics/CPerspectiveCamera.h"
#include "graphics/COrthoCamera.h"
#include "graphics/CBaseShaderCompiler.h"
#include "graphics/CVertexDeclaration.h"
#include "graphics/InternalShaderData.h"

///math
#include "math/TVector3.h"
#include "math/TVector4.h"
#include "math/TMatrix3.h"
#include "math/TMatrix4.h"
#include "math/TQuaternion.h"
#include "math/TRect.h"
#include "math/MathUtils.h"

///platform
#include "platform/win32/CWin32WindowSystem.h"
#include "platform/win32/CWin32Timer.h"
#include "platform/win32/CWin32DLLManager.h"
#include "platform/win32/CWin32FileSystem.h"
#include "platform/CTextFileReader.h"
#include "platform/unix/CUnixWindowSystem.h"
#include "platform/unix/CUnixDLLManager.h"
#include "platform/unix/CUnixFileSystem.h"
#include "platform/unix/CUnixTimer.h"
#include "platform/CConfigFileReader.h"
#include "platform/CBaseFile.h"

///utils
#include "utils/Types.h"
#include "utils/Config.h"
#include "utils/Color.h"
#include "utils/ILogger.h"
#include "utils/CFileLogger.h"
#include "utils/ITimer.h"
#include "utils/Utils.h"
#include "utils/CResult.h"

/// D3D11GraphicsContext plugin's header
#if defined (TDE2_BUILD_D3D11_GCTX_PLUGIN)
	#include "./../plugins/D3D11GraphicsContext/include/CD3D11GCtxPlugin.h"
#endif

/// OGLGraphicsContext plugin's header
#if defined (TDE2_BUILD_OGL_GCTX_PLUGIN)
	#include "./../plugins/OGLGraphicsContext/include/COGLGCtxPlugin.h"
#endif