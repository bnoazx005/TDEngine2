/*!
	\file CSceneLoadingTriggerComponent.h
	\date 27.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include <core/memory/CPoolAllocator.h>
#include <ecs/CBaseComponent.h>
#include <math/TAABB.h>
#include <math/TVector3.h>
#include <editor/IEditorsManager.h>


namespace TDEngine2
{
	CLASS_META(SECTION = scene_loading_ecs_plugin, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TSceneLoadingTriggerComponentData
	{
		FIELD_META(name = scene_path) std::string mScenePath;

		FIELD_META(name = offset) TVector3        mOffset = ZeroVector3;
		FIELD_META(name = sizes) TVector3         mSizes = TVector3(1.0f);

		bool                                      mIsDirty = true;
		bool                                      mOverlappingState = false;

		TDE2_DECLARE_COMPONENT_META(TSceneLoadingTriggerComponentData);
	};


	/*!
		\brief A factory function for creation objects of CSceneLoadingTriggerComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSceneLoadingTriggerComponent's implementation
	*/

	TDE2_API IComponent* CreateSceneLoadingTriggerComponent(E_RESULT_CODE& result);


	/*!
		class CSceneLoadingTriggerComponent

		\brief The class represents a volume that invokes loading of specified scene's chunk
	*/

	class CSceneLoadingTriggerComponent: public CBaseComponentT<CSceneLoadingTriggerComponent, TSceneLoadingTriggerComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateSceneLoadingTriggerComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSceneLoadingTriggerComponent)

			TDE2_API E_RESULT_CODE SetScenePath(const std::string& scenePath);

			TDE2_API E_RESULT_CODE SetVolumeOffset(const TVector3& value);
			TDE2_API E_RESULT_CODE SetVolumeSizes(const TVector3& value);

			TDE2_API void SetDirtyFlag(bool value);
			TDE2_API void SetOverlappingState(bool value);

			TDE2_API const std::string& GetScenePath() const;

			TDE2_API const TVector3& GetVolumeOffset() const;
			TDE2_API const TVector3& GetVolumeSizes() const;

			TDE2_API bool IsDirty() const;
			TDE2_API bool GetOverlappingState() const;

#if TDE2_EDITORS_ENABLED
			TDE2_API static void DrawInspectorGUI(const TEditorContext& context);
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSceneLoadingTriggerComponent)
	};


	/*!
		struct TSceneLoadingTriggerComponentParameters

		\brief The structure contains parameters for creation of CSceneLoadingTriggerComponent
	*/

	typedef struct TSceneLoadingTriggerComponentParameters : public TBaseComponentParameters
	{
		std::string mScenePath;
	} TSceneLoadingTriggerComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(SceneLoadingTriggerComponent, TSceneLoadingTriggerComponentParameters);
}