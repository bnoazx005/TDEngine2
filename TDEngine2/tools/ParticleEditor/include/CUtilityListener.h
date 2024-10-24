/*!
	\file CUtilityListener.h
	\date 09.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <string>


namespace TDEngine2
{
	class IEditorWindow;
	class CParticleEditorWindow;


	TDE2_DECLARE_SCOPED_PTR(CParticleEditorWindow)
}


class CUtilityListener : public TDEngine2::IEngineListener
{
	public:
		/*!
			\brief The event occurs after the engine was initialized

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnStart() override;

		/*!
			\brief The event occurs every frame

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnUpdate(const float& dt) override;

		/*!
			\brief The event occurs before the engine will be destroyed

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnFree() override;

		/*!
				\brief The method is used to inject a pointer to the engine into the object of this type

				\param[in] An instance of the engine's core
		*/

		void SetEngineInstance(TDEngine2::IEngineCore* pEngineCore) override;

	protected:
		void _drawMainMenu();
		void _drawTimeControlBar();

	protected:
		TDEngine2::IEngineCore*      mpEngineCoreInstance;

		TDEngine2::TPtr<TDEngine2::IGraphicsContext> mpGraphicsContext;

		TDEngine2::TPtr<TDEngine2::IWindowSystem> mpWindowSystem;

		TDEngine2::TPtr<TDEngine2::IResourceManager> mpResourceManager;

		TDEngine2::TPtr<TDEngine2::CParticleEditorWindow> mpParticleEditor;

		TDEngine2::TResourceId       mCurrEditableEffectId;

		std::string                  mLastSavedPath;

		TDEngine2::CEntity*          mpEditableEntity;

		bool                         mIsTimeControlBarWindowOpened = true;
		bool                         mShouldPauseOnNextFrame = false;

};