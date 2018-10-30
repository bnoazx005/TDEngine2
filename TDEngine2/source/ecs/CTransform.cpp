#include "./../../include/ecs/CTransform.h"


namespace TDEngine2
{
	CTransform::CTransform() :
		CBaseComponent()
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
	}

	void CTransform::SetPosition(const TVector3& position)
	{
		mPosition = position;
	}

	void CTransform::SetRotation(const TVector3& eulerAngles)
	{
		mRotation = TQuaternion(eulerAngles);
	}

	void CTransform::SetRotation(const TQuaternion& q)
	{
		mRotation = q;
	}

	void CTransform::SetScale(const TVector3& scale)
	{
		mScale = scale;
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
}