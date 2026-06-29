/*!
	\file GroupLayoutComponents.h
	\date 04.03.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../math/TVector2.h"


namespace TDEngine2
{
	enum class E_UI_ELEMENT_ALIGNMENT_TYPE : U8;


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TGridGroupLayoutComponentData
	{
		FIELD_META(name = cell_size) TVector2                     mCellSize = TVector2(100.0f);
		FIELD_META(name = spacing) TVector2                       mSpaceBetweenElements = TVector2(10.0f);

		FIELD_META(name = align_type) E_UI_ELEMENT_ALIGNMENT_TYPE mAlignType;

		bool                                                      mIsDirty = true;

		TDE2_DECLARE_COMPONENT_META(TGridGroupLayoutComponentData);
	};


	/*!
		\brief A factory function for creation objects of CGridGroupLayout's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CGridGroupLayout's implementation
	*/

	TDE2_API IComponent* CreateGridGroupLayout(E_RESULT_CODE& result);


	/*!
		class CGridGroupLayout

		\brief The component stores the data that's used to group a bunch of child LayoutElement entities
	*/

	class CGridGroupLayout : public CBaseComponentT<CGridGroupLayout, TGridGroupLayoutComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateGridGroupLayout(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CGridGroupLayout)

			TDE2_API E_RESULT_CODE SetCellSize(const TVector2& value);

			TDE2_API E_RESULT_CODE SetSpaceBetweenElements(const TVector2& value);

			TDE2_API void SetElementsAlignType(E_UI_ELEMENT_ALIGNMENT_TYPE value);

			TDE2_API void SetDirty(bool value);

			TDE2_API const TVector2& GetCellSize() const;

			TDE2_API const TVector2& GetSpaceBetweenElements() const;

			TDE2_API E_UI_ELEMENT_ALIGNMENT_TYPE GetElementsAlignType() const;

			TDE2_API bool IsDirty() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGridGroupLayout)
	};


	/*!
		struct TGridGroupLayoutParameters

		\brief The structure contains parameters for creation of CGridGroupLayout
	*/

	typedef struct TGridGroupLayoutParameters : public TBaseComponentParameters
	{
	} TGridGroupLayoutParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(GridGroupLayout, TGridGroupLayoutParameters);
}