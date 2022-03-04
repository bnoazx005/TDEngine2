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

	class CGridGroupLayout : public CBaseComponent, public CPoolMemoryAllocPolicy<CGridGroupLayout, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateGridGroupLayout(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CGridGroupLayout)

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

			TDE2_API E_RESULT_CODE SetCellSize(const TVector2& value);

			TDE2_API E_RESULT_CODE SetSpaceBetweenElements(const TVector2& value);

			TDE2_API void SetLeftPadding(F32 value);
			TDE2_API void SetRightPadding(F32 value);
			TDE2_API void SetTopPadding(F32 value);
			TDE2_API void SetBottomPadding(F32 value);

			TDE2_API void SetRowsCount(U16 value);
			TDE2_API void SetColumnsCount(U16 value);

			TDE2_API void SetDirty(bool value);

			TDE2_API const TVector2& GetCellSize() const;

			TDE2_API const TVector2& GetSpaceBetweenElements() const;

			TDE2_API F32 GetLeftPadding() const;
			TDE2_API F32 GetRightPadding() const;
			TDE2_API F32 GetTopPadding() const;
			TDE2_API F32 GetBottomPadding() const;

			TDE2_API U16 GetRowsCount() const;
			TDE2_API U16 GetColumnsCount() const;

			TDE2_API bool IsDirty() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGridGroupLayout)
		protected:
			TVector2 mCellSize;
			TVector2 mSpaceBetweenElements;
			
			F32      mLeftPadding;
			F32      mRightPadding;
			F32      mTopPadding;
			F32      mBottomPadding;

			U16      mRowsCount = 1;
			U16      mColsCount = 1;

			bool     mIsDirty = true;
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