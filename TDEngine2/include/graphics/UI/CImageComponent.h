/*!
	\file CImageComponent.h
	\date 04.05.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"
#include "../../math/TRect.h"
#include "../../utils/Color.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TImageComponentData
	{
		FIELD_META(name = sprite_id) std::string mImageSpriteId = "DefaultResources/Textures/DefaultUIWhite_Sprite.png"; /// \todo Replace with global configurable constant
		TResourceId                              mImageResourceId = TResourceId::Invalid;
		FIELD_META(name = color) TColor32F       mColor = TColorUtils::mWhite;
		bool                                     mIsDirty = true;

		TDE2_DECLARE_COMPONENT_META(TImageComponentData);
	};


	/*!
		\brief A factory function for creation objects of CImage's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CImage's implementation
	*/

	TDE2_API IComponent* CreateImage(E_RESULT_CODE& result);


	/*!
		class CImage

		\brief The interface describes a functionality of UI element that represents an image. The entity is one of fundamental parts of UI.
	*/

	class CImage : public CBaseComponentT<CImage, TImageComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateImage(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CImage)

			TDE2_API E_RESULT_CODE SetImageId(const std::string& id);
			TDE2_API E_RESULT_CODE SetImageResourceId(TResourceId resourceId);
			
			TDE2_API void SetColor(const TColor32F& value);

			TDE2_API void SetDirtyFlag(bool value);

			TDE2_API const std::string& GetImageId() const;
			TDE2_API TResourceId GetImageResourceId() const;

			TDE2_API const TColor32F& GetColor() const;

			TDE2_API bool IsDirty() const;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CImage)
	};


	/*!
		struct TImageParameters

		\brief The structure contains parameters for creation of CImage
	*/

	typedef struct TImageParameters : public TBaseComponentParameters
	{
	} TImageParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(Image, TImageParameters);
}