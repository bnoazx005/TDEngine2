/*!
	\file CBaseRaycastContext.h
	\date 23.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "IRaycastContext.h"
#include <memory>


namespace TDEngine2
{
	class CPhysics2DSystem;
	class IPhysics3DSystem;
	class IAllocator;


	TDE2_DECLARE_SCOPED_PTR(IAllocator)


	/*!
		\brief A factory function for creation objects of CBaseRaycastContext's type.

		\param[in, out] p2DPhysicsSystem A pointer to 2D physics system's implementation
		\param[in, out] p3DPhysicsSystem A pointer to 3D physics system's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseRaycastContext's implementation
	*/

	TDE2_API IRaycastContext* CreateBaseRaycastContext(CPhysics2DSystem* p2DPhysicsSystem, IPhysics3DSystem* p3DPhysicsSystem, E_RESULT_CODE& result);


	/*!
		class CBaseRaycastContext

		\brief The class implements world ray casting via 2D and 3D physics engines Box2D and Bullet3 respectively
	*/

	class CBaseRaycastContext : public CBaseObject, public IRaycastContext
	{
		public:
			friend TDE2_API IRaycastContext* CreateBaseRaycastContext(CPhysics2DSystem*, IPhysics3DSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a context

				\param[in, out] pAllocator A pointer to IAllocator implementation
				\param[in, out] p2DPhysicsSystem A pointer to 2D physics system's implementation
				\param[in, out] p3DPhysicsSystem A pointer to 3D physics system's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(CPhysics2DSystem* p2DPhysicsSystem, IPhysics3DSystem* p3DPhysicsSystem);

			TDE2_API E_RESULT_CODE Register3DPhysics(IPhysics3DSystem* p3DPhysicsSystem) override;

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

			TDE2_API TRaycastResult* Raycast2DClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance) override;

			/*!
				\brief The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction

				\return The method casts a ray into a scene and returns closest object which is intersected by that.
				If there wasn't intersections nullptr is returned
			*/

			TDE2_API TRaycastResult* Raycast3DClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance) override;

			/*!
				\brief The method casts a ray into the world and gathers all collision objects that intersect with the it

				\param[in] origin A position in world space of a ray's origin
				\param[in] direction A normalized direction of a ray
				\param[in] maxDistance A maximal distance of a ray through given direction

				\return The method returns true if some intersections were found, false in other cases
			*/

			TDE2_API bool Raycast3DAll(const TVector3& origin, const TVector3& direction, F32 maxDistance, std::vector<TRaycastResult>& result) override;

			/*!
				\brief The method is used to reset internal state of the context
			*/

			TDE2_API void Reset() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseRaycastContext)
		protected:
			static constexpr U32  mMaxRaycastsPerFrame   = 16384;
			static constexpr U32  mRaycastResultTypeSize = sizeof(TRaycastResult);

			TPtr<IAllocator>      mpAllocator;

			std::unique_ptr<U8[]> mpMemoryBlock;

			CPhysics2DSystem*     mp2DPhysicsSystem;
			IPhysics3DSystem*     mp3DPhysicsSystem;
	};
}