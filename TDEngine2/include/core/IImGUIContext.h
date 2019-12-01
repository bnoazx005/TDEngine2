/*!
	/file IImGUIContext.h
	/date 01.12.2019
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include "IEngineSubsystem.h"


namespace TDEngine2
{
	class IWindowSystem;
	class IGraphicsObjectManager;
	class IInputContext;


	/*!
		interface IImGUIContext

		\brief The interface represents context of immediate mode GUI which is 
		responsible for implemnentation of all editor's stuffs
	*/

	class IImGUIContext : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an internal state of a context

				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
												IInputContext* pInputContext) = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_IMGUI_CONTEXT; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IImGUIContext)
	};
}