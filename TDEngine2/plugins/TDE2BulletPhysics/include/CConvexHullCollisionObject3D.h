/*!
	\file CConvexHullCollisionObject3D.h
	\date 06.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include <ecs/CBaseComponent.h>
#include <math/TVector4.h>
#include <vector>
#include <physics/3D/IConvexHullCollisionObject3D.h>


namespace TDEngine2
{
	CLASS_META(SECTION = bullet_ecs_plugin, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TConvexHullCollisionObject3DComponentData
	{
		FIELD_META(name = vertices) std::vector<TVector4> mVertices;

		TDE2_DECLARE_COMPONENT_META(TConvexHullCollisionObject3DComponentData);
	};


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

	class CConvexHullCollisionObject3D : public CBaseComponentT<CConvexHullCollisionObject3D, TConvexHullCollisionObject3DComponentData>, public IConvexHullCollisionObject3D
	{
		public:
			friend TDE2_API IComponent* CreateConvexHullCollisionObject3D(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CConvexHullCollisionObject3D)

			TDE2_API E_RESULT_CODE SetVertices(const std::vector<TVector4>& vertices) override;

			TDE2_API const std::vector<TVector4>& GetVertices() const override;
			
			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConvexHullCollisionObject3D)
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