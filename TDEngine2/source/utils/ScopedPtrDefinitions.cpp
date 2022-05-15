/// The file contains deinition of deleters for CScopedPtr
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IEventManager.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IPluginManager.h"
#include "../../include/core/IEngineCore.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/core/IResourceFactory.h"
#include "../../include/core/IResourceLoader.h"
#include "../../include/core/IResource.h"
#include "../../include/platform/MountableStorages.h"
#include "../../include/platform/IOStreams.h"
#include "../../include/ecs/IComponentManager.h"
#include "../../include/physics/IRaycastContext.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/graphics/ITexture2D.h"
#include "../../include/graphics/IGlobalShaderProperties.h"
#include "../../include/graphics/IFramePostProcessor.h"
#include "../../include/graphics/CRenderQueue.h"


namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(IFileSystem)
	TDE2_DEFINE_SCOPED_PTR(IEventManager)
	TDE2_DEFINE_SCOPED_PTR(IResourceManager)
	TDE2_DEFINE_SCOPED_PTR(IGraphicsContext)
	TDE2_DEFINE_SCOPED_PTR(IWindowSystem)
	TDE2_DEFINE_SCOPED_PTR(IJobManager)
	TDE2_DEFINE_SCOPED_PTR(IPluginManager)
	TDE2_DEFINE_SCOPED_PTR(IAllocator)
	TDE2_DEFINE_SCOPED_PTR(IAllocatorFactory)
	TDE2_DEFINE_SCOPED_PTR(IMountableStorage)
	TDE2_DEFINE_SCOPED_PTR(IStream)
	TDE2_DEFINE_SCOPED_PTR(IInputStream)
	TDE2_DEFINE_SCOPED_PTR(IOutputStream)
	TDE2_DEFINE_SCOPED_PTR(IComponentFactory)
	TDE2_DEFINE_SCOPED_PTR(IMaterialInstance)
	TDE2_DEFINE_SCOPED_PTR(IRaycastContext)
	TDE2_DEFINE_SCOPED_PTR(IResourceFactory)
	TDE2_DEFINE_SCOPED_PTR(IResourceLoader)
	TDE2_DEFINE_SCOPED_PTR(IMaterial)
	TDE2_DEFINE_SCOPED_PTR(IResource)
	TDE2_DEFINE_SCOPED_PTR(IEngineCore)
	TDE2_DEFINE_SCOPED_PTR(ITexture2D)
	TDE2_DEFINE_SCOPED_PTR(IGlobalShaderProperties)
	TDE2_DEFINE_SCOPED_PTR(IFramePostProcessor)
	TDE2_DEFINE_SCOPED_PTR(CRenderQueue)
}