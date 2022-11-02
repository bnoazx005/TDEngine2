#include "../include/ComponentsInspectors.h"
#include "../include/CBoxCollisionObject3D.h"
#include "../include/CSphereCollisionObject3D.h"
#include "../include/CConvexHullCollisionObject3D.h"
#include "../include/CTrigger3D.h"
#include <ecs/IComponent.h>
#include <ecs/CTransform.h>
#include <ecs/IWorld.h>
#include <editor/Inspectors.h>
#include <core/IImGUIContext.h>
#include <metadata.h>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	template <typename TFunctionType>
	void Header(const std::string& headerText, const TEditorContext& editorContext, TFunctionType action)
	{
		IImGUIContext& imguiContext = editorContext.mImGUIContext;

		if (imguiContext.CollapsingHeader(headerText, true))
		{
			imguiContext.DisplayIDGroup(static_cast<I32>(editorContext.mCurrEntityId), [&imguiContext, &editorContext]
			{
				imguiContext.DisplayContextMenu(Wrench::StringUtils::GetEmptyStr(), [&editorContext](IImGUIContext& imgui)
				{
					if (CTransform::GetTypeId() != editorContext.mComponent.GetComponentTypeId()) /// CTransform couldn't be removed from any entity
					{
						imgui.MenuItem("Remove Component", Wrench::StringUtils::GetEmptyStr(), [&editorContext]
						{
							if (auto pEntity = editorContext.mWorld.FindEntity(editorContext.mCurrEntityId))
							{
								pEntity->RemoveComponent(editorContext.mComponent.GetComponentTypeId());
							}
						});
					}
				});
			});

			action(editorContext);
		}
	}


	template <typename T>
	static void DrawCommonPhysicsFields(IImGUIContext& imguiContext, T& baseCollisionObject)
	{
		static std::vector<std::string> collisionTypes;
		if (collisionTypes.empty())
		{
			for (auto&& currEnumFieldInfo : Meta::EnumTrait<E_COLLISION_OBJECT_TYPE>::GetFields())
			{
				collisionTypes.emplace_back(currEnumFieldInfo.name);
			}
		}

		/// \note Collision type
		{
			I32 currCollisionType = static_cast<I32>(baseCollisionObject.GetCollisionType());

			imguiContext.BeginHorizontal();
			imguiContext.Label("Type: ");
			currCollisionType = imguiContext.Popup("##CollisionType", currCollisionType, collisionTypes);
			imguiContext.EndHorizontal();

			baseCollisionObject.SetCollisionType(static_cast<E_COLLISION_OBJECT_TYPE>(currCollisionType));
		}

		/// \note mass
		if (E_COLLISION_OBJECT_TYPE::COT_DYNAMIC == baseCollisionObject.GetCollisionType())
		{
			F32 mass = baseCollisionObject.GetMass();

			imguiContext.BeginHorizontal();
			imguiContext.Label("Mass: ");
			imguiContext.FloatField("##Mass", mass, [&baseCollisionObject, &mass] { baseCollisionObject.SetMass(mass); });
			imguiContext.EndHorizontal();
		}
	}


	static void DrawBoxCollision3DGUI(const TEditorContext& editorContext)
	{
		Header("BoxCollision3D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CBoxCollisionObject3D& box3Dcollision = dynamic_cast<CBoxCollisionObject3D&>(component);

			DrawCommonPhysicsFields(imguiContext, box3Dcollision);

			/// \note Extents
			{
				TVector3 extents = box3Dcollision.GetSizes();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Extents: ");
				imguiContext.Vector3Field("##Extents", extents, [&box3Dcollision, &extents] { box3Dcollision.SetSizes(extents); });
				imguiContext.EndHorizontal();
			}
		});
	}

	static void DrawSphereCollision3DGUI(const TEditorContext& editorContext)
	{
		Header("SphereCollision3D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CSphereCollisionObject3D& sphereCollision = dynamic_cast<CSphereCollisionObject3D&>(component);

			DrawCommonPhysicsFields(imguiContext, sphereCollision);

			/// \note Radius
			{
				F32 radius = sphereCollision.GetRadius();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Radius: ");
				imguiContext.FloatField("##Radius", radius, [&sphereCollision, &radius] { sphereCollision.SetRadius(radius); });
				imguiContext.EndHorizontal();
			}	
		});
	}

	static void DrawConvexHullCollision3DGUI(const TEditorContext& editorContext)
	{
		Header("ConvexHullCollision3D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CConvexHullCollisionObject3D& convexHullCollision = dynamic_cast<CConvexHullCollisionObject3D&>(component);

			DrawCommonPhysicsFields(imguiContext, convexHullCollision);
		});
	}

	static void DrawTrigger3DGUI(const TEditorContext& editorContext)
	{
		Header("Trigger3D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			// \todo Implement this drawer
		});
	}


	E_RESULT_CODE RegisterComponentsInspector(TPtr<IEditorsManager> pEditorsManager)
	{
		E_RESULT_CODE result = RC_OK;

		result = result | pEditorsManager->RegisterComponentInspector(CBoxCollisionObject3D::GetTypeId(), DrawBoxCollision3DGUI);
		result = result | pEditorsManager->RegisterComponentInspector(CSphereCollisionObject3D::GetTypeId(), DrawSphereCollision3DGUI);
		result = result | pEditorsManager->RegisterComponentInspector(CConvexHullCollisionObject3D::GetTypeId(), DrawConvexHullCollision3DGUI);
		result = result | pEditorsManager->RegisterComponentInspector(CTrigger3D::GetTypeId(), DrawTrigger3DGUI);

		return result;
	}
}

#endif