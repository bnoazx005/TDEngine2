#include "../../include/ecs/CUIElementsRenderSystem.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/UI/CCanvasComponent.h"
#include "../../include/graphics/UI/CLayoutElementComponent.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/IRenderer.h"


namespace TDEngine2
{
	CUIElementsRenderSystem::CUIElementsRenderSystem() :
		CBaseSystem()
	{
	}

	E_RESULT_CODE CUIElementsRenderSystem::Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pRenderer || !pGraphicsObjectManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpUIElementsRenderGroup = pRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SPRITES);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUIElementsRenderSystem::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	void CUIElementsRenderSystem::InjectBindings(IWorld* pWorld)
	{
		
	}

	void CUIElementsRenderSystem::Update(IWorld* pWorld, F32 dt)
	{
		
	}


	TDE2_API ISystem* CreateUIElementsRenderSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ISystem, CUIElementsRenderSystem, result, pRenderer, pGraphicsObjectManager);
	}
}