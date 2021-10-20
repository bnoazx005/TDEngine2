/// The file contains deinition of deleters for CScopedPtr
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IEventManager.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IPluginManager.h"
#include "../../include/core/memory/IMemoryManager.h"
#include "../../include/core/memory/IAllocator.h"


namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(IFileSystem)
	TDE2_DEFINE_SCOPED_PTR(IEventManager)
	TDE2_DEFINE_SCOPED_PTR(IResourceManager)
	TDE2_DEFINE_SCOPED_PTR(IGraphicsContext)
	TDE2_DEFINE_SCOPED_PTR(IWindowSystem)
	TDE2_DEFINE_SCOPED_PTR(IJobManager)
	TDE2_DEFINE_SCOPED_PTR(IPluginManager)
	TDE2_DEFINE_SCOPED_PTR(IMemoryManager)
	TDE2_DEFINE_SCOPED_PTR(IAllocator)
}