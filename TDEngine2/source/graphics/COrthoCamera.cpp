#include "../../include/graphics/COrthoCamera.h"
#include "../../include/ecs/ICameraSystem.h"


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

	E_RESULT_CODE COrthoCamera::Load(IArchiveReader* pReader)
	{
		return CBaseComponent::Load(pReader);
	}

	E_RESULT_CODE COrthoCamera::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(COrthoCamera::GetTypeId()));

			pWriter->SetFloat("width", mWidth);
			pWriter->SetFloat("height", mHeight);
		}
		pWriter->EndGroup();

		return RC_OK;
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
		return CREATE_IMPL(IComponent, COrthoCamera, result, fov, aspect, zn, zf);
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
		return CREATE_IMPL(IComponentFactory, COrthoCameraFactory, result);
	}
}