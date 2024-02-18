#include "../include/CVideoProcessSystem.h"
#include "../include/CUIVideoContainerComponent.h"
#include <core/IResourceManager.h>
#include <core/IResource.h>
#include <core/IFileSystem.h>
#include <ecs/IWorld.h>
#include <graphics/ITexture2D.h>
#include <graphics/UI/CImageComponent.h>
#include <editor/CPerfProfiler.h>
#include "../deps/theoraplay/theoraplay.h"


namespace TDEngine2
{
	TInternalVideoData::~TInternalVideoData()
	{
		THEORAPLAY_stopDecode(mpDecoder);
		THEORAPLAY_freeVideo(mpCurrVideoFrame);
	}


	CVideoProcessSystem::CVideoProcessSystem() :
		CBaseSystem()
	{
	}

	CVideoProcessSystem::~CVideoProcessSystem()
	{
	}


	static const std::string VideoTextureIdPattern = "Internal/VideoTexture_{0}";
	static const std::string EmptyVideoTextureIdPattern = Wrench::StringUtils::Format(VideoTextureIdPattern, "Empty");


	E_RESULT_CODE CVideoProcessSystem::Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpFileSystem = pFileSystem;

		const TTexture2DParameters emptyVideoTextureParams{ 2, 2, FT_NORM_UBYTE4, 1, 1, 0 };
		mEmptyVideoTextureHandle = mpResourceManager->Create<ITexture2D>(EmptyVideoTextureIdPattern, emptyVideoTextureParams);

		mIsInitialized = true;

		return RC_OK;
	}

	void CVideoProcessSystem::InjectBindings(IWorld* pWorld)
	{
		mpVideoContainers.clear();
		mpVideoReceivers.clear();

		for (const TEntityId currEntityId : pWorld->FindEntitiesWithComponents<CUIVideoContainerComponent>())
		{
			CEntity* pEntity = pWorld->FindEntity(currEntityId);

			CUIVideoContainerComponent* pContainer = pEntity->GetComponent<CUIVideoContainerComponent>();

			mpVideoContainers.push_back(pContainer);
			mpVideoReceivers.push_back(pEntity->AddComponent<CImage>());

			CImage* pImage = mpVideoReceivers.back();
			if (pImage && !pContainer->mpInternalData->mpDecoder)
			{
				pImage->SetImageId(EmptyVideoTextureIdPattern);
				pImage->SetImageResourceId(mEmptyVideoTextureHandle);
			}
		}
	}

	void CVideoProcessSystem::Update(IWorld* pWorld, F32 dt)
	{	
		TDE2_PROFILER_SCOPE("CVideoProcessSystem::Update");

		CUIVideoContainerComponent* pVideoContainer = nullptr;
		CImage* pCurrImageSource = nullptr;
		
		THEORAPLAY_Decoder* pCurrVideoDecoder = nullptr;
		const THEORAPLAY_VideoFrame* pCurrVideoFrame = nullptr;

		TPtr<ITexture2D> pTexture = nullptr;

		for (USIZE i = 0; i < mpVideoContainers.size(); i++)
		{
			pVideoContainer = mpVideoContainers[i];
			pCurrImageSource = mpVideoReceivers[i];
			pCurrVideoDecoder = pVideoContainer->mpInternalData->mpDecoder;
			pCurrVideoFrame = pVideoContainer->mpInternalData->mpCurrVideoFrame;

			if (!pVideoContainer->mIsPlaying)
			{
				continue;
			}

			if (!pCurrVideoDecoder)
			{
				pCurrVideoDecoder = THEORAPLAY_startDecodeFile(
					mpFileSystem->ResolveVirtualPath(pVideoContainer->mVideoResourceId, false).c_str(),
					static_cast<U32>(pVideoContainer->mFPS), THEORAPLAY_VIDFMT_RGBA);

				pVideoContainer->mpInternalData->mpDecoder = pCurrVideoDecoder;
			}

			if (!THEORAPLAY_isInitialized(pCurrVideoDecoder) || !THEORAPLAY_isDecoding(pCurrVideoDecoder))
			{
				continue;
			}

			if (pVideoContainer->mStopPlayback) // \note Received signal to stop the playback
			{
				pVideoContainer->mStopPlayback = false;
				pVideoContainer->mIsPlaying = false;

				THEORAPLAY_freeVideo(pCurrVideoFrame);
				THEORAPLAY_stopDecode(pCurrVideoDecoder);

				pVideoContainer->mpInternalData->mpDecoder = nullptr;
				pVideoContainer->mpInternalData->mpCurrVideoFrame = nullptr;

				continue;
			}

			if (!pCurrVideoFrame)
			{
				pCurrVideoFrame = THEORAPLAY_getVideo(pCurrVideoDecoder);
				if (!pCurrVideoFrame)
				{
					if (!pVideoContainer->mIsLooped)
					{
						pVideoContainer->StopPlayback();
						continue;
					}

					if (!pVideoContainer->mpInternalData->mCurrVideoFrameIndex)
					{
						continue;
					}

					THEORAPLAY_freeVideo(pCurrVideoFrame);
					THEORAPLAY_stopDecode(pCurrVideoDecoder);

					pVideoContainer->mpInternalData->mpDecoder = nullptr;
					pVideoContainer->mpInternalData->mpCurrVideoFrame = nullptr;
					pVideoContainer->mpInternalData->mCurrVideoFrameIndex = 0;

					pVideoContainer->ResetState();

					continue;
				}
			}

			pVideoContainer->mpInternalData->mpCurrVideoFrame = pCurrVideoFrame;

			const U32 currTimeMs = static_cast<U32>(1000 * pVideoContainer->mCurrTime);
			const U32 frameTime = static_cast<U32>(1000 / std::min(pCurrVideoFrame->fps, static_cast<F64>(pVideoContainer->mFPS)));

			if (pCurrVideoFrame->playms > currTimeMs) 
			{
				pVideoContainer->mCurrTime += dt;
				continue;
			}

			if (frameTime && (currTimeMs - pCurrVideoFrame->playms) >= frameTime) // \note Skip frames if current time is greater than frame's one
			{
				const THEORAPLAY_VideoFrame* pPrevVideoFrame = pCurrVideoFrame;

				while (pCurrVideoFrame = THEORAPLAY_getVideo(pCurrVideoDecoder))
				{
					THEORAPLAY_freeVideo(pPrevVideoFrame);
					pPrevVideoFrame = pCurrVideoFrame;

					if ((currTimeMs - pCurrVideoFrame->playms) < frameTime)
					{
						break;
					}
				}

				if (!pCurrVideoFrame)
				{
					pCurrVideoFrame = pPrevVideoFrame;
				}

				pVideoContainer->mpInternalData->mpCurrVideoFrame = pCurrVideoFrame;
			}

			// \note Update the texture, create a new one if it doesn't exist
			TResourceId currVideoTextureHandle = pVideoContainer->mpInternalData->mVideoTextureHandle;
			if (TResourceId::Invalid == currVideoTextureHandle)
			{
				const TTexture2DParameters params{ pCurrVideoFrame->width, pCurrVideoFrame->height, FT_NORM_UBYTE4, 1, 1, 0 };
				const std::string textureId = Wrench::StringUtils::Format(VideoTextureIdPattern, i);

				currVideoTextureHandle = mpResourceManager->Create<ITexture2D>(textureId, params);

				TDE2_ASSERT(TResourceId::Invalid != currVideoTextureHandle);

				pVideoContainer->mpInternalData->mVideoTextureHandle = currVideoTextureHandle;
				
				pCurrImageSource->SetImageId(textureId);
				pCurrImageSource->SetImageResourceId(currVideoTextureHandle);
			}

			pTexture = mpResourceManager->GetResource<ITexture2D>(currVideoTextureHandle);
			if (!pTexture)
			{
				continue;
			}

			if (pTexture->GetWidth() != pCurrVideoFrame->width || pTexture->GetHeight() != pCurrVideoFrame->height)
			{
				pTexture->Resize(pCurrVideoFrame->width, pCurrVideoFrame->height);
			}

			pTexture->WriteData(TRectI32(0, 0, static_cast<I32>(pCurrVideoFrame->width), static_cast<I32>(pCurrVideoFrame->height)), pCurrVideoFrame->pixels);

			THEORAPLAY_freeVideo(pCurrVideoFrame);
			pVideoContainer->mpInternalData->mpCurrVideoFrame = nullptr;
			pVideoContainer->mpInternalData->mCurrVideoFrameIndex++;

			pVideoContainer->mCurrTime += dt;
		}
	}


	TDE2_API ISystem* CreateVideoProcessSystem(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CVideoProcessSystem, result, pResourceManager, pFileSystem);
	}
}