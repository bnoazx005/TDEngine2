/*!
	\file CEditorsManager.h
	\date 08.01.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorsManager.h"
#include "../core/CBaseObject.h"
#include <vector>
#include <string>
#include <tuple>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class IDesktopInputContext;


	/*!
		\brief A factory function for creation objects of CEditorsManager's type

		\param[in, out] pInputContext A pointer to IInputContext implementation
		\param[in, out] pImGUIContext A pointer to IImGUIContext implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorsManager's implementation
	*/

	TDE2_API IEditorsManager* CreateEditorsManager(IInputContext* pInputContext, IImGUIContext* pImGUIContext, E_RESULT_CODE& result);


	/*!
		class CEditorsManager

		\brief The class implements main manager for all editors that are available within the engine
	*/

	class CEditorsManager : public IEditorsManager, public CBaseObject
	{
		public:
			friend TDE2_API IEditorsManager* CreateEditorsManager(IInputContext* pInputContext, IImGUIContext* pImGUIContext, E_RESULT_CODE& result);
		public:
			typedef std::vector<std::tuple<std::string, IEditorWindow*>> TEditorsArray;
		public:
			/*!
				\brief The method initializes an internal state of main manager for all engine's editors

				\param[in, out] pInputContext A pointer to IInputContext implementation
				\param[in, out] pImGUIContext A pointer to IImGUIContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IInputContext* pInputContext, IImGUIContext* pImGUIContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method registers custom editor within the manager

				\param[in] commandName A name of a command within the development menu's which the editor will be linked to
				\param[in, out] pEditorWindow A pointer to IEditorWindow implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterEditor(const std::string& commandName, IEditorWindow* pEditorWindow) override;

			/*!
				\brief The method updates the current state of the manager

				\param[in] dt A time elapsed from last frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Update(F32 dt) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEditorsManager)

			TDE2_API E_RESULT_CODE _showEditorWindows(F32 dt);
		protected:
			IDesktopInputContext* mpInputContext;

			IImGUIContext*        mpImGUIContext;

			bool                  mIsVisible;

			TEditorsArray         mRegisteredEditors;
	};
}

#endif