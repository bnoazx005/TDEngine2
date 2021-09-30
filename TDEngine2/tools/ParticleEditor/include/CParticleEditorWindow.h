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

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateParticleEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result);

	/*!
		class CParticleEditorWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CParticleEditorWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateParticleEditorWindow(IResourceManager*, IInputContext*, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IInputContext* pInputContext);

#if TDE2_EDITORS_ENABLED

			/*!
				\brief The method rolls back last editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ExecuteUndoAction() override;

			/*!
				\brief The method applies latest rolled back editor's action

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ExecuteRedoAction() override;

#endif

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

			TDE2_API void _displayEmitterShapeSettings();

			TDE2_API void _drawColorDataModifiers(const std::string& label, TParticleColorParameter& colorData, const std::function<void()>& onChangedAction);
			TDE2_API void _drawVelocityDataModifiers(TParticleVelocityParameter& velocityData, const std::function<void()>& onChangedAction);

		protected:
			CScopedPtr<CAnimationCurveEditorWindow> mpCurveEditor;

			TResourceId mCurrParticleEffectId;

			IResourceManager*     mpResourceManager;
			IDesktopInputContext* mpInputContext;

			CScopedPtr<IEditorActionsHistory> mpEditorHistory;

			IParticleEffect* mpCurrParticleEffect;

			static std::vector<std::string> mEmittersTypesIds;
			static std::vector<std::string> mColorTypesIds;
			static std::vector<std::string> mSimulationSpaceTypesIds;
			static std::vector<std::string> mVelocityParamTypesIds;
			static std::vector<std::string> mEmitterShapesTypesIds;
	};
}