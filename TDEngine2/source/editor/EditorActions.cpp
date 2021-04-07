#include "../../include/editor/EditorActions.h"
#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CUndoEditorAction::CUndoEditorAction() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CUndoEditorAction::Init(CLevelEditorWindow* pEditorWindow)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorWindow)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorWindow = pEditorWindow;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUndoEditorAction::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CUndoEditorAction::Execute()
	{
		return mpEditorWindow->ExecuteUndoAction();
	}

	E_RESULT_CODE CUndoEditorAction::Restore()
	{
		return mpEditorWindow->ExecuteRedoAction();
	}

	std::string CUndoEditorAction::ToString() const
	{
		return "CUndoEditorAction";
	}


	TDE2_API IEditorAction* CreateUndoAction(CLevelEditorWindow* pEditorWindow, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorAction, CUndoEditorAction, result, pEditorWindow);
	}


	/*!
		\note CRedoEditorAction's definition
	*/

	CRedoEditorAction::CRedoEditorAction() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CRedoEditorAction::Init(CLevelEditorWindow* pEditorWindow)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorWindow)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorWindow = pEditorWindow;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CRedoEditorAction::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CRedoEditorAction::Execute()
	{
		return mpEditorWindow->ExecuteRedoAction();
	}

	E_RESULT_CODE CRedoEditorAction::Restore()
	{
		return mpEditorWindow->ExecuteUndoAction();
	}

	std::string CRedoEditorAction::ToString() const
	{
		return "CRedoEditorAction";
	}


	TDE2_API IEditorAction* CreateRedoAction(CLevelEditorWindow* pEditorWindow, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorAction, CRedoEditorAction, result, pEditorWindow);
	}


	/*!
		\note CTransformObjectAction's definition
	*/

	CTransformObjectAction::CTransformObjectAction() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTransformObjectAction::Init(IWorld* pWorld, TEntityId entityId, const TSRTEntity& transform)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pWorld || (entityId == TEntityId::Invalid))
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;

		mEntityId = entityId;

		mTransform = transform;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTransformObjectAction::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CTransformObjectAction::Execute()
	{
		return _changeTransform();
	}

	E_RESULT_CODE CTransformObjectAction::Restore()
	{
		return _changeTransform();
	}

	std::string CTransformObjectAction::ToString() const
	{
		return "CTransformObjectAction";
	}

	E_RESULT_CODE CTransformObjectAction::_changeTransform()
	{
		if (auto pEntity = mpWorld->FindEntity(mEntityId))
		{
			if (auto pTransform = pEntity->GetComponent<CTransform>())
			{
				TSRTEntity transform = mTransform;

				mTransform.mPosition = pTransform->GetPosition();
				mTransform.mRotation = pTransform->GetRotation();
				mTransform.mScale    = pTransform->GetScale();

				pTransform->SetPosition(transform.mPosition);
				pTransform->SetRotation(transform.mRotation);
				pTransform->SetScale(transform.mScale);
			}

			return RC_OK;
		}

		return RC_FAIL;
	}


	TDE2_API IEditorAction* CreateTransformObjectAction(IWorld* pWorld, TEntityId entityId, const TSRTEntity& transform, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorAction, CTransformObjectAction, result, pWorld, entityId, transform);
	}


	/*!
		\note CCommandAction's definition
	*/

	CCommandAction::CCommandAction() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CCommandAction::Init(const TCommandFunctor& doAction, const TCommandFunctor& undoAction)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!doAction || !undoAction)
		{
			return RC_INVALID_ARGS;
		}

		mDoActionCallback = doAction;
		mUndoActionCallback = undoAction;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CCommandAction::Free()
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

	E_RESULT_CODE CCommandAction::Execute()
	{
		if (mDoActionCallback)
		{
			mDoActionCallback();
		}

		return RC_OK;
	}

	E_RESULT_CODE CCommandAction::Restore()
	{
		if (mUndoActionCallback)
		{
			mUndoActionCallback();
		}

		return RC_OK;
	}

	std::string CCommandAction::ToString() const
	{
		return "CCommandAction";
	}


	TDE2_API IEditorAction* CreateCommandAction(const CCommandAction::TCommandFunctor& doAction, const CCommandAction::TCommandFunctor& undoAction, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorAction, CCommandAction, result, doAction, undoAction);
	}
}

#endif