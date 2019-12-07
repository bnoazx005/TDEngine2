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
	class CWin32WindowSystem;
	class CUnixWindowSystem;
	class IResourceManager;


	/*!
		interface IImGUIContextVisitor

		\brief The interface describes functionality of a visitor which is used
		to proper configure of IImGUIContext object
	*/

	class IImGUIContextVisitor
	{
		public:
			/*!
				\brief The method configures the immediate GUI context for WIN32 platform

				\param[in] pWindowSystem A pointer to CWin32WindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureForWin32Platform(const CWin32WindowSystem* pWindowSystem) = 0;
			
			/*!
				\brief The method configures the immediate GUI context for UNIX platform

				\param[in] pWindowSystem A pointer to CUnixWindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureForUnixPlatform(const CUnixWindowSystem* pWindowSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IImGUIContextVisitor)
	};


	/*!
		interface IImGUIContext

		\brief The interface represents context of immediate mode GUI which is 
		responsible for implemnentation of all editor's stuffs
	*/

	class IImGUIContext : public IEngineSubsystem, public IImGUIContextVisitor
	{
		public:
			/*!
				\brief The method initializes an internal state of a context

				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
												IResourceManager* pResourceManager, IInputContext* pInputContext) = 0;

			/*!
				\brief The method begins to populate immediage GUI state. Any UI element should be drawn during
				BeginFrame/EndFrame scope

				\param[in] dt Time elapsed from last frame was rendered
			*/

			TDE2_API virtual void BeginFrame(float dt) = 0;

			/*!
				\brief The method flushes current state and send all the data onto GPU to render it
			*/

			TDE2_API virtual void EndFrame() = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_IMGUI_CONTEXT; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IImGUIContext)
	};
}