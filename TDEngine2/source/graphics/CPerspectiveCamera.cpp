#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/ecs/ICameraSystem.h"


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

		return _initInternal();
	}

	E_RESULT_CODE CPerspectiveCamera::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mFOV = pReader->GetFloat("fov");
		mAspectRatio = pReader->GetFloat("aspect");

		return RC_OK;
	}

	E_RESULT_CODE CPerspectiveCamera::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CPerspectiveCamera::GetTypeId()));

			pWriter->SetFloat("fov", mFOV);
			pWriter->SetFloat("aspect", mAspectRatio);
		}
		pWriter->EndGroup();

		return RC_OK;
	}
	
	E_RESULT_CODE CPerspectiveCamera::ComputeProjectionMatrix(const ICameraSystem* pCameraSystem)
	{
		if (!pCameraSystem)
		{
			return RC_INVALID_ARGS;
		}

		return pCameraSystem->ComputePerspectiveProjection(this);
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

	E_CAMERA_PROJECTION_TYPE CPerspectiveCamera::GetProjType() const
	{
		return E_CAMERA_PROJECTION_TYPE::PERSPECTIVE;
	}


	IComponent* CreatePerspectiveCamera(F32 fov, F32 aspect, F32 zn, F32 zf, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CPerspectiveCamera, result, fov, aspect, zn, zf);
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
		return CREATE_IMPL(IComponentFactory, CPerspectiveCameraFactory, result);
	}
}