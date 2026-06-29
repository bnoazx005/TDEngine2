#include "../../../include/graphics/UI/CImageComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateImageFactory)
	TDE2_DEFINE_COMPONENT_META(TImageComponentData)

	CImage::CImage() : 
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CImage::SetImageId(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mData.mImageSpriteId = id;
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CImage::SetImageResourceId(TResourceId resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mData.mImageResourceId = resourceId;
		mData.mIsDirty = true;

		return RC_OK;
	}

	void CImage::SetColor(const TColor32F& value)
	{
		mData.mColor = value;
		mData.mIsDirty = true;
	}

	void CImage::SetDirtyFlag(bool value)
	{
		mData.mIsDirty = value;
	}

	const std::string& CImage::GetImageId() const
	{
		return mData.mImageSpriteId;
	}

	TResourceId CImage::GetImageResourceId() const
	{
		return mData.mImageResourceId;
	}

	const TColor32F& CImage::GetColor() const
	{
		return mData.mColor;
	}

	bool CImage::IsDirty() const
	{
		return mData.mIsDirty;
	}
	
	const std::string& CImage::GetTypeName() const
	{
		static const std::string typeName = "image_ui";
		return typeName;
	}


	IComponent* CreateImage(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CImage, result);
	}


	/*!
		\brief CImageFactory's definition
	*/

	CImageFactory::CImageFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CImageFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateImage(result);
	}

	E_RESULT_CODE CImageFactory::SetupComponent(CImage* pComponent, const TImageParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateImageFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CImageFactory, result);
	}
}