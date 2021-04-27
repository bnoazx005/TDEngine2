/*!
	\file CLayoutElementComponent.h
	\date 27.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


	/*!
		\brief A factory function for creation objects of CLayoutElement's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLayoutElement's implementation
	*/

	TDE2_API IComponent* CreateLayoutElement(E_RESULT_CODE& result);


	/*!
		class CLayoutElement

		\brief The interface describes a functionality of an element that determines positioning and alignment of UI rectangle

		An anchor is a pointer of a rectangle's corner relative to parent's one. There are minAnchor and maxAnchor both lie in range (0, 0) and (1, 1).
		The position and sizes of the element's rectangle are defined via anchors positions and min/max offsets vectors.

		When both vertical and horizontal anchors are separated the minOffset vector stores left bottom corner's position, maxOffset - right top's one respectively.
		But if they're matched at some axis minOffset contains position of a rectangle's pivot in coordinates relative to parent's rectangle.
	*/

	class CLayoutElement : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateLayoutElement(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CLayoutElement)

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

			TDE2_API E_RESULT_CODE SetMinAnchor(const TVector2& value);
			TDE2_API E_RESULT_CODE SetMaxAnchor(const TVector2& value);

			TDE2_API E_RESULT_CODE SetMinOffset(const TVector2& value);
			TDE2_API E_RESULT_CODE SetMaxOffset(const TVector2& value);

			TDE2_API const TVector2& GetMinAnchor() const;
			TDE2_API const TVector2& GetMaxAnchor() const;

			TDE2_API const TVector2& GetMinOffset() const;
			TDE2_API const TVector2& GetMaxOffset() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLayoutElement)
		protected:
			TVector2 mMinAnchor;
			TVector2 mMaxAnchor;

			TVector2 mMinOffset;
			TVector2 mMaxOffset;
	};


	/*!
		struct TLayoutElementParameters

		\brief The structure contains parameters for creation of CLayoutElement
	*/

	typedef struct TLayoutElementParameters : public TBaseComponentParameters
	{
	} TLayoutElementParameters;


	/*!
		\brief A factory function for creation objects of CLayoutElementFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLayoutElementFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateLayoutElementFactory(E_RESULT_CODE& result);


	/*!
		class CLayoutElementFactory

		\brief The class is factory facility to create a new objects of CLayoutElement type
	*/

	class CLayoutElementFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateLayoutElementFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLayoutElementFactory)
	};
}