#include "../../include/editor/CSelectionManager.h"
#include "../../include/editor/IEditorsManager.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CObjectsSelectionSystem.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IEventManager.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IGraphicsContext.h"
#include <functional>
#include <cmath>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CSelectionManager::CSelectionManager():
		CBaseObject()
	{
	}

	E_RESULT_CODE CSelectionManager::Init(TPtr<IResourceManager> pResourceManager, TPtr<IWindowSystem> pWindowSystem, TPtr<IGraphicsContext> pGraphicsContext, IEditorsManager* pEditorsManager)
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

		mSelectionGeometryBufferHandle = TResourceId::Invalid;
		
		E_RESULT_CODE result = _createRenderTarget(mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight());
		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::_onFreeInternal()
	{
		return mpResourceManager->ReleaseResource(mSelectionGeometryBufferHandle);
	}

	E_RESULT_CODE CSelectionManager::BuildSelectionMap(const TRenderFrameCallback& onDrawVisibleObjectsCallback)
	{
		if (!mpEditorsManager->IsEditorModeEnabled())
		{
			return RC_OK;
		}

		TPtr<IRenderTarget> pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mSelectionGeometryBufferHandle);

		mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget.Get());

		mpGraphicsContext->ClearDepthBuffer(1.0f);
		mpGraphicsContext->ClearRenderTarget(pCurrRenderTarget.Get(), TColor32F(0.0f, 0.0f, 0.0f, 0.0f));

		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pCurrRenderTarget->GetWidth()), static_cast<F32>(pCurrRenderTarget->GetHeight()), 0.0f, 1.0f);

		if (onDrawVisibleObjectsCallback)
		{
			onDrawVisibleObjectsCallback();
		}

		ITexture2D* pReadableRTCopyTexture = mpResourceManager->GetResource<ITexture2D>(mReadableSelectionBufferHandle).Get();
		pCurrRenderTarget->Blit(pReadableRTCopyTexture);

		mpGraphicsContext->BindRenderTarget(0, nullptr);

		return RC_OK;
	}

	TEntityId CSelectionManager::PickObject(const TVector2& position)
	{
		I32 x = std::lround(position.x);
		I32 y = std::lround(position.y) + mWindowHeaderHeight;

		auto pSelectionTexture = mpResourceManager->GetResource<ITexture2D>(mReadableSelectionBufferHandle);
		if (pSelectionTexture)
		{
			if (!mpGraphicsContext->GetContextInfo().mIsTextureYCoordInverted)
			{
				y = pSelectionTexture->GetHeight() - y;
			}

			auto&& selectionMapData = pSelectionTexture->GetInternalData();
			
			// \note multiply by 4, where 4 is a size of a single pixel data
			U32* pPixelData = reinterpret_cast<U32*>(&selectionMapData[(y * pSelectionTexture->GetWidth() + x) << 2]);

			SetSelectedEntity(TEntityId(static_cast<U32>(*pPixelData) - 1));

			return GetSelectedEntityId();
		}
		
		SetSelectedEntity(TEntityId::Invalid);
		return TEntityId::Invalid;
	}

	E_RESULT_CODE CSelectionManager::OnEvent(const TBaseEvent* pEvent)
	{
		TypeId eventType = pEvent->GetEventType();

		static const std::unordered_map<TypeId, std::function<E_RESULT_CODE(const TBaseEvent*)>> handlers
		{
			{
				TOnEditorModeEnabled::GetTypeId(), [this](const TBaseEvent* pEventData)
				{
					return mpWorld->ActivateSystem(mObjectSelectionSystemId);
				}
			},
			{
				TOnEditorModeDisabled::GetTypeId(), [this](const TBaseEvent* pEventData)
				{
					return mpWorld->DeactivateSystem(mObjectSelectionSystemId);
				}
			},
			{
				TOnWindowResized::GetTypeId(), [this](const TBaseEvent* pEventData)
				{
					if (const TOnWindowResized* pOnResizedEvent = dynamic_cast<const TOnWindowResized*>(pEventData))
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
			return iter->second(pEvent);
		}

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::SetWorldInstance(TPtr<IWorld> pWorld)
	{
		if (!pWorld)
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;

		if ((mObjectSelectionSystemId = mpWorld->FindSystem<CObjectsSelectionSystem>()) == TSystemId::Invalid)
		{
			LOG_ERROR("[CSelectionManager] \"ObjectsSelection\" system wasn't found");
			return RC_FAIL;
		}

		return mpWorld->DeactivateSystem(mObjectSelectionSystemId);
	}

	E_RESULT_CODE CSelectionManager::SetSelectedEntity(TEntityId id)
	{
		return _setSelection(id);
	}

	E_RESULT_CODE CSelectionManager::AddSelectedEntity(TEntityId id)
	{
		return _setSelection(id, false);
	}

	E_RESULT_CODE CSelectionManager::ClearSelection()
	{
		_resetCurrentSelection();
		return RC_OK;
	}

	TEventListenerId CSelectionManager::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TEntityId CSelectionManager::GetSelectedEntityId() const
	{
		return mSelectedEntities.empty() ? TEntityId::Invalid : mSelectedEntities.back();
	}

	const std::vector<TEntityId>& CSelectionManager::GetSelectedEntities() const
	{
		return mSelectedEntities;
	}

	bool CSelectionManager::IsEntityBeingSelected(TEntityId id) const
	{
		return std::find(mSelectedEntities.cbegin(), mSelectedEntities.cend(), id) != mSelectedEntities.cend();
	}

	E_RESULT_CODE CSelectionManager::_createRenderTarget(U32 width, U32 height)
	{
		E_RESULT_CODE result = RC_OK;

		if (TResourceId::Invalid != mSelectionGeometryBufferHandle)
		{
			auto pRenderTarget = mpResourceManager->GetResource<CBaseRenderTarget>(mSelectionGeometryBufferHandle);
			if (pRenderTarget)
			{
				// \note Should recreate the render target
				if (pRenderTarget->GetWidth() != width || pRenderTarget->GetHeight() != height)
				{
					result = result | mpResourceManager->ReleaseResource(mSelectionGeometryBufferHandle);
					result = result | mpResourceManager->ReleaseResource(mReadableSelectionBufferHandle);

					mSelectionGeometryBufferHandle = TResourceId::Invalid;
					mReadableSelectionBufferHandle = TResourceId::Invalid;
				}			
			}
		}

		if (TResourceId::Invalid == mSelectionGeometryBufferHandle)
		{
			TTexture2DParameters renderTargetParams { width, height, FT_UINT1, 1, 1, 0 };

			mSelectionGeometryBufferHandle = mpResourceManager->Create<IRenderTarget>("SelectionBuffer", renderTargetParams);
			mReadableSelectionBufferHandle = mpResourceManager->Create<ITexture2D>("SelectionBufferCPUCopy", renderTargetParams);
		}

		mWindowHeaderHeight = height - mpWindowSystem->GetClientRect().height;

		return RC_OK;
	}

	E_RESULT_CODE CSelectionManager::_setSelection(TEntityId id, bool resetSelection)
	{
		if (resetSelection)
		{
			_resetCurrentSelection();
		}

		mSelectedEntities.push_back(id);

		// \note mark the entity as selected via adding CSelectedEntityComponent
		if (CEntity* pSelectedEntity = mpWorld->FindEntity(id))
		{
			auto pSelectedEntityComponent = pSelectedEntity->AddComponent<CSelectedEntityComponent>();
			TDE2_ASSERT(pSelectedEntityComponent);

			TOnObjectSelected onObjectSelected;
			onObjectSelected.mObjectID = id;
			onObjectSelected.mpWorld = mpWorld.Get();

			mpEventManager->Notify(&onObjectSelected);
		}

		return RC_OK;
	}

	void CSelectionManager::_resetCurrentSelection()
	{
		if (mSelectedEntities.empty() || mSelectedEntities.back() == TEntityId::Invalid)
		{
			return;
		}

		for (TEntityId currSelectedEntity : mSelectedEntities)
		{
			if (CEntity* pSelectedEntity = mpWorld->FindEntity(currSelectedEntity))
			{
				PANIC_ON_FAILURE(pSelectedEntity->RemoveComponent<CSelectedEntityComponent>());
			}
		}

		mSelectedEntities.clear();
	}


	TDE2_API ISelectionManager* CreateSelectionManager(TPtr<IResourceManager> pResourceManager, TPtr<IWindowSystem> pWindowSystem, TPtr<IGraphicsContext> pGraphicsContext,
														IEditorsManager* pEditorsManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISelectionManager, CSelectionManager, result, pResourceManager, pWindowSystem, pGraphicsContext, pEditorsManager);
	}
}

#endif