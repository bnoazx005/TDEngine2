/*!
	\file CUtilityListener.h
	\date 09.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include "CFontPreviewWindow.h"
#include "CConfigWindow.h"


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
		TDEngine2::IEngineCore*      mpEngineCoreInstance;

		TDEngine2::CScopedPtr<TDEngine2::IGraphicsContext> mpGraphicsContext;

		TDEngine2::CScopedPtr<TDEngine2::IWindowSystem>    mpWindowSystem;

		TDEngine2::CScopedPtr<TDEngine2::IResourceManager> mpResourceManager;

		TDEngine2::CScopedPtr<TDEngine2::IFileSystem>      mpFileSystem;

		TDEngine2::CScopedPtr<TDEngine2::IInputContext>    mpInputContext;

		TDEngine2::CScopedPtr<TDEngine2::CFontPreviewWindow> mpPreviewEditorWindow;
		TDEngine2::CScopedPtr<TDEngine2::IEditorWindow> mpConfigEditorWindow;

};