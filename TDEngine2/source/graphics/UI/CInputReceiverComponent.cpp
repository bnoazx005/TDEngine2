#include "../../../include/graphics/UI/CInputReceiverComponent.h"


namespace TDEngine2
{
	struct TInputReceiverArchiveKeys
	{
		static const std::string mIgnoreInputKeyId;
	};


	const std::string TInputReceiverArchiveKeys::mIgnoreInputKeyId = "ignore_input";


	CInputReceiver::CInputReceiver() :
		CBaseComponent(), mIsClicked(false), mIsIgnoreInput(false)
	{
	}

	E_RESULT_CODE CInputReceiver::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mIsIgnoreInput = pReader->GetBool(TInputReceiverArchiveKeys::mIgnoreInputKeyId);
		
		return RC_OK;
	}

	E_RESULT_CODE CInputReceiver::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CInputReceiver::GetTypeId()));

			pWriter->SetBool(TInputReceiverArchiveKeys::mIgnoreInputKeyId, mIsIgnoreInput);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	void CInputReceiver::SetPressedFlag(bool value)
	{
		mIsClicked = value;
	}

	void CInputReceiver::SetIgnoreInputFlag(bool value)
	{
		mIsIgnoreInput = value;
	}

	bool CInputReceiver::IsPressed() const
	{
		return mIsClicked;
	}

	bool CInputReceiver::IsIgnoreInputFlag() const
	{
		return mIsIgnoreInput;
	}


	IComponent* CreateInputReceiver(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CInputReceiver, result);
	}


	/*!
		\brief CInputReceiverFactory's definition
	*/

	CInputReceiverFactory::CInputReceiverFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CInputReceiverFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateInputReceiver(result);
	}

	E_RESULT_CODE CInputReceiverFactory::SetupComponent(CInputReceiver* pComponent, const TInputReceiverParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateInputReceiverFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CInputReceiverFactory, result);
	}
}