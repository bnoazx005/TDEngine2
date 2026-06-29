/*!
	\file C9SliceImageComponent.h
	\date 12.02.2021
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
	struct T9SliceImageComponentData
	{
		FIELD_META(name = sprite_id) std::string mImageSpriteId;
		TResourceId                              mImageResourceId = TResourceId::Invalid;

		FIELD_META(name = left_slice) F32        mXStart = 0.0f;
		FIELD_META(name = right_slice) F32       mXEnd = 1.0f;
		FIELD_META(name = bottom_slice) F32      mYStart = 0.0f;
		FIELD_META(name = mTopSliceKey) F32      mYEnd = 1.0f;

		FIELD_META(name = margin) F32            mRelativeBorderSize = 0.1f;

		FIELD_META(name = color) TColor32F       mColor = TColorUtils::mWhite;

		TDE2_DECLARE_COMPONENT_META(T9SliceImageComponentData);
	};


	/*!
		\brief A factory function for creation objects of C9SliceImage's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to C9SliceImage's implementation
	*/

	TDE2_API IComponent* Create9SliceImage(E_RESULT_CODE& result);


	/*!
		class C9SliceImage

		\brief The implementation of 9 sliced UI images
	*/

	class C9SliceImage : public CBaseComponentT<C9SliceImage, T9SliceImageComponentData>
	{
		public:
			friend TDE2_API IComponent* Create9SliceImage(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(C9SliceImage)

			TDE2_API E_RESULT_CODE SetImageId(const std::string& id);
			TDE2_API E_RESULT_CODE SetImageResourceId(TResourceId resourceId);

			TDE2_API E_RESULT_CODE SetLeftXSlicer(F32 value);
			TDE2_API E_RESULT_CODE SetRightXSlicer(F32 value);
			TDE2_API E_RESULT_CODE SetBottomYSlicer(F32 value);
			TDE2_API E_RESULT_CODE SetTopYSlicer(F32 value);

			/*!
				\brief The method sets up border size of the image's mesh

				\param[in] value The parameter lies in range between of 0 and 0.5
			*/

			TDE2_API E_RESULT_CODE SetRelativeBorderSize(F32 value);

			TDE2_API void SetColor(const TColor32F& value);

			TDE2_API const std::string& GetImageId() const;
			TDE2_API TResourceId GetImageResourceId() const;

			TDE2_API F32 GetLeftXSlicer() const;
			TDE2_API F32 GetRightXSlicer() const;
			TDE2_API F32 GetBottomYSlicer() const;
			TDE2_API F32 GetTopYSlicer() const;

			TDE2_API F32 GetRelativeBorderSize() const;

			TDE2_API const TColor32F& GetColor() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(C9SliceImage)
	};


	/*!
		struct T9SliceImageParameters

		\brief The structure contains parameters for creation of C9SliceImage
	*/

	typedef struct T9SliceImageParameters : public TBaseComponentParameters
	{
	} T9SliceImageParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(9SliceImage, T9SliceImageParameters);
}