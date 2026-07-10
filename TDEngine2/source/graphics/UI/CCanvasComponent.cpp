#include "../../../include/graphics/UI/CCanvasComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateCanvasFactory)
	TDE2_DEFINE_COMPONENT_META(TCanvasComponentData)


	CCanvas::CCanvas() :
		CBaseComponentT()
	{
	}

	void CCanvas::SetWidth(U32 value)
	{
		mData.mWidth = value;
		mData.mIsDirty = true;
	}

	void CCanvas::SetHeight(U32 value)
	{
		mData.mHeight = value;
		mData.mIsDirty = true;
	}

	void CCanvas::SetProjMatrix(const TMatrix4& projMat)
	{
		mData.mProjMatrix = projMat;
		mData.mIsDirty = false;
	}

	void CCanvas::SetInheritSizesFromMainCamera(bool value)
	{
		mData.mInheritsSizesFromMainCamera = value;
	}

	void CCanvas::SetPriority(int value)
	{
		mData.mPriority = value;
		mData.mIsDirty = true;
	}

	U32 CCanvas::GetWidth() const
	{
		return mData.mWidth;
	}

	U32 CCanvas::GetHeight() const
	{
		return mData.mHeight;
	}

	bool CCanvas::IsDirty() const
	{
		return mData.mIsDirty;
	}

	bool CCanvas::DoesInheritSizesFromMainCamera() const
	{
		return mData.mInheritsSizesFromMainCamera;
	}

	const TMatrix4& CCanvas::GetProjMatrix() const
	{
		return mData.mProjMatrix;
	}

	int CCanvas::GetPriority() const
	{
		return mData.mPriority;
	}

	const std::string& CCanvas::GetTypeName() const
	{
		static const std::string typeName = "canvas";
		return typeName;
	}


	IComponent* CreateCanvas(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CCanvas, result);
	}


	/*!
		\brief CCanvasFactory's definition
	*/

	CCanvasFactory::CCanvasFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CCanvasFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateCanvas(result);
	}

	E_RESULT_CODE CCanvasFactory::SetupComponent(CCanvas* pComponent, const TCanvasParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateCanvasFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CCanvasFactory, result);
	}
}