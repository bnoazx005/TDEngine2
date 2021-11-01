/*!
	\file CProxyWindowSystem.h
	\date 01.11.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../core/Event.h"
#include "../core/CBaseObject.h"
#include "../core/IWindowSystem.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CProxyWindowSystem type

		\return A pointer to CProxyWindowSystem's implementation
	*/

	TDE2_API IWindowSystem* CreateProxyWindowSystem(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags, E_RESULT_CODE& result);


	/*!
		class CProxyWindowSystem

		\brief The class is a stub that's used when windowless mode is enabled for the application
	*/

	class CProxyWindowSystem : public IWindowSystem, public CBaseObject
	{
		public:
			friend TDE2_API IWindowSystem* CreateProxyWindowSystem(TPtr<IEventManager>, const std::string&, U32, U32, U32, E_RESULT_CODE&);

		public:
			TDE2_REGISTER_TYPE(CProxyWindowSystem)
				
			/*!
				\brief The method initializes a main window

				\param[in, out] pEventManager A pointer to IEventManager implementation
				\param[in] name A name of a main window
				\param[in] width An internal window's area width
				\param[in] height An internal window's area height
				\param[in] flags An additional flags (bitwise value) lets configure
				additional settings of a window is created

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IEventManager> pEventManager, const std::string& name, U32 width, U32 height, U32 flags = 0x0) override;
						
			/*!
				\brief The method processes a window's update (executing messages, redraw content)

				\param[in] onFrameUpdate A callback that is invoked every time, when the window doesn't process a message

				\return The method returns false, if Quit method was invoked or the window was closed
				in other possible way, true in other cases
			*/

			TDE2_API bool Run(const std::function<void()>& onFrameUpdate) override;

			/*!
				\brief The method stops the engine's main loop

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Quit() override;

			/*!
				\brief The method changes a window's title

				\param[in] title A new title's value

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetTitle(const std::string& title) override;

			/*!
				\brief The method pass all the needed data into the immediate GUI context

				\param[in, out] pVisitor A pointer to IImGUIContextVisitor implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ConfigureImGUIContext(IImGUIContextVisitor* pVisitor) const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			TDE2_API const TWindowSystemInternalData& GetInternalData() const override;

			/*!
				\brief The method returns a width of a window's client area

				\return The method returns a width of a window's client area
			*/

			TDE2_API U32 GetWidth() const override;

			/*!
				\brief The method returns a height of a window's client area

				\return The method returns a height of a window's client area
			*/

			TDE2_API U32 GetHeight() const override;

			/*!
				\brief The method returns a pointer to a in-engine timer

				\returns The method returns a pointer to a in-engine timer
			*/

			TDE2_API TPtr<ITimer> GetTimer() const override;

			/*!
				\brief The method returns a window's title

				\returns The method returns a window's title
			*/

			TDE2_API const std::string& GetTitle() const override;

			/*!
				\brief The method returns current set flags

				\return The method returns current set flags
			*/

			TDE2_API U32 GetFlags() const override;

			/*!
				\brief The method returns a pointer to IEventManager implementation

				\return The method returns a pointer to IEventManager implementation
			*/

			TDE2_API TPtr<IEventManager> GetEventManager() const override;

			/*!
				\brief The method returns an object of TRect type which contains
				information about sizes of the window and its position

				\return The method returns an object of TRect type which contains
				information about sizes of the window and its position
			*/

			TDE2_API TRectU32 GetWindowRect() const override;

			/*!
				\brief The method returns sizes of internal area of the window. If
				the window is drawn without borders and title bar this output is the same
				as GetWindowRect()

				\return The method returns sizes of internal area of the window
			*/

			TDE2_API TRectU32 GetClientRect() const override;

#if TDE2_EDITORS_ENABLED
			/*!
				\brief The method displays platform specific dialog window that allows to select file to open

				\param[in] filters An array of filters, each filter is an extension

				\return Either an error code, or a path to the file
			*/

			TDE2_API TResult<std::string> ShowOpenFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters) override;

			/*!
				\brief The method displays platform specific dialog window that allows to select place where the file should be saved

				\param[in] filters An array of filters, each filter is an extension

				\return Either an error code, or a path to the file
			*/


			TDE2_API TResult<std::string> ShowSaveFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters) override;
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProxyWindowSystem)

		protected:
			TPtr<ITimer>        mpTimer;

			TPtr<IEventManager> mpEventManager;

			bool                mQuitFlag;

			U32                 mWidth;
			U32                 mHeight;
			U32                 mFlags;
	};
}