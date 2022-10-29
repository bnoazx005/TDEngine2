/*!
	\file IPhysics3DSystem.h
	\date 29.10.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Config.h"
#include "../core/IBaseObject.h"
#include "../math/TVector3.h"
#include "IRaycastContext.h"


namespace TDEngine2
{
	/*!
		interface IPhysics3DSystem

		\brief The interface describes the functionality that any 3D physics system should provide to the game world
	*/

	class IPhysics3DSystem: public virtual IBaseObject
	{
		public:
			typedef std::function<void(const TRaycastResult&)> TOnRaycastHitCallback;
		public:
			/*!
				\brief The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned. The method isn't asynchronous, its callback will
				be called before the method returns execution context to its caller

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction
				\param[in] onHitCallback A callback which is called only if some object was hitted by the ray

				\return The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned
			*/

			TDE2_API virtual void RaycastClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance, const TOnRaycastHitCallback& onHitCallback) = 0;

			/*!
				\brief The method casts a ray into the world and gathers all collision objects that intersect with the it

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction
				\param[out] result An array with TRaycastResult objects

				\return The method returns true if some intersections were found, false in other cases
			*/

			TDE2_API virtual bool RaycastAll(const TVector3& origin, const TVector3& direction, F32 maxDistance, std::vector<TRaycastResult>& hitResults) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPhysics3DSystem)
	};


	TDE2_DECLARE_SCOPED_PTR(IPhysics3DSystem)
}