/*!
	\file CConvexHullCollisionObject3D.h
	\date 06.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject3D.h"
#include <math/TVector4.h>
#include <vector>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CConvexHullCollisionObject3D's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CConvexHullCollisionObject3D's implementation
	*/

	TDE2_API IComponent* CreateConvexHullCollisionObject3D(E_RESULT_CODE& result);


	/*!
		class CConvexHullCollisionObject3D

		\brief The interface describes a functionality of a convex hull that is constructer from a set of vertices
		The main goal is to more accurate approximation of triangle meshes
	*/

	class CConvexHullCollisionObject3D : public CBaseCollisionObject3D, public CPoolMemoryAllocPolicy<CConvexHullCollisionObject3D, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateConvexHullCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CConvexHullCollisionObject3D)

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

			TDE2_API virtual E_RESULT_CODE SetVertices(const std::vector<TVector4>& vertices);

			TDE2_API virtual const std::vector<TVector4>& GetVertices() const;

			/*!
				\brief The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine

				\param[in] pVisitor A pointer to implementation of ICollisionObjects3DVisitor

				\return The method returns a pointer to internal representation of a collision
				shape that is used by Bullet3 physics engine
			*/

			TDE2_API btCollisionShape* GetCollisionShape(const ICollisionObjects3DVisitor* pVisitor) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConvexHullCollisionObject3D)
		protected:
			const std::vector<TVector4>* mpVertices = nullptr;
	};


	/*!
		struct TConvexHullCollisionObject3DParameters

		\brief The structure contains parameters for creation of CConvexHullCollisionObject3D
	*/

	typedef struct TConvexHullCollisionObject3DParameters : public TBaseComponentParameters
	{
	} TConvexHullCollisionObject3DParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(ConvexHullCollisionObject3D, TConvexHullCollisionObject3DParameters);
}