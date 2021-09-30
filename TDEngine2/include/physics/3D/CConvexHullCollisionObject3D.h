/*!
	\file CConvexHullCollisionObject3D.h
	\date 06.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseCollisionObject3D.h"
#include "../../math/TVector4.h"
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

	class CConvexHullCollisionObject3D : public CBaseCollisionObject3D
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


	/*!
		\brief A factory function for creation objects of CConvexHullCollisionObject3DFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CConvexHullCollisionObject3DFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateConvexHullCollisionObject3DFactory(E_RESULT_CODE& result);


	/*!
		class CConvexHullCollisionObject3DFactory

		\brief The class is factory facility to create a new objects of CConvexHullCollisionObject3D type
	*/

	class CConvexHullCollisionObject3DFactory : public ICollisionObject3DFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateConvexHullCollisionObject3DFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConvexHullCollisionObject3DFactory)
	};
}