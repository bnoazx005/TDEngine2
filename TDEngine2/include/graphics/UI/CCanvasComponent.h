/*!
	\file CCanvasComponent.h
	\date 28.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TMatrix4.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


	/*!
		\brief A factory function for creation objects of CCanvas's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CCanvas's implementation
	*/

	TDE2_API IComponent* CreateCanvas(E_RESULT_CODE& result);


	/*!
		class CCanvas

		\brief The class is a main element that determines sizes of UI's field
	*/

	class CCanvas : public CBaseComponent, public CPoolMemoryAllocPolicy<CCanvas, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateCanvas(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CCanvas)

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

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			TDE2_API void SetWidth(U32 value);
			TDE2_API void SetHeight(U32 value);

			TDE2_API void SetProjMatrix(const TMatrix4& projMat);

			TDE2_API void SetInheritSizesFromMainCamera(bool value);

			TDE2_API U32 GetWidth() const;
			TDE2_API U32 GetHeight() const;

			TDE2_API bool IsDirty() const;
			TDE2_API bool DoesInheritSizesFromMainCamera() const;

			TDE2_API const TMatrix4& GetProjMatrix() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCanvas)
		protected:
			U32 mWidth;
			U32 mHeight;

			bool mIsDirty;
			bool mInheritsSizesFromMainCamera;

			TMatrix4 mProjMatrix;
	};


	/*!
		struct TCanvasParameters

		\brief The structure contains parameters for creation of CCanvas
	*/

	typedef struct TCanvasParameters : public TBaseComponentParameters
	{
	} TCanvasParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(Canvas, TCanvasParameters);
}