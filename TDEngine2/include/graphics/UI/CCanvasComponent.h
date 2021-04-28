/*!
	\file CCanvasComponent.h
	\date 28.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"


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

	class CCanvas : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateCanvas(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CCanvas)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

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

			TDE2_API void SetWidth(U32 value);
			TDE2_API void SetHeight(U32 value);

			TDE2_API U32 GetWidth() const;
			TDE2_API U32 GetHeight() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCanvas)
		protected:
			U32 mWidth;
			U32 mHeight;
	};


	/*!
		struct TCanvasParameters

		\brief The structure contains parameters for creation of CCanvas
	*/

	typedef struct TCanvasParameters : public TBaseComponentParameters
	{
	} TCanvasParameters;


	/*!
		\brief A factory function for creation objects of CCanvasFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CCanvasFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateCanvasFactory(E_RESULT_CODE& result);


	/*!
		class CCanvasFactory

		\brief The class is factory facility to create a new objects of CCanvas type
	*/

	class CCanvasFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateCanvasFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CCanvasFactory)
	};
}