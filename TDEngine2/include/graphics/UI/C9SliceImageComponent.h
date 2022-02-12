/*!
	\file C9SliceImageComponent.h
	\date 12.02.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"
#include "../../math/TRect.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


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

	class C9SliceImage : public CBaseComponent, public CPoolMemoryAllocPolicy<C9SliceImage, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* Create9SliceImage(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(C9SliceImage)

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			TDE2_API E_RESULT_CODE SetImageId(const std::string& id);
			TDE2_API E_RESULT_CODE SetImageResourceId(TResourceId resourceId);

			TDE2_API E_RESULT_CODE SetLeftXSlicer(F32 value);
			TDE2_API E_RESULT_CODE SetRightXSlicer(F32 value);
			TDE2_API E_RESULT_CODE SetBottomYSlicer(F32 value);
			TDE2_API E_RESULT_CODE SetTopYSlicer(F32 value);

			TDE2_API const std::string& GetImageId() const;
			TDE2_API TResourceId GetImageResourceId() const;

			TDE2_API F32 GetLeftXSlicer() const;
			TDE2_API F32 GetRightXSlicer() const;
			TDE2_API F32 GetBottomYSlicer() const;
			TDE2_API F32 GetTopYSlicer() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(C9SliceImage)
		protected:
			std::string mImageSpriteId;
			TResourceId mImageResourceId;

			F32         mXStart = 0.0f, mXEnd = 1.0f;
			F32         mYStart = 0.0f, mYEnd = 1.0f;
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