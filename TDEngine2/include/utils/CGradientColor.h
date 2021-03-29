/*!
	\file CGradientColor.h
	\date 29.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../core/Serialization.h"
#include "Types.h"
#include "Color.h"
#include <vector>
#include <tuple>


namespace TDEngine2
{
	class CGradientColor;


	/*!
		\brief A factory function for creation objects of CGradientColor's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CGradientColor's implementation
	*/

	TDE2_API CGradientColor* CreateGradientColor(const TColor32F& leftBoundColor, const TColor32F& rightBoundColor, E_RESULT_CODE& result);


	/*!
		class CGradientColor

		\brief The simple interpolator object that based of set of
	*/

	class CGradientColor : public CBaseObject, public ISerializable
	{
		public:
			friend TDE2_API CGradientColor* CreateGradientColor(const TColor32F&, const TColor32F&, E_RESULT_CODE& result);

		public:
			typedef std::tuple<F32, TColor32F> TColorSample;
			typedef std::vector<TColorSample> TColorsArray;

		public:
			/*!
				\brief The method initializes an internal state of a curve

				\param[in] leftBoundColor 
				\param[in] rightBoundColor

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TColor32F& leftBoundColor, const TColor32F& rightBoundColor);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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

			TDE2_API inline TColorsArray::iterator begin() { return mPoints.begin(); }
			TDE2_API inline TColorsArray::iterator end() { return mPoints.end(); }

			/*!
				\brief The method adds a new sample of color. The point should lay in range of [0; 1]

				\param[in] point An object that contains all point's parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddPoint(const TColorSample& point);

			TDE2_API E_RESULT_CODE RemovePoint(U32 index);

			/*!
				\brief The method allows to get value at the specified coordinate based on the curve. If there are no points
				zero will be returned. The same scenario happens if t goes out of the boundaries

				\param[in] t A coordinate (time) on the curve at which we want to get curve's value

				\return The interpolated value at given coordinate, or zero if it lies somewhere outside of the curve
			*/

			TDE2_API TColor32F Sample(F32 t) const;

			TColorSample* GetPoint(U32 index);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGradientColor)

			TDE2_API E_RESULT_CODE _addPointInternal(const TColorSample& point);
			TDE2_API I32 _getFrameIndexByTime(F32 t) const;
		protected:
			TColorsArray mPoints;
	};
}