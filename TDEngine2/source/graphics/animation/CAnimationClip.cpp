#include "../../../include/graphics/animation/CAnimationClip.h"
#include "../../../include/core/IResourceManager.h"
#include "../../../include/core/IGraphicsContext.h"
#include "../../../include/graphics/animation/IAnimationTrack.h"
#include "../../../include/graphics/animation/AnimationTracks.h"
#include "../../../include/metadata.h"


namespace TDEngine2
{
	const std::string TAnimationEvents::mOnStart = "on_start";
	const std::string TAnimationEvents::mOnFinished = "on_finished";


	struct TAnimationClipKeys
	{
		static const std::string mDurationKeyId;
		static const std::string mWrapModeKeyId;
		static const std::string mTracksKeyId;
		static const std::string mSingleTrackKeyId;
		static const std::string mTypeIdKeyId;
	};

	const std::string TAnimationClipKeys::mDurationKeyId = "duration";
	const std::string TAnimationClipKeys::mWrapModeKeyId = "wrap-mode";
	const std::string TAnimationClipKeys::mTracksKeyId = "tracks";
	const std::string TAnimationClipKeys::mSingleTrackKeyId = "track";
	const std::string TAnimationClipKeys::mTypeIdKeyId = "type_id";


	const CAnimationClip::TAnimationTracksFactory CAnimationClip::mTracksFactory
	{
		{ CVector2AnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateVector2AnimationTrack(pClip, result); } },
		{ CVector3AnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateVector3AnimationTrack(pClip, result); } },
		{ CQuaternionAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateQuaternionAnimationTrack(pClip, result); } },
		{ CColorAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateColorAnimationTrack(pClip, result); } },
		{ CBooleanAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateBooleanAnimationTrack(pClip, result); } },
		{ CFloatAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateFloatAnimationTrack(pClip, result); } },
		{ CIntegerAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateIntegerAnimationTrack(pClip, result); } },
		{ CEventAnimationTrack::GetTypeId(), [](IAnimationClip* pClip) { E_RESULT_CODE result = RC_OK; return CreateEventAnimationTrack(pClip, result); } },
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
		E_RESULT_CODE result = RC_OK;

		while (!mpTracks.empty())
		{
			if (IAnimationTrack* pTrack = mpTracks.begin()->second)
			{
				result = result | pTrack->Free();
			}

			mpTracks.erase(mpTracks.cbegin());
		}

		return result;
	}

	E_RESULT_CODE CAnimationClip::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mDuration = pReader->GetFloat(TAnimationClipKeys::mDurationKeyId);
		mWrapMode = Meta::EnumTrait<E_ANIMATION_WRAP_MODE_TYPE>::FromString(pReader->GetString(TAnimationClipKeys::mWrapModeKeyId));

		E_RESULT_CODE result = RC_OK;

		pReader->BeginGroup(TAnimationClipKeys::mTracksKeyId);
		{
			while (pReader->HasNextItem())
			{
				pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					pReader->BeginGroup(TAnimationClipKeys::mSingleTrackKeyId);

					const TypeId trackTypeId = static_cast<TypeId>(pReader->GetUInt32(TAnimationClipKeys::mTypeIdKeyId));

					auto trackHandle = _createTrackInternal(trackTypeId, "#");
					if (TAnimationTrackId::Invalid == trackHandle)
					{
						TDE2_ASSERT(false);
						result = result | RC_FAIL;
					}

					if (trackTypeId == CEventAnimationTrack::GetTypeId()) // \note Cache handle of an event track for fast access
					{
						mEventTrackHandle = trackHandle;
					}

					if (IAnimationTrack* pTrack = mpTracks[trackHandle])
					{
						pTrack->Load(pReader);
					}

					pReader->EndGroup();
				}
				pReader->EndGroup();
			}
		}
		pReader->EndGroup();

		return result;
	}
	
	E_RESULT_CODE CAnimationClip::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup("meta");
		{
			pWriter->SetString("resource-type", "animation-clip");
			pWriter->SetUInt16("version-tag", mVersionTag);
		}
		pWriter->EndGroup();

		pWriter->SetFloat(TAnimationClipKeys::mDurationKeyId, mDuration);
		pWriter->SetString(TAnimationClipKeys::mWrapModeKeyId, Meta::EnumTrait<E_ANIMATION_WRAP_MODE_TYPE>::ToString(mWrapMode)); // \todo replace with proper serialization of the enum type

		pWriter->BeginGroup(TAnimationClipKeys::mTracksKeyId, true);
		{
			for (auto&& currTrackEntity : mpTracks)
			{
				IAnimationTrack* pCurrTrack = currTrackEntity.second;

				pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					pWriter->BeginGroup(TAnimationClipKeys::mSingleTrackKeyId);
					{
						pWriter->SetUInt32(TAnimationClipKeys::mTypeIdKeyId, static_cast<U32>(pCurrTrack->GetTrackTypeId()));
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

		if (handle == mEventTrackHandle)
		{
			mEventTrackHandle = TAnimationTrackId::Invalid;
		}

		return RC_OK;
	}

	void CAnimationClip::ForEachTrack(const std::function<bool(TAnimationTrackId trackId, IAnimationTrack*)>& action)
	{
		if (!action)
		{
			return;
		}

		for (auto&& currTrackEntity : mpTracks)
		{
			auto&& pCurrTrack = currTrackEntity.second;
			if (!pCurrTrack)
			{
				continue;
			}

			if (!action(currTrackEntity.first, pCurrTrack))
			{
				break;
			}
		}
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

	IAnimationTrack* CAnimationClip::GetEventTrack() const
	{
		return (TAnimationTrackId::Invalid == mEventTrackHandle) ? nullptr : mpTracks.at(mEventTrackHandle);
	}

	U32 CAnimationClip::GetTracksCount() const
	{
		return static_cast<U32>(mpTracks.size());
	}

	const TPtr<IResourceLoader> CAnimationClip::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IAnimationClip>();
	}

	TAnimationTrackId CAnimationClip::_createTrackInternal(TypeId typeId, const std::string& name)
	{
		if (typeId == TypeId::Invalid)
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

		/// \note There could be the only event track for the clip
		if (CEventAnimationTrack::GetTypeId() == typeId)
		{
			mEventTrackHandle = handle;
		}

		pTrack->SetName(name.empty() ? Wrench::StringUtils::Format("Track{0}", static_cast<U32>(handle)) : name);

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

	E_RESULT_CODE CAnimationClipLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (TResult<TFileEntryId> animationClipFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<IAnimationClip*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(animationClipFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CAnimationClipLoader::GetResourceTypeId() const
	{
		return IAnimationClip::GetTypeId();
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
		return IAnimationClip::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateAnimationClipFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CAnimationClipFactory, result, pResourceManager, pGraphicsContext);
	}
}