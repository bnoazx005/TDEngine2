/*!
	\file CLabelComponent.h
	\date 23.05.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"
#include "../../math/TRect.h"


namespace TDEngine2
{
	enum class TResourceId : U32;
	enum class E_FONT_ALIGN_POLICY : U16;
	enum class E_TEXT_OVERFLOW_POLICY : U16;


	/*!
		\brief A factory function for creation objects of CLabel's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLabel's implementation
	*/

	TDE2_API IComponent* CreateLabel(E_RESULT_CODE& result);


	/*!
		class CLabel

		\brief The interface describes a functionality of UI element that represents labels
	*/

	class CLabel : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateLabel(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CLabel)

			/*!
				\brief The method initializes an internal state

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

			TDE2_API void SetText(const std::string& text);

			TDE2_API E_RESULT_CODE SetFontId(const std::string& fontId);

			TDE2_API E_RESULT_CODE SetFontResourceHandle(TResourceId handle);

			TDE2_API void SetAlignType(E_FONT_ALIGN_POLICY value);

			TDE2_API void SetOverflowPolicyType(E_TEXT_OVERFLOW_POLICY value);

			TDE2_API const std::string& GetText() const;

			TDE2_API const std::string& GetFontId() const;

			TDE2_API TResourceId GetFontResourceHandle() const;

			TDE2_API E_FONT_ALIGN_POLICY GetAlignType() const;

			TDE2_API E_TEXT_OVERFLOW_POLICY GetOverflowPolicyType() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLabel)

		protected:
			std::string mText;
			std::string mFontResourceId;

			E_FONT_ALIGN_POLICY mAlignType;

			E_TEXT_OVERFLOW_POLICY mOverflowPolicyType;

			TResourceId mFontResourceHandle;
	};


	/*!
		struct TLabelParameters

		\brief The structure contains parameters for creation of CLabel
	*/

	typedef struct TLabelParameters : public TBaseComponentParameters
	{
	} TLabelParameters;


	/*!
		\brief A factory function for creation objects of CLabelFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLabelFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateLabelFactory(E_RESULT_CODE& result);


	/*!
		class CLabelFactory

		\brief The class is factory facility to create a new objects of CLabel type
	*/

	class CLabelFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateLabelFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLabelFactory)
	};
}