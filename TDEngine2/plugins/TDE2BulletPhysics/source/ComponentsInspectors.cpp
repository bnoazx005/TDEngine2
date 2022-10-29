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


	static void DrawBoxCollision3DGUI(const TEditorContext& editorContext)
	{
		Header("BoxCollision3D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CBaseCollisionObject3D& box3Dcollision = dynamic_cast<CBaseCollisionObject3D&>(component);

			// \todo Implement this drawer
		});
	}

	static void DrawSphereCollision3DGUI(const TEditorContext& editorContext)
	{
		Header("SphereCollision3D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CSphereCollisionObject3D& sphereCollision = dynamic_cast<CSphereCollisionObject3D&>(component);

			// \todo Implement this drawer
		});
	}

	static void DrawConvexHullCollision3DGUI(const TEditorContext& editorContext)
	{
		Header("ConvexHullCollision3D", editorContext, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			IComponent& component = editorContext.mComponent;

			CConvexHullCollisionObject3D& convexHullCollision = dynamic_cast<CConvexHullCollisionObject3D&>(component);

			// \todo Implement this drawer
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