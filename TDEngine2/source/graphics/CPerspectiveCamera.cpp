#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/ecs/ICameraSystem.h"


namespace TDEngine2
{
	CPerspectiveCamera::CPerspectiveCamera():
		CBaseCamera()
	{
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


	IComponent* CreatePerspectiveCamera(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CPerspectiveCamera, result);
	}


	/*!
		\brief CPerspectiveCameraFactory's definition
	*/

	CPerspectiveCameraFactory::CPerspectiveCameraFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CPerspectiveCameraFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreatePerspectiveCamera(result);
	}

	E_RESULT_CODE CPerspectiveCameraFactory::SetupComponent(CPerspectiveCamera* pComponent, const TPerspectiveCameraParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetFOV(params.mFOV);
		pComponent->SetAspect(params.mAspect);
		pComponent->SetNearPlane(params.mZNear);
		pComponent->SetFarPlane(params.mZFar);

		return RC_OK;
	}


	IComponentFactory* CreatePerspectiveCameraFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CPerspectiveCameraFactory, result);
	}
}