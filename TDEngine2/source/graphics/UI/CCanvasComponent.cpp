#include "../../../include/graphics/UI/CCanvasComponent.h"


namespace TDEngine2
{
	struct TCanvasArchiveKeys
	{
	};


	CCanvas::CCanvas() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CCanvas::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsDirty = true;
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CCanvas::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CCanvas::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	void CCanvas::SetWidth(U32 value)
	{
		mWidth = value;
		mIsDirty = true;
	}

	void CCanvas::SetHeight(U32 value)
	{
		mHeight = value;
		mIsDirty = true;
	}

	void CCanvas::SetProjMatrix(const TMatrix4& projMat)
	{
		mProjMatrix = projMat;
		mIsDirty = false;
	}

	void CCanvas::SetInheritSizesFromMainCamera(bool value)
	{
		mInheritsSizesFromMainCamera = value;
	}

	U32 CCanvas::GetWidth() const
	{
		return mWidth;
	}

	U32 CCanvas::GetHeight() const
	{
		return mHeight;
	}

	bool CCanvas::IsDirty() const
	{
		return mIsDirty;
	}

	bool CCanvas::DoesInheritSizesFromMainCamera() const
	{
		return mInheritsSizesFromMainCamera;
	}

	const TMatrix4& CCanvas::GetProjMatrix() const
	{
		return mProjMatrix;
	}


	IComponent* CreateCanvas(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CCanvas, result);
	}


	CCanvasFactory::CCanvasFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CCanvasFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CCanvasFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CCanvasFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TCanvasParameters* params = static_cast<const TCanvasParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateCanvas(result);
	}

	IComponent* CCanvasFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateCanvas(result);
	}

	TypeId CCanvasFactory::GetComponentTypeId() const
	{
		return CCanvas::GetTypeId();
	}


	IComponentFactory* CreateCanvasFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CCanvasFactory, result);
	}
}