/*!
	\file CParticleEditorWindow.h
	\date 09.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <string>
#include <vector>


namespace TDEngine2
{
	const std::string EditableEntityId         = "ParticleEffectEntity";
	const std::string EditableEffectResourceId = "ParticleEffectResource";


	/*!
		\brief A factory function for creation objects of CParticleEditorWindow's type

		\param[in, out] desc An object that determines parameters of the window
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateParticleEditorWindow(IResourceManager* pResourceManager, E_RESULT_CODE& result);

	/*!
		class CParticleEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CParticleEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateParticleEditorWindow(IResourceManager*, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			TDE2_API void SetParticleEffectResourceHandle(TResourceId handle);

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CParticleEditorWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _onEmitterSettingsHandle();
			TDE2_API void _onParticlesSettingsHandle();
			TDE2_API void _onRenderingSettingsHandle();

		protected:
			CScopedPtr<CAnimationCurveEditorWindow> mpCurveEditor;

			TResourceId mCurrParticleEffectId;

			IResourceManager* mpResourceManager;

			IParticleEffect* mpCurrParticleEffect;

			static std::vector<std::string> mEmittersTypesIds;
			static std::vector<std::string> mColorTypesIds;
	};
}