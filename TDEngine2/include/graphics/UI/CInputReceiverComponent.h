/*!
	\file CInputReceiverComponent.h
	\date 21.05.2021
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
		\brief A factory function for creation objects of CInputReceiver's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CInputReceiver's implementation
	*/

	TDE2_API IComponent* CreateInputReceiver(E_RESULT_CODE& result);


	/*!
		class CInputReceiver

		\brief The interface describes a functionality of UI elements that should react on input events.
		The best usage example is a button
	*/

	class CInputReceiver : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateInputReceiver(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CInputReceiver)

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

			TDE2_API void SetPressedFlag(bool value);

			TDE2_API void SetIgnoreInputFlag(bool value);

			TDE2_API bool IsPressed() const;

			TDE2_API bool IsIgnoreInputFlag() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CInputReceiver)
		protected:
			bool mIsClicked;
			bool mIsIgnoreInput;
	};


	/*!
		struct TInputReceiverParameters

		\brief The structure contains parameters for creation of CInputReceiver
	*/

	typedef struct TInputReceiverParameters : public TBaseComponentParameters
	{
	} TInputReceiverParameters;


	/*!
		\brief A factory function for creation objects of CInputReceiverFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CInputReceiverFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateInputReceiverFactory(E_RESULT_CODE& result);


	/*!
		class CInputReceiverFactory

		\brief The class is factory facility to create a new objects of CInputReceiver type
	*/

	class CInputReceiverFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateInputReceiverFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CInputReceiverFactory)
	};
}