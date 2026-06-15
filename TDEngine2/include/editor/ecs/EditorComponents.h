/*!
	\file EditorComponents.h
	\date 09.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../core/memory/CPoolAllocator.h"
#include "../../ecs/CBaseComponent.h"
#include <vector>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DECLARE_FLAG_COMPONENT(SelectedEntityComponent)


	enum class TSceneId: U32;


	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TSceneInfoComponentData
	{
		FIELD_META(name = scene_id) std::string mSceneId = "";

		TDE2_DECLARE_COMPONENT_META(TSceneInfoComponentData);
	};


	/*!
		\brief A factory function for creation objects of CSceneInfoComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateSceneInfoComponent(E_RESULT_CODE& result);


	class CSceneInfoComponent : public CBaseComponentT<CSceneInfoComponent, TSceneInfoComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateSceneInfoComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSceneInfoComponent)

			TDE2_API void SetSceneId(const std::string& id);

			TDE2_API const std::string& GetSceneId() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneInfoComponent)
	};

	
	typedef struct TSceneInfoComponentParameters : public TBaseComponentParameters
	{
		std::string mSceneId;
	} TSceneInfoComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(SceneInfoComponent, TSceneInfoComponentParameters);
}

#endif