/*!
	\file CLayoutElementComponent.h
	\date 27.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"
#include "../../math/TRect.h"


namespace TDEngine2
{
	enum class TResourceId : U32;


	enum class E_UI_ELEMENT_ALIGNMENT_TYPE : U8
	{
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		LEFT_TOP,
		LEFT_BOTTOM,
		RIGHT_TOP,
		RIGHT_BOTTOM
	};


	/*!
		\brief The method computes an offset from the left bottom corner of holderRect that should be applied to contentRect to make it aligned
	*/
	
	TDE2_API TVector2 CalcContentRectAlignByType(const TRectF32& holderRect, const TRectF32& contentRect, E_UI_ELEMENT_ALIGNMENT_TYPE alignType);



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

	class CLayoutElement : public CBaseComponent, public CPoolMemoryAllocPolicy<CLayoutElement, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateLayoutElement(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CLayoutElement)

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

			TDE2_API E_RESULT_CODE SetPivot(const TVector2& value);

			TDE2_API void SetWorldRect(const TRectF32& rect);
			TDE2_API void SetAnchorWorldRect(const TRectF32& rect);
			TDE2_API void SetParentWorldRect(const TRectF32& rect);

			TDE2_API void SetOwnerCanvasId(TEntityId canvasEntityId);

			TDE2_API void SetDirty(bool value);
			

			TDE2_API const TVector2& GetMinAnchor() const;
			TDE2_API const TVector2& GetMaxAnchor() const;

			TDE2_API const TVector2& GetMinOffset() const;
			TDE2_API const TVector2& GetMaxOffset() const;

			TDE2_API const TVector2& GetPivot() const;

			TDE2_API const TRectF32& GetWorldRect() const;
			TDE2_API const TRectF32& GetAnchorWorldRect() const;
			TDE2_API const TRectF32& GetParentWorldRect() const;

			TDE2_API TEntityId GetOwnerCanvasId() const;

			TDE2_API bool IsDirty() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLayoutElement)
		protected:
			TVector2 mMinAnchor;
			TVector2 mMaxAnchor;

			TVector2 mMinOffset;
			TVector2 mMaxOffset;

			TVector2 mPivot;

			TRectF32 mWorldRect;
			TRectF32 mAnchorWorldRect;
			TRectF32 mParentWorldRect;

			TEntityId mCanvasEntityId;

			bool      mIsDirty;
	};


	/*!
		struct TLayoutElementParameters

		\brief The structure contains parameters for creation of CLayoutElement
	*/

	typedef struct TLayoutElementParameters : public TBaseComponentParameters
	{
	} TLayoutElementParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(LayoutElement, TLayoutElementParameters);
}