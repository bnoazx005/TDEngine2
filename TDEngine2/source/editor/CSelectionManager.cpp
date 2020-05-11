#include "./../../include/editor/CSelectionManager.h"
#include "./../../include/editor/IEditorsManager.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CObjectsSelectionSystem.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/editor/ecs/EditorComponents.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IResourceHandler.h"
#include "./../../include/core/IEventManager.h"
#include "./../../include/graphics/CBaseRenderTarget.h"
#include "./../../include/graphics/CBaseTexture2D.h"
#include "./../../include/core/IWindowSystem.h"
#include "./../../include/core/IGraphicsContext.h"
#include <functional>
#include <cmath>


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

		if (!pEditorsManager || !pResourceManager || !pWindowSystem || !pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpEditorsManager  = pEditorsManager;
		mpWindowSystem    = pWindowSystem;
		mpGraphicsContext = pGraphicsContext;
		mpEventManager    = pWindowSystem->GetEventManager();

		mpSelectionGeometryBuffer = nullptr;
		
		E_RESULT_CODE result = _createRenderTarget(mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight());
		if (result != RC_OK)
		{
			return result;
		}

		mLastSelectedEntityID = TEntityId::Invalid;

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

		mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget);

		mpGraphicsContext->ClearDepthBuffer(1.0f);
		mpGraphicsContext->ClearRenderTarget(pCurrRenderTarget, TColor32F(0.0f, 0.0f, 0.0f, 0.0f));

		if (onDrawVisibleObjectsCallback)
		{
			onDrawVisibleObjectsCallback();
		}

		mpGraphicsContext->BindRenderTarget(0, nullptr);

		return pCurrRenderTarget->Blit(pReadableRTCopyTexture);
	}

	TEntityId CSelectionManager::PickObject(const TVector2& position)
	{
		I32 x = std::lround(position.x);
		I32 y = std::lround(position.y) + mWindowHeaderHeight;

		// \note reset selection
		if (mLastSelectedEntityID != TEntityId::Invalid)
		{
			if (CEntity* pSelectedEntity = mpWorld->FindEntity(mLastSelectedEntityID))
			{
				PANIC_ON_FAILURE(pSelectedEntity->RemoveComponent<CSelectedEntityComponent>());
			}
		}

		if (auto pSelectionTexture = mpReadableSelectionBuffer->Get<ITexture2D>(RAT_BLOCKING))
		{
			y = pSelectionTexture->GetHeight() - y;

			auto&& selectionMapData = pSelectionTexture->GetInternalData();
			
			// \note multiply by 4, where 4 is a size of a single pixel data
			U32* pPixelData = reinterpret_cast<U32*>(&selectionMapData[(y * pSelectionTexture->GetWidth() + x) << 2]);

			mLastSelectedEntityID = TEntityId(static_cast<U32>(*pPixelData) - 1);
			
			// \note mark the entity as selected via adding CSelectedEntityComponent
			if (CEntity* pSelectedEntity = mpWorld->FindEntity(mLastSelectedEntityID))
			{
				auto pSelectedEntityComponent = pSelectedEntity->AddComponent<CSelectedEntityComponent>();
				TDE2_ASSERT(pSelectedEntityComponent);

				TOnObjectSelected onObjectSelected;
				onObjectSelected.mObjectID = mLastSelectedEntityID;
				onObjectSelected.mpWorld   = mpWorld;

				mpEventManager->Notify(&onObjectSelected);
			}

			return mLastSelectedEntityID;
		}
		
		return (mLastSelectedEntityID = TEntityId::Invalid);
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
		return TEventListenerId(GetTypeId());
	}

	TEntityId CSelectionManager::GetSelectedEntityId() const
	{
		return mLastSelectedEntityID;
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

		mWindowHeaderHeight = height - mpWindowSystem->GetClientRect().height;

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