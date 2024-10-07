/*!
	\file CLabelComponent.h
	\date 23.05.2021
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

	class CLabel : public CBaseComponent, public CPoolMemoryAllocPolicy<CLabel, 1 << 20>
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

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			TDE2_API void SetText(const std::string& text);

			TDE2_API E_RESULT_CODE SetFontId(const std::string& fontId);

			TDE2_API E_RESULT_CODE SetFontResourceHandle(TResourceId handle);

			TDE2_API void SetAlignType(E_FONT_ALIGN_POLICY value);

			TDE2_API void SetOverflowPolicyType(E_TEXT_OVERFLOW_POLICY value);

			TDE2_API void SetFontDataVersionId(U32 value);

			TDE2_API void SetTextHeight(U32 height);

			TDE2_API void SetColor(const TColor32F& color);

			TDE2_API void ResetDirtyFlag();

			TDE2_API const std::string& GetText() const;

			TDE2_API const std::string& GetFontId() const;

			TDE2_API TResourceId GetFontResourceHandle() const;

			TDE2_API E_FONT_ALIGN_POLICY GetAlignType() const;

			TDE2_API E_TEXT_OVERFLOW_POLICY GetOverflowPolicyType() const;

			TDE2_API U32 GetFontDataVersionId() const;

			TDE2_API const TColor32F& GetColor() const;

			TDE2_API U32 GetTextHeight() const;

			TDE2_API bool IsDirty() const;

			TDE2_REGISTER_COMPONENT_PROPERTIES
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLabel)

		protected:
			TDE2_MULTI_THREAD_CHECK_LOCK;

			std::string mText;
			std::string mPrevText;
			std::string mFontResourceId;

			E_FONT_ALIGN_POLICY mAlignType;

			E_TEXT_OVERFLOW_POLICY mOverflowPolicyType;

			TResourceId mFontResourceHandle;

			U32         mFontDataVersionId = 0;
			U32         mTextHeight = 12;

			TColor32F   mFontVertexColor = TColorUtils::mWhite;
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