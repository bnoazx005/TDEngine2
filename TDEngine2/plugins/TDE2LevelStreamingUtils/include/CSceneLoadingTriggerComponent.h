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

	class CSceneLoadingTriggerComponent: public CBaseComponent, public CPoolMemoryAllocPolicy<CSceneLoadingTriggerComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateSceneLoadingTriggerComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSceneLoadingTriggerComponent)

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
		protected:
			std::string mScenePath;

			TVector3    mOffset = ZeroVector3;
			TVector3    mSizes = TVector3(1.0f);

			bool mIsDirty = true;
			bool mOverlappingState = false;
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