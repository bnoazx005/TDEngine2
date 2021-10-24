/*!
	\file IRaycastContext.h
	\date 23.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Config.h"
#include "../core/IBaseObject.h"
#include "../math/TVector3.h"


namespace TDEngine2
{
	/*!
		struct TRaycastResult

		\brief The type contains all data that represents a collision between a casted ray and some physics object
	*/

	typedef struct TRaycastResult
	{
		TEntityId mEntityId;

		TVector3  mPoint;
		TVector3  mNormal;
	} TRaycastResult, *TRaycastResultPtr;


	/*!
		interface IRaycastContext

		\brief The interface describes functionality of world ray casting
	*/

	class IRaycastContext: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned. The method should be used for work with 2D physics objects
				which are controled by Box2D

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction

				\return The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned
			*/

			TDE2_API virtual TRaycastResult* Raycast2DClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance) = 0;

			/*!
				\brief The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction

				\return The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned
			*/

			TDE2_API virtual TRaycastResult* Raycast3DClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance) = 0;

			/*!
				\brief The method casts a ray into the world and gathers all collision objects that intersect with the it

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction
				\param[out] result An array with TRaycastResult objects

				\return The method returns true if some intersections were found, false in other cases
			*/

			TDE2_API virtual bool Raycast3DAll(const TVector3& origin, const TVector3& direction, F32 maxDistance, std::vector<TRaycastResult>& result) = 0;

			/*!
				\brief The method is used to reset internal state of the context
			*/

			TDE2_API virtual void Reset() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRaycastContext)
	};


	TDE2_DECLARE_SCOPED_PTR(IRaycastContext)
}