#include "./../../include/graphics/COrthoCamera.h"
#include "./../../include/ecs/ICameraSystem.h"


namespace TDEngine2
{
	COrthoCamera::COrthoCamera() :
		CBaseCamera()
	{
	}

	E_RESULT_CODE COrthoCamera::Init(F32 width, F32 height, F32 zn, F32 zf)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mWidth  = width;
		mHeight = height;
		mZNear  = zn;
		mZFar   = zf;

		mIsInitialized = true;

		return _initInternal();
	}
	
	E_RESULT_CODE COrthoCamera::ComputeProjectionMatrix(const ICameraSystem* pCameraSystem)
	{
		if (!pCameraSystem)
		{
			return RC_INVALID_ARGS;
		}

		return pCameraSystem->ComputeOrthographicProjection(this);
	}

	void COrthoCamera::SetWidth(F32 width)
	{
		mWidth = width;
	}

	void COrthoCamera::SetHeight(F32 height)
	{
		mHeight = height;
	}

	F32 COrthoCamera::GetWidth() const
	{
		return mWidth;
	}

	F32 COrthoCamera::GetHeight() const
	{
		return mHeight;
	}

	E_CAMERA_PROJECTION_TYPE COrthoCamera::GetProjType() const
	{
		return E_CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC;
	}


	IComponent* CreateOrthoCamera(F32 fov, F32 aspect, F32 zn, F32 zf, E_RESULT_CODE& result)
	{
		COrthoCamera* pCameraInstance = new (std::nothrow) COrthoCamera();

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


	COrthoCameraFactory::COrthoCameraFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE COrthoCameraFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE COrthoCameraFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* COrthoCameraFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TOrthoCameraParameters* cameraParams = static_cast<const TOrthoCameraParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateOrthoCamera(cameraParams->mViewportWidth, cameraParams->mViewportHeight, cameraParams->mZNear, cameraParams->mZFar, result);
	}

	IComponent* COrthoCameraFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		/// \todo refactor this fragment
		return CreateOrthoCamera(8.0f, 5.0f, 1.0f, 1000.0f, result);
	}

	TypeId COrthoCameraFactory::GetComponentTypeId() const
	{
		return COrthoCamera::GetTypeId();
	}


	IComponentFactory* CreateOrthoCameraFactory(E_RESULT_CODE& result)
	{
		COrthoCameraFactory* pOrthoCameraFactoryInstance = new (std::nothrow) COrthoCameraFactory();

		if (!pOrthoCameraFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pOrthoCameraFactoryInstance->Init();

		if (result != RC_OK)
		{
			delete pOrthoCameraFactoryInstance;

			pOrthoCameraFactoryInstance = nullptr;
		}

		return pOrthoCameraFactoryInstance;
	}
}