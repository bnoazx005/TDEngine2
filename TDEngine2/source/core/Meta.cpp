#include "../../include/core/Meta.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/editor/CPerfProfiler.h"


namespace TDEngine2
{
	static IComponent* GetComponentByTypeName(CEntity* pEntity, const std::string& componentId)
	{
		auto&& entityComponents = pEntity->GetComponents();

		auto it = std::find_if(entityComponents.cbegin(), entityComponents.cend(), [&componentId](const IComponent* pComponent)
		{
			return pComponent->GetTypeName() == componentId;
		});

		return (it == entityComponents.cend()) ? nullptr : *it;
	}
	

	TDE2_API IPropertyWrapperPtr ResolveBinding(IWorld* pWorld, CEntity* pEntity, const std::string& path)
	{
		TDE2_PROFILER_SCOPE("ResolveBinding, binding: " + path);

		std::string binding = Wrench::StringUtils::RemoveAllWhitespaces(path);

		std::string::size_type pos = 0;

		// \note If there are child appearances in the path go down into the hierarchy
		CEntity* pCurrEntity = pEntity;

		auto&& hierarchy = Wrench::StringUtils::Split(binding, "/");
		for (auto it = hierarchy.cbegin(); it != std::prev(hierarchy.cend()); it++)
		{
			CTransform* pTransform = pCurrEntity->GetComponent<CTransform>();

			bool hasChildFound = false;

			for (TEntityId childEntityId : pTransform->GetChildren())
			{
				if (CEntity* pChildEntity = pWorld->FindEntity(childEntityId))
				{
					if (pChildEntity->GetName() == *it)
					{
						pCurrEntity = pChildEntity;
						hasChildFound = true;
						break;
					}
				}
			}

			if (!hasChildFound)
			{
				return IPropertyWrapperPtr(nullptr);
			}
		}

		// \note Check whether the component with given identifier exist or not
		const std::string& componentBinding = hierarchy.back();

		pos = componentBinding.find_first_of('.');
		if (pos == std::string::npos)
		{
			return IPropertyWrapperPtr(nullptr);
		}

		const std::string componentTypeId = componentBinding.substr(0, pos); // \note extract component's name

		if (IComponent* pSelectedComponent = GetComponentByTypeName(pCurrEntity, componentTypeId))
		{
			return pSelectedComponent->GetProperty(componentBinding.substr(pos + 1));
		}

		return IPropertyWrapperPtr(nullptr);
	}
}