#include "../../../include/graphics/UI/CInputReceiverComponent.h"


namespace TDEngine2
{
	struct TInputReceiverArchiveKeys
	{
	};


	CInputReceiver::CInputReceiver() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CInputReceiver::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsClicked = false;
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CInputReceiver::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CInputReceiver::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	void CInputReceiver::SetPressedFlag(bool value)
	{
		mIsClicked = value;
	}

	bool CInputReceiver::IsPressed() const
	{
		return mIsClicked;
	}


	IComponent* CreateInputReceiver(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CInputReceiver, result);
	}


	CInputReceiverFactory::CInputReceiverFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CInputReceiverFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CInputReceiverFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CInputReceiverFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TInputReceiverParameters* params = static_cast<const TInputReceiverParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateInputReceiver(result);
	}

	IComponent* CInputReceiverFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateInputReceiver(result);
	}

	TypeId CInputReceiverFactory::GetComponentTypeId() const
	{
		return CInputReceiver::GetTypeId();
	}


	IComponentFactory* CreateInputReceiverFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CInputReceiverFactory, result);
	}
}