#include "../../include/graphics/CSkeleton.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	struct TSkeletonArchiveKeys
	{
	};


	CSkeleton::CSkeleton() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CSkeleton::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mJoints.push_back({ 0, -1, "Root", IdentityMatrix4 }); /// \note Add default root node

		mState = E_RESOURCE_STATE_TYPE::RST_LOADED;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSkeleton::Reset()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}


	E_RESULT_CODE CSkeleton::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mJoints.clear();

		return RC_OK;
	}

	E_RESULT_CODE CSkeleton::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource_type", "skeleton");
			pWriter->SetUInt16("version_tag", mAssetsVersionTag);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	TResult<U32> CSkeleton::CreateJoint(const std::string& name, const TMatrix4& bindTransform, I32 parent)
	{
		if (parent > static_cast<I32>(mJoints.size()))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		auto&& it = std::find_if(mJoints.cbegin(), mJoints.cend(), [&name](const TJoint& joint) { return name == joint.mName; });
		if (it == mJoints.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		const U32 jointId = static_cast<I32>(mJoints.size());
		mJoints.push_back({ jointId, parent, name, bindTransform });

		return Wrench::TOkValue<U32>(jointId);
	}

	E_RESULT_CODE CSkeleton::RemoveJoint(const std::string& name)
	{
		auto it = std::find_if(mJoints.begin(), mJoints.end(), [&name](const TJoint& joint) { return name == joint.mName; });
		if (it == mJoints.end())
		{
			return RC_FAIL;
		}

		const I32 pos = static_cast<I32>(std::distance(mJoints.begin(), it));
		it = mJoints.erase(it);

		for (; it != mJoints.end(); ++it) /// \note Update all indices and parent-child links 
		{
			auto& currJoint = *it;

			--currJoint.mIndex;
			
			if (currJoint.mParentIndex >= pos)
			{
				--currJoint.mParentIndex;
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CSkeleton::RemoveJoint(U32 id)
	{
		if (id >= static_cast<U32>(mJoints.size()))
		{
			return RC_INVALID_ARGS;
		}

		auto it = mJoints.erase(mJoints.begin() + id);

		for (; it != mJoints.end(); ++it) /// \note Update all indices and parent-child links 
		{
			auto& currJoint = *it;

			--currJoint.mIndex;

			if (currJoint.mParentIndex >= static_cast<I32>(id))
			{
				--currJoint.mParentIndex;
			}
		}

		return RC_OK;
	}
	
	TResult<TJoint*> CSkeleton::GetJoint(U32 id)
	{
		if (id >= static_cast<U32>(mJoints.size()))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		return Wrench::TOkValue<TJoint*>(&mJoints[id]);
	}

	const IResourceLoader* CSkeleton::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ISkeleton>();
	}


	TDE2_API ISkeleton* CreateSkeleton(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISkeleton, CSkeleton, result, pResourceManager, pGraphicsContext, name);
	}


	CSkeletonLoader::CSkeletonLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSkeletonLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pGraphicsContext || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpFileSystem      = pFileSystem;
		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSkeletonLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CSkeletonLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (TResult<TFileEntryId> materialFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<ISkeleton*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(materialFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CSkeletonLoader::GetResourceTypeId() const
	{
		return ISkeleton::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateSkeletonLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CSkeletonLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	CSkeletonFactory::CSkeletonFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSkeletonFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSkeletonFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CSkeletonFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TSkeletonParameters& meshParams = dynamic_cast<const TSkeletonParameters&>(params);

		return dynamic_cast<IResource*>(CreateSkeleton(mpResourceManager, mpGraphicsContext, name, result));
	}

	IResource* CSkeletonFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateSkeleton(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CSkeletonFactory::GetResourceTypeId() const
	{
		return ISkeleton::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateSkeletonFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CSkeletonFactory, result, pResourceManager, pGraphicsContext);
	}
}