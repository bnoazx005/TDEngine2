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
#include "core/memory/IMemoryManager.h"
#include "core/memory/CMemoryManager.h"
#include "core/Event.h"
#include "core/IEventManager.h"
#include "core/CEventManager.h"
#include "core/IInputContext.h"
#include "core/IInputDevice.h"
#include "core/CConfigFileEngineCoreBuilder.h"
#include "core/IFont.h"
#include "core/CFont.h"

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
#include "ecs/CTransformSystem.h"
#include "ecs/CSpriteRendererSystem.h"
#include "ecs/ICameraSystem.h"
#include "ecs/CCameraSystem.h"
#include "ecs/CPhysics2DSystem.h"
#include "ecs/CStaticMeshRendererSystem.h"
#include "ecs/CBaseSystem.h"

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
#include "graphics/ICubemapTexture.h"
#include "graphics/CBaseTexture2D.h"
#include "graphics/CBaseCubemapTexture.h"
#include "graphics/ICamera.h"
#include "graphics/CBaseCamera.h"
#include "graphics/CPerspectiveCamera.h"
#include "graphics/COrthoCamera.h"
#include "graphics/CBaseShaderCompiler.h"
#include "graphics/CVertexDeclaration.h"
#include "graphics/InternalShaderData.h"
#include "graphics/IGraphicsObjectManager.h"
#include "graphics/CBaseGraphicsObjectManager.h"
#include "graphics/IRenderTarget.h"
#include "graphics/CBaseRenderTarget.h"
#include "graphics/IRenderable.h"
#include "graphics/IRenderer.h"
#include "graphics/CForwardRenderer.h"
#include "graphics/CRenderQueue.h"
#include "graphics/IGraphicsLayersInfo.h"
#include "graphics/CGraphicsLayersInfo.h"
#include "graphics/IMaterial.h"
#include "graphics/CBaseMaterial.h"
#include "graphics/IGlobalShaderProperties.h"
#include "graphics/CGlobalShaderProperties.h"
#include "graphics/ITextureAtlas.h"
#include "graphics/CTextureAtlas.h"
#include "graphics/IDebugUtility.h"
#include "graphics/CDebugUtility.h"
#include "graphics/IMesh.h"
#include "graphics/CStaticMesh.h"
#include "graphics/VertexData.h"
#include "graphics/IStaticMeshContainer.h"
#include "graphics/CStaticMeshContainer.h"


///math
#include "math/TVector2.h"
#include "math/TVector3.h"
#include "math/TVector4.h"
#include "math/TMatrix3.h"
#include "math/TMatrix4.h"
#include "math/TQuaternion.h"
#include "math/TRect.h"
#include "math/MathUtils.h"

///physics
#include "physics/2D/ICollisionObject2D.h"
#include "physics/2D/CBaseCollisionObject2D.h"
#include "physics/2D/CBoxCollisionObject2D.h"
#include "physics/2D/CCircleCollisionObject2D.h"
#include "physics/2D/ICollisionObjectsVisitor.h"
#include "physics/2D/ITrigger2D.h"
#include "physics/2D/CTrigger2D.h"

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
#include "platform/CBinaryFileReader.h"
#include "platform/CBinaryFileWriter.h"
#include "platform/CImageFileWriter.h"
#include "platform/CYAMLFile.h"
#include "platform/CBinaryMeshFileReader.h"

///utils
#include "utils/Types.h"
#include "utils/Config.h"
#include "utils/Color.h"
#include "utils/ILogger.h"
#include "utils/CFileLogger.h"
#include "utils/ITimer.h"
#include "utils/Utils.h"
#include "utils/CResult.h"
#include "utils/CResourceContainer.h"
#include "utils/CU8String.h"
#include "utils/CContainers.h"

/// D3D11GraphicsContext plugin's header
#if defined (TDE2_BUILD_D3D11_GCTX_PLUGIN)
	#include "./../plugins/D3D11GraphicsContext/include/CD3D11GCtxPlugin.h"
#endif

/// OGLGraphicsContext plugin's header
#if defined (TDE2_BUILD_OGL_GCTX_PLUGIN)
	#include "./../plugins/OGLGraphicsContext/include/COGLGCtxPlugin.h"
#endif