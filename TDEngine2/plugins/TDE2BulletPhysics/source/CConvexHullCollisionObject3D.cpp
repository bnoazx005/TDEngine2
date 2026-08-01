#include "../include/CConvexHullCollisionObject3D.h"
#include "../deps/bullet3/src/btBulletDynamicsCommon.h"
#define META_EXPORT_BULLET_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TConvexHullCollisionObject3DComponentData)


	CConvexHullCollisionObject3D::CConvexHullCollisionObject3D() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CConvexHullCollisionObject3D::SetVertices(const std::vector<TVector4>& vertices)
	{
		mData.mVertices = std::move(vertices);
		return RC_OK;
	}

	const std::vector<TVector4>& CConvexHullCollisionObject3D::GetVertices() const
	{
		return mData.mVertices;
	}

	const std::string& CConvexHullCollisionObject3D::GetTypeName() const
	{
		static std::string typeName{ "convex_hull_collision_3d" };
		return typeName;
	}


	IComponent* CreateConvexHullCollisionObject3D(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CConvexHullCollisionObject3D, result);
	}


	/*!
		\brief CConvexHullCollisionObject3DFactory's definition
	*/

	CConvexHullCollisionObject3DFactory::CConvexHullCollisionObject3DFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CConvexHullCollisionObject3DFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateConvexHullCollisionObject3D(result);
	}

	E_RESULT_CODE CConvexHullCollisionObject3DFactory::SetupComponent(CConvexHullCollisionObject3D* pComponent, const TConvexHullCollisionObject3DParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateConvexHullCollisionObject3DFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CConvexHullCollisionObject3DFactory, result);
	}
}