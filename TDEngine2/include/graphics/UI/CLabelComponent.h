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

			TDE2_API void ResetDirtyFlag();

			TDE2_API const std::string& GetText() const;

			TDE2_API const std::string& GetFontId() const;

			TDE2_API TResourceId GetFontResourceHandle() const;

			TDE2_API E_FONT_ALIGN_POLICY GetAlignType() const;

			TDE2_API E_TEXT_OVERFLOW_POLICY GetOverflowPolicyType() const;

			TDE2_API bool IsDirty() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLabel)

		protected:
			std::string mText;
			std::string mPrevText;
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


	TDE2_DECLARE_COMPONENT_FACTORY(Label, TLabelParameters);
}