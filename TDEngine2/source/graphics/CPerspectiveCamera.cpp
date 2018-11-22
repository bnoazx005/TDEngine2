#include "./../../include/graphics/CPerspectiveCamera.h"


namespace TDEngine2
{
	CPerspectiveCamera::CPerspectiveCamera():
		CBaseCamera()
	{
	}

	E_RESULT_CODE CPerspectiveCamera::Init(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mFOV         = fov;
		mAspectRatio = aspect;
		mZNear       = zn;
		mZFar        = zf;

		mIsInitialized = true;

		return RC_OK;
	}

	void CPerspectiveCamera::SetFOV(F32 fov)
	{
		mFOV = fov;
	}

	void CPerspectiveCamera::SetAspect(F32 aspect)
	{
		mAspectRatio = aspect;
	}

	F32 CPerspectiveCamera::GetFOV() const
	{
		return mFOV;
	}

	F32 CPerspectiveCamera::GetAspect() const
	{
		return mAspectRatio;
	}


	IComponent* CreatePerspectiveCamera(F32 fov, F32 aspect, F32 zn, F32 zf, E_RESULT_CODE& result)
	{
		CPerspectiveCamera* pCameraInstance = new (std::nothrow) CPerspectiveCamera();

		if (!pCameraInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCameraInstance->Init(fov, aspect, zn, zf);

		if (result != RC_OK)
		{
			delete pCameraInstance;

			pCameraInstance = nullptr;
		}

		return pCameraInstance;
	}


	CPerspectiveCameraFactory::CPerspectiveCameraFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CPerspectiveCameraFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CPerspectiveCameraFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CPerspectiveCameraFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TPerspectiveCameraParameters* cameraParams = static_cast<const TPerspectiveCameraParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreatePerspectiveCamera(cameraParams->mFOV, cameraParams->mAspect, cameraParams->mZNear, cameraParams->mZFar, result);
	}

	IComponent* CPerspectiveCameraFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;
		
		/// \todo replace 3.14 with Pi contants in MathUtils.h
		return CreatePerspectiveCamera(3.14f * 0.5f, 1.0f, 1.0f, 1000.0f, result);
	}

	TypeId CPerspectiveCameraFactory::GetComponentTypeId() const
	{
		return CPerspectiveCamera::GetTypeId();
	}


	IComponentFactory* CreatePerspectiveCameraFactory(E_RESULT_CODE& result)
	{
		CPerspectiveCameraFactory* pPerspectiveCameraFactoryInstance = new (std::nothrow) CPerspectiveCameraFactory();

		if (!pPerspectiveCameraFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pPerspectiveCameraFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pPerspectiveCameraFactoryInstance;

			pPerspectiveCameraFactoryInstance = nullptr;
		}

		return pPerspectiveCameraFactoryInstance;
	}
}