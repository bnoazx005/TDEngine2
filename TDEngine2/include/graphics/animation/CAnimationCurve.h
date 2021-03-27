/*!
	\file CAnimationCurve.h
	\date 13.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../core/CBaseObject.h"
#include "../../core/Serialization.h"
#include "../../math/TRect.h"
#include "../../math/TVector2.h"
#include <vector>


namespace TDEngine2
{
	class CAnimationCurve;


	/*!
		\brief A factory function for creation objects of CAnimationCurve's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationCurve's implementation
	*/

	TDE2_API CAnimationCurve* CreateAnimationCurve(const TRectF32& bounds, E_RESULT_CODE& result);


	/*!
		class CAnimationCurve

		\brief The simple interpolator object that based of set of 
	*/

	class CAnimationCurve : public CBaseObject, public ISerializable
	{
		public:
			friend TDE2_API CAnimationCurve* CreateAnimationCurve(const TRectF32&, E_RESULT_CODE& result);

		public:
			typedef struct TKeyFrame
			{
				F32 mTime;
				F32 mValue;

				TVector2 mInTangent;
				TVector2 mOutTangent;
			} TKeyFrame;

			typedef std::vector<TKeyFrame> TKeysArray;

		public:
			/*!
				\brief The method initializes an internal state of a curve

				\param[in] bounds The rectangle that defines left bottom origin of the curve and right top corner which is its maximum possible value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TRectF32& bounds);

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

			TDE2_API inline TKeysArray::iterator begin() { return mPoints.begin(); }
			TDE2_API inline TKeysArray::iterator end() { return mPoints.end(); }

			/*!
				\brief The method adds a new key point into the curve. The point should lay in range of specified curve's boundaries

				\param[in] point An object that contains all point's parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddPoint(const TKeyFrame& point);

			TDE2_API E_RESULT_CODE RemovePoint(U32 index);

			/*!
				\brief The method allows to get value at the specified coordinate based on the curve. If there are no points
				zero will be returned. The same scenario happens if t goes out of the boundaries

				\param[in] t A coordinate (time) on the curve at which we want to get curve's value

				\return The interpolated value at given coordinate, or zero if it lies somewhere outside of the curve
			*/

			TDE2_API F32 Sample(F32 t) const;

			TKeyFrame* GetPoint(U32 index);

			TDE2_API const TRectF32& GetBounds() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationCurve)

			/*!
				\brief The method returns an index of a key in the array that's time lesser than given
			*/

			TDE2_API I32 _getFrameIndexByTime(F32 time) const;
			TDE2_API F32 _adjustTrackTime(F32 time) const;

			TDE2_API E_RESULT_CODE _updateBounds();
		protected:
			TRectF32 mBounds;

			TKeysArray mPoints;
	};
}