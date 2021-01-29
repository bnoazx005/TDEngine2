#include "../../../include/graphics/animation/CAnimationClip.h"
#include "../../../include/core/IResourceManager.h"
#include "../../../include/core/IGraphicsContext.h"
#include "../../../include/graphics/animation/IAnimationTrack.h"
#include "../../../include/graphics/animation/AnimationTracks.h"
#include "../../../include/metadata.h"


namespace TDEngine2
{
	const CAnimationClip::TAnimationTracksFactory CAnimationClip::mTracksFactory
	{
		{ CVector2AnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateVector2AnimationTrack(pClip, result); } },
		{ CVector3AnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateVector3AnimationTrack(pClip, result); } },
		{ CQuaternionAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateQuaternionAnimationTrack(pClip, result); } },
		{ CColorAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateColorAnimationTrack(pClip, result); } },
		{ CBooleanAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateBooleanAnimationTrack(pClip, result); } },
		{ CFloatAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateFloatAnimationTrack(pClip, result); } },
		{ CIntegerAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateIntegerAnimationTrack(pClip, result); } },
	};

	CAnimationClip::CAnimationClip() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CAnimationClip::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationClip::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
									   const TAnimationClipParameters& params)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		if ((result = SetDuration(params.mDuration)) != RC_OK)
		{
			return result;
		}

		mWrapMode = params.mWrapMode;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationClip::Reset()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CAnimationClip::Load(IArchiveReader* pReader)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}
	
	E_RESULT_CODE CAnimationClip::Save(IArchiveWriter* pWriter)
	{
		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource-type", "animation-clip");
			pWriter->SetUInt16("version-tag", mVersionTag);
		}
		pWriter->EndGroup();

		pWriter->SetFloat("duration", mDuration);
		pWriter->SetString("wrap-mode", Meta::EnumTrait<E_ANIMATION_WRAP_MODE_TYPE>::ToString(mWrapMode)); // \todo replace with proper serialization of the enum type

		pWriter->BeginGroup("tracks", true);		
		{
			for (auto&& currTrackEntity : mpTracks)
			{
				IAnimationTrack* pCurrTrack = currTrackEntity.second;

				pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					pWriter->BeginGroup("track");
					{
						pWriter->SetUInt32("type_id", static_cast<U32>(pCurrTrack->GetTrackTypeId()));
						E_RESULT_CODE result = pCurrTrack->Save(pWriter);
						TDE2_ASSERT(result == RC_OK);
					}
					pWriter->EndGroup();
				}
				pWriter->EndGroup();
			}
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CAnimationClip::RemoveTrack(TAnimationTrackId handle)
	{
		if (handle == TAnimationTrackId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		auto iter = mpTracks.find(handle);
		if (iter == mpTracks.cend())
		{
			return RC_FAIL;
		}

		mpTracks.erase(iter);

		return RC_OK;
	}

	E_RESULT_CODE CAnimationClip::SetDuration(F32 duration)
	{
		if (duration < 0.0f)
		{
			TDE2_ASSERT(duration > 0.0f);
			return RC_INVALID_ARGS;
		}

		mDuration = duration;

		return RC_OK;
	}

	void CAnimationClip::SetWrapMode(E_ANIMATION_WRAP_MODE_TYPE mode)
	{
		mWrapMode = mode;
	}

	F32 CAnimationClip::GetDuration() const
	{
		return mDuration;
	}

	E_ANIMATION_WRAP_MODE_TYPE CAnimationClip::GetWrapMode() const
	{
		return mWrapMode;
	}

	const IResourceLoader* CAnimationClip::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<CAnimationClip>();;
	}

	TAnimationTrackId CAnimationClip::_createTrackInternal(TypeId typeId, const std::string& name)
	{
		if (typeId == TypeId::Invalid || name.empty())
		{
			return TAnimationTrackId::Invalid;
		}

		auto factoryIter = mTracksFactory.find(typeId);
		if (factoryIter == mTracksFactory.cend())
		{
			TDE2_ASSERT(false);
			return TAnimationTrackId::Invalid;
		}

		IAnimationTrack* pTrack = (factoryIter->second)(this);
		if (!pTrack)
		{
			TDE2_ASSERT(false);
			return TAnimationTrackId::Invalid;
		}

		TAnimationTrackId handle = TAnimationTrackId(mpTracks.size());
		mpTracks.insert({ handle, pTrack });

		pTrack->SetName(name);

		return handle;
	}

	IAnimationTrack* CAnimationClip::_getTrackInternal(TAnimationTrackId handle) const
	{
		auto iter = mpTracks.find(handle);

		if (handle == TAnimationTrackId::Invalid || iter == mpTracks.cend())
		{
			TDE2_ASSERT(false);
			return nullptr;
		}

		return iter->second;
	}


	TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												 const TAnimationClipParameters& params, E_RESULT_CODE& result)
	{

		return CREATE_IMPL(IAnimationClip, CAnimationClip, result, pResourceManager, pGraphicsContext, name, params);
	}


	TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												 E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationClip, CAnimationClip, result, pResourceManager, pGraphicsContext, name);
	}


	/*!
		\brief CAnimationClipLoader's definition
	*/

	CAnimationClipLoader::CAnimationClipLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CAnimationClipLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

		mpFileSystem = pFileSystem;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationClipLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationClipLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		TDE2_UNIMPLEMENTED();

		return RC_OK;// dynamic_cast<IAnimationClip*>(pResource)->Lo(mpFileSystem, pResource->GetName() + ".info");
	}

	TypeId CAnimationClipLoader::GetResourceTypeId() const
	{
		return CAnimationClip::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateAnimationClipLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CAnimationClipLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	/*!
		\brief CAnimationClipFactory's definition
	*/

	CAnimationClipFactory::CAnimationClipFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CAnimationClipFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CAnimationClipFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CAnimationClipFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateAnimationClip(mpResourceManager, mpGraphicsContext, name, dynamic_cast<const TAnimationClipParameters&>(params), result));
	}

	IResource* CAnimationClipFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateAnimationClip(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CAnimationClipFactory::GetResourceTypeId() const
	{
		return CAnimationClip::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateAnimationClipFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CAnimationClipFactory, result, pResourceManager, pGraphicsContext);
	}
}