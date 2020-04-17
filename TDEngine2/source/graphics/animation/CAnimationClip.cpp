#include "../../../include/graphics/animation/CAnimationClip.h"
#include "../../../include/core/IResourceManager.h"
#include "../../../include/core/IGraphicsContext.h"
#include "../../../include/graphics/animation/IAnimationTrack.h"


namespace TDEngine2
{
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

	E_RESULT_CODE CAnimationClip::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CAnimationClip>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = RST_PENDING;

			return result;
		}

		mState = RST_LOADED;

		return result;
	}

	E_RESULT_CODE CAnimationClip::Unload()
	{
		return Reset();
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
		pWriter->SetUInt8("wrap-mode", static_cast<U8>(mWrapMode)); // \todo replace with proper serialization of the enum type

		pWriter->BeginGroup("tracks", true);		
		{
			for (auto&& pCurrTrack : mpTracks)
			{
				TDE2_ASSERT(pCurrTrack->Save(pWriter) != RC_OK);
			}
		}
		pWriter->EndGroup();

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


	TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												 const TAnimationClipParameters& params, E_RESULT_CODE& result)
	{
		CAnimationClip* pAnimationClipInstance = new (std::nothrow) CAnimationClip();

		if (!pAnimationClipInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pAnimationClipInstance->Init(pResourceManager, pGraphicsContext, name, params);

		if (result != RC_OK)
		{
			delete pAnimationClipInstance;

			pAnimationClipInstance = nullptr;
		}

		return pAnimationClipInstance;
	}


	TDE2_API IAnimationClip* CreateAnimationClip(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												 E_RESULT_CODE& result)
	{
		CAnimationClip* pAnimationClipInstance = new (std::nothrow) CAnimationClip();

		if (!pAnimationClipInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pAnimationClipInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pAnimationClipInstance;

			pAnimationClipInstance = nullptr;
		}

		return pAnimationClipInstance;
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

	U32 CAnimationClipLoader::GetResourceTypeId() const
	{
		return CAnimationClip::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateAnimationClipLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CAnimationClipLoader* pAnimationClipLoaderInstance = new (std::nothrow) CAnimationClipLoader();

		if (!pAnimationClipLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pAnimationClipLoaderInstance->Init(pResourceManager, pGraphicsContext, pFileSystem);

		if (result != RC_OK)
		{
			delete pAnimationClipLoaderInstance;

			pAnimationClipLoaderInstance = nullptr;
		}

		return pAnimationClipLoaderInstance;
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

	U32 CAnimationClipFactory::GetResourceTypeId() const
	{
		return CAnimationClip::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateAnimationClipFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CAnimationClipFactory* pAnimationClipFactoryInstance = new (std::nothrow) CAnimationClipFactory();

		if (!pAnimationClipFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pAnimationClipFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pAnimationClipFactoryInstance;

			pAnimationClipFactoryInstance = nullptr;
		}

		return pAnimationClipFactoryInstance;
	}
}