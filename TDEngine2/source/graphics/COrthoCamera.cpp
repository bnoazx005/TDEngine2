#include "../../include/graphics/COrthoCamera.h"
#include "../../include/ecs/ICameraSystem.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateOrthoCameraFactory)


	COrthoCamera::COrthoCamera() :
		CBaseCamera()
	{
	}

	E_RESULT_CODE COrthoCamera::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mWidth = pReader->GetFloat("width");
		mHeight = pReader->GetFloat("height");

		return RC_OK;
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

	E_RESULT_CODE COrthoCamera::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<COrthoCamera*>(pDestObject))
		{
			pComponent->mWidth = mWidth;
			pComponent->mHeight = mHeight;
			pComponent->mZNear = mZNear;
			pComponent->mZFar = mZFar;

			return RC_OK;
		}

		return RC_FAIL;
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


	IComponent* CreateOrthoCamera(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, COrthoCamera, result);
	}


	/*!
		\brief COrthoCameraFactory's definition
	*/

	COrthoCameraFactory::COrthoCameraFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* COrthoCameraFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateOrthoCamera(result);
	}

	E_RESULT_CODE COrthoCameraFactory::SetupComponent(COrthoCamera* pComponent, const TOrthoCameraParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetWidth(params.mViewportWidth);
		pComponent->SetHeight(params.mViewportHeight);
		pComponent->SetNearPlane(params.mZNear);
		pComponent->SetFarPlane(params.mZFar);

		return RC_OK;
	}


	IComponentFactory* CreateOrthoCameraFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, COrthoCameraFactory, result);
	}
}