#include "../../include/graphics/CSkeleton.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/core/IJobManager.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/utils/CFileLogger.h"
#include <queue>
#include <tuple>


namespace TDEngine2
{
	struct TSkeletonArchiveKeys
	{
		static const std::string mJointsGroupKey;

		struct TJointsGroup
		{
			static const std::string mIndexKey;
			static const std::string mParentIndexKey;
			static const std::string mNameKey;
			static const std::string mBindTransformKey;
		};

		static const std::string mStoresInvBindFlagKey;
	};


	const std::string TSkeletonArchiveKeys::mJointsGroupKey = "joints";

	const std::string TSkeletonArchiveKeys::TJointsGroup::mIndexKey = "id";
	const std::string TSkeletonArchiveKeys::TJointsGroup::mParentIndexKey = "parent_id";
	const std::string TSkeletonArchiveKeys::TJointsGroup::mNameKey = "name";
	const std::string TSkeletonArchiveKeys::TJointsGroup::mBindTransformKey = "bind_transform";

	const std::string TSkeletonArchiveKeys::mStoresInvBindFlagKey = "stores_inv_bind_poses";


	static TMatrix4 ComputeInvBindTransform(const std::vector<TJoint>& joints)
	{
		return IdentityMatrix4;
	}


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

		TJoint tmpJoint;

		pReader->BeginGroup(TSkeletonArchiveKeys::mJointsGroupKey);
		{
			while (pReader->HasNextItem())
			{
				pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					tmpJoint.mIndex       = pReader->GetUInt32(TSkeletonArchiveKeys::TJointsGroup::mIndexKey);
					tmpJoint.mParentIndex = pReader->GetInt32(TSkeletonArchiveKeys::TJointsGroup::mParentIndexKey);
					tmpJoint.mName        = pReader->GetString(TSkeletonArchiveKeys::TJointsGroup::mNameKey);

					pReader->BeginGroup(TSkeletonArchiveKeys::TJointsGroup::mBindTransformKey);
					{
						if (auto matrixLoadResult = LoadMatrix4(pReader))
						{
							tmpJoint.mLocalBindTransform = matrixLoadResult.Get();
						}
					}
					pReader->EndGroup();
				}
				pReader->EndGroup();

				mJoints.push_back(tmpJoint);
			}
		}
		pReader->EndGroup();

		mShouldStoreInvBindPoses = pReader->GetBool(TSkeletonArchiveKeys::mStoresInvBindFlagKey);

		TDE2_ASSERT(!mJoints.empty());

		if (mShouldStoreInvBindPoses) /// \note Don't need postLoad because matrices were already inverted and computed previously
		{
			return RC_OK;
		}

		return _postLoad();
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

		pWriter->BeginGroup(TSkeletonArchiveKeys::mJointsGroupKey, true);
		{
			for (const TJoint& currJoint : mJoints)
			{
				pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					pWriter->SetUInt32(TSkeletonArchiveKeys::TJointsGroup::mIndexKey, currJoint.mIndex);
					pWriter->SetInt32(TSkeletonArchiveKeys::TJointsGroup::mParentIndexKey, currJoint.mParentIndex);
					pWriter->SetString(TSkeletonArchiveKeys::TJointsGroup::mNameKey, currJoint.mName);

					pWriter->BeginGroup(TSkeletonArchiveKeys::TJointsGroup::mBindTransformKey);
					{
						SaveMatrix4(pWriter, currJoint.mLocalBindTransform);
					}
					pWriter->EndGroup();
				}
				pWriter->EndGroup();
			}
		}
		pWriter->EndGroup();

		pWriter->SetBool(TSkeletonArchiveKeys::mStoresInvBindFlagKey, mShouldStoreInvBindPoses);

		return RC_OK;
	}

	TResult<U32> CSkeleton::CreateJoint(const std::string& name, const TMatrix4& bindTransform, I32 parent)
	{
		if (parent > static_cast<I32>(mJoints.size()))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		auto&& it = std::find_if(mJoints.cbegin(), mJoints.cend(), [&name](const TJoint& joint) { return name == joint.mName; });
		if (it != mJoints.cend())
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

	void CSkeleton::SetInvBindPoseUsing(bool flag)
	{
		mShouldStoreInvBindPoses = flag;
	}
	
	TJoint* CSkeleton::GetJoint(U32 id)
	{
		return (id >= static_cast<U32>(mJoints.size())) ? nullptr : &mJoints[id];
	}

	TJoint* CSkeleton::GetJointByName(const std::string& name)
	{
		auto it = std::find_if(mJoints.begin(), mJoints.end(), [&name](const TJoint& joint) { return name == joint.mName; });
		return (it == mJoints.end()) ? nullptr : &*it;
	}

	void CSkeleton::ForEachJoint(const std::function<void(TJoint*)>& action)
	{
		if (!action)
		{
			return;
		}

		for (TJoint& currJoint : mJoints)
		{
			action(&currJoint);
		}
	}

	void CSkeleton::ForEachChildJoint(U32 jointIndex, const std::function<void(TJoint*)>& action)
	{
		if (!action)
		{
			return;
		}

		for (TJoint& currJoint : mJoints)
		{
			if (static_cast<U32>(currJoint.mParentIndex) != jointIndex)
			{
				continue;
			}

			action(&currJoint);
		}
	}

	const IResourceLoader* CSkeleton::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ISkeleton>();
	}

	E_RESULT_CODE CSkeleton::_postLoad()
	{
		/// \note Compute inverted bind transforms for all joints
		/// We can assume that all joints are ordered based on child - parent relationship, parents are first

#if TDE2_DEBUG_MODE
		/// \note Check the assumption above in debug code
		for (size_t i = 0; i < mJoints.size(); ++i)
		{
			TDE2_ASSERT(mJoints[i].mParentIndex < static_cast<I32>(mJoints[i].mIndex));
		}
#endif

		std::queue<std::tuple<U32, TMatrix4>> jointsQueue;
		jointsQueue.emplace(0, IdentityMatrix4);

		while (!jointsQueue.empty())
		{
			auto&& entity = jointsQueue.front();
			jointsQueue.pop();

			TJoint& currJoint = mJoints[std::get<U32>(entity)];

			TMatrix4 bindTransform = std::get<TMatrix4>(entity) * currJoint.mLocalBindTransform;
			currJoint.mInvBindTransform = Inverse(bindTransform);

			for (U32 i = std::get<U32>(entity) + 1; i < mJoints.size(); ++i)
			{
				if (currJoint.mIndex != static_cast<U32>(mJoints[i].mParentIndex))
				{
					continue;
				}

				jointsQueue.emplace(mJoints[i].mIndex, bindTransform);
			}
		}

		return RC_OK;
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