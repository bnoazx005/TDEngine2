#include "./../../include/editor/CSelectionManager.h"
#include "./../../include/editor/IEditorsManager.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CObjectsSelectionSystem.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/graphics/CBaseRenderTarget.h"
#include "./../../include/graphics/CBaseTexture2D.h"
#include "./../../include/core/IWindowSystem.h"
#include "./../../include/core/IGraphicsContext.h"
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CSelectionManager::CSelectionManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CSelectionManager::Init(IResourceManager* pResourceManager, IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, IEditorsManager* pEditorsManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorsManager || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpEditorsManager  = pEditorsManager;
		mpWindowSystem    = pWindowSystem;
		mpGraphicsContext = pGraphicsContext;

		mpSelectionGeometryBuffer = nullptr;

		E_RESULT_CODE result = _createRenderTarget(mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight());
		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		E_RESULT_CODE result = RC_OK;

		result = result | mpSelectionGeometryBuffer->Free();

		mIsInitialized = false;
		delete this;

		return result;
	}

	E_RESULT_CODE CSelectionManager::BuildSelectionMap(const TRenderFrameCallback& onDrawVisibleObjectsCallback)
	{
		IRenderTarget* pCurrRenderTarget   = mpSelectionGeometryBuffer->Get<IRenderTarget>(RAT_BLOCKING);
		ITexture2D* pReadableRTCopyTexture = mpReadableSelectionBuffer->Get<ITexture2D>(RAT_BLOCKING);

		mpGraphicsContext->BindRenderTarget(pCurrRenderTarget);

		if (onDrawVisibleObjectsCallback)
		{
			onDrawVisibleObjectsCallback();
		}

		mpGraphicsContext->BindRenderTarget(nullptr);

		return pCurrRenderTarget->Blit(pReadableRTCopyTexture);
	}

	E_RESULT_CODE CSelectionManager::OnEvent(const TBaseEvent* pEvent)
	{
		TypeId eventType = pEvent->GetEventType();

		static const std::unordered_map<TypeId, std::function<E_RESULT_CODE()>> handlers
		{
			{
				TOnEditorModeEnabled::GetTypeId(), [this, pEvent]
				{
					return mpWorld->ActivateSystem(mObjectSelectionSystemId);
				}
			},
			{
				TOnEditorModeDisabled::GetTypeId(), [this, pEvent]
				{
					return mpWorld->DeactivateSystem(mObjectSelectionSystemId);
				}
			},
			{
				TOnWindowResized::GetTypeId(), [this, pEvent]
				{
					if (const TOnWindowResized* pOnResizedEvent = dynamic_cast<const TOnWindowResized*>(pEvent))
					{
						return _createRenderTarget(pOnResizedEvent->mWidth, pOnResizedEvent->mHeight);
					}

					return RC_FAIL;
				}
			}
		};

		auto iter = handlers.cbegin();

		if ((iter = handlers.find(eventType)) != handlers.cend())
		{
			return iter->second();
		}

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::SetWorldInstance(IWorld* pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;

		if ((mObjectSelectionSystemId = mpWorld->FindSystem<CObjectsSelectionSystem>()) == InvalidSystemId)
		{
			LOG_ERROR("[CSelectionManager] \"ObjectsSelection\" system wasn't found");
			return RC_FAIL;
		}

		return mpWorld->DeactivateSystem(mObjectSelectionSystemId);
	}

	TEventListenerId CSelectionManager::GetListenerId() const
	{
		return GetTypeId();
	}

	E_RESULT_CODE CSelectionManager::_createRenderTarget(U32 width, U32 height)
	{
		E_RESULT_CODE result = RC_OK;

		if (mpSelectionGeometryBuffer)
		{
			if (CBaseRenderTarget* pRenderTarget = mpSelectionGeometryBuffer->Get<CBaseRenderTarget>(RAT_BLOCKING))
			{
				// \note Should recreate the render target
				if (pRenderTarget->GetWidth() != width || pRenderTarget->GetHeight() != height)
				{
					if ((result = mpSelectionGeometryBuffer->Free()) != RC_OK)
					{
						return result;
					}

					if ((result = mpReadableSelectionBuffer->Free()) != RC_OK)
					{
						return result;
					}

					mpSelectionGeometryBuffer = nullptr;
				}				
			}
		}

		if (!mpSelectionGeometryBuffer)
		{
			TTexture2DParameters renderTargetParams { width, height, FT_UINT1, 1, 1, 0 };

			mpSelectionGeometryBuffer = mpResourceManager->Create<CBaseRenderTarget>("SelectionBuffer", renderTargetParams);
			mpReadableSelectionBuffer = mpResourceManager->Create<CBaseTexture2D>("SelectionBufferCPUCopy", renderTargetParams);
		}

		return RC_OK;
	}


	TDE2_API ISelectionManager* CreateSelectionManager(IResourceManager* pResourceManager, IWindowSystem* pWindowSystem, IGraphicsContext* pGraphicsContext, 
													   IEditorsManager* pEditorsManager, E_RESULT_CODE& result)
	{
		CSelectionManager* pSelectionManagerInstance = new (std::nothrow) CSelectionManager();

		if (!pSelectionManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pSelectionManagerInstance->Init(pResourceManager, pWindowSystem, pGraphicsContext, pEditorsManager);

		if (result != RC_OK)
		{
			delete pSelectionManagerInstance;

			pSelectionManagerInstance = nullptr;
		}

		return dynamic_cast<ISelectionManager*>(pSelectionManagerInstance);
	}
}

#endif