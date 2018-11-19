#include "./../../include/ecs/CTransform.h"


namespace TDEngine2
{
	CTransform::CTransform() :
		CBaseComponent(), mTransform(IdentityMatrix4), mHasChanged(true)
	{
	}

	E_RESULT_CODE CTransform::Init(const TVector3& position, const TQuaternion& rotation, const TVector3& scale)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mPosition = position;

		mRotation = rotation;

		mScale = scale;
		
		mIsInitialized = true;

		return RC_OK;
	}

	void CTransform::Reset()
	{
		mPosition = ZeroVector3;

		mRotation = UnitQuaternion;

		mScale = TVector3(1.0f, 1.0f, 1.0f);

		mHasChanged = true;
	}

	void CTransform::SetPosition(const TVector3& position)
	{
		mPosition = position;

		mHasChanged = true;
	}

	void CTransform::SetRotation(const TVector3& eulerAngles)
	{
		mRotation = TQuaternion(eulerAngles);

		mHasChanged = true;
	}

	void CTransform::SetRotation(const TQuaternion& q)
	{
		mRotation = q;

		mHasChanged = true;
	}

	void CTransform::SetScale(const TVector3& scale)
	{
		mScale = scale;

		mHasChanged = true;
	}

	const TVector3& CTransform::GetPosition() const
	{
		return mPosition;
	}

	const TQuaternion& CTransform::GetRotation() const
	{
		return mRotation;
	}

	const TVector3& CTransform::GetScale() const
	{
		return mScale;
	}
	
	bool CTransform::HasChanged() const
	{
		return mHasChanged;
	}


	IComponent* CreateTransform(const TVector3& position, const TQuaternion& rotation, const TVector3& scale, E_RESULT_CODE& result)
	{
		CTransform* pTransformInstance = new (std::nothrow) CTransform();

		if (!pTransformInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTransformInstance->Init(position, rotation, scale);

		if (result != RC_OK)
		{
			delete pTransformInstance;

			pTransformInstance = nullptr;
		}

		return pTransformInstance;
	}


	CTransformFactory::CTransformFactory():
		CBaseObject()
	{
	}

	E_RESULT_CODE CTransformFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTransformFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CTransformFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TTransformParameters* transformParams = static_cast<const TTransformParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateTransform(transformParams->mPosition, transformParams->mRotation, transformParams->mScale, result);
	}

	IComponent* CTransformFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateTransform(ZeroVector3, UnitQuaternion, TVector3(1.0f, 1.0f, 1.0f), result);
	}

	TypeId CTransformFactory::GetComponentTypeId() const
	{
		return CTransform::GetTypeId();
	}


	IComponentFactory* CreateTransformFactory(E_RESULT_CODE& result)
	{
		CTransformFactory* pTransformFactoryInstance = new (std::nothrow) CTransformFactory();

		if (!pTransformFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTransformFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pTransformFactoryInstance;

			pTransformFactoryInstance = nullptr;
		}

		return pTransformFactoryInstance;
	}
}