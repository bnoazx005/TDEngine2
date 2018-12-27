#include "./../../include/graphics/CQuadSprite.h"


namespace TDEngine2
{
	CQuadSprite::CQuadSprite() :
		CBaseComponent(), mMaterialName()
	{
	}

	E_RESULT_CODE CQuadSprite::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CQuadSprite::SetMaterialName(const std::string& materialName)
	{
		mMaterialName = materialName;
	}

	const std::string& CQuadSprite::GetMaterialName() const
	{
		return mMaterialName;
	}


	IComponent* CreateQuadSprite(E_RESULT_CODE& result)
	{
		CQuadSprite* pQuadSpriteInstance = new (std::nothrow) CQuadSprite();

		if (!pQuadSpriteInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pQuadSpriteInstance->Init();

		if (result != RC_OK)
		{
			delete pQuadSpriteInstance;

			pQuadSpriteInstance = nullptr;
		}

		return pQuadSpriteInstance;
	}


	CQuadSpriteFactory::CQuadSpriteFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CQuadSpriteFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CQuadSpriteFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CQuadSpriteFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TQuadSpriteParameters* transformParams = static_cast<const TQuadSpriteParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateQuadSprite(result);
	}

	IComponent* CQuadSpriteFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateQuadSprite(result);
	}

	TypeId CQuadSpriteFactory::GetComponentTypeId() const
	{
		return CQuadSprite::GetTypeId();
	}


	IComponentFactory* CreateQuadSpriteFactory(E_RESULT_CODE& result)
	{
		CQuadSpriteFactory* pQuadSpriteFactoryInstance = new (std::nothrow) CQuadSpriteFactory();

		if (!pQuadSpriteFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pQuadSpriteFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pQuadSpriteFactoryInstance;

			pQuadSpriteFactoryInstance = nullptr;
		}

		return pQuadSpriteFactoryInstance;
	}
}