/*!
	\file IWindowSystem.h
	\date 17.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../math/TRect.h"
#include "IEngineSubsystem.h"
#include "Event.h"
#include <vector>
#include <string>
#include <functional>
#include <tuple>


namespace TDEngine2
{
	class ITimer;
	class IDLLManager;
	class IEventManager;
	class IImGUIContextVisitor;


	/*!
		struct TOnWindowResized

		\brief The structure represents an event which occurs
		when a sizes of a window were changed
	*/

	typedef struct TOnWindowResized : TBaseEvent
	{
		virtual ~TOnWindowResized() = default;

		TDE2_REGISTER_TYPE(TOnWindowResized)

		REGISTER_EVENT_TYPE(TOnWindowResized)

		U32 mWidth;

		U32 mHeight;
	} TOnWindowResized, *TOnWindowResizedPtr;


	/*!
		struct TOnWindowMoved

		\brief The structure represents an event which occurs
		when a window was moved. mX and mY members represents a 
		coordinates of top-left corner of a client area
	*/

	typedef struct TOnWindowMoved : TBaseEvent
	{
		virtual ~TOnWindowMoved() = default;

		TDE2_REGISTER_TYPE(TOnWindowMoved)

		REGISTER_EVENT_TYPE(TOnWindowMoved)

		U32 mX;

		U32 mY;
	} TOnWindowMoved, *TOnWindowMovedPtr;


	/*!
		interface IWindowSystem

		\brief The interface contains methods which help to configure,
		create and manage application's windows
	*/

	class IWindowSystem : public IEngineSubsystem
	{
		public:
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

			TDE2_API virtual E_RESULT_CODE Init(IEventManager* pEventManager, const std::string& name, U32 width, U32 height, U32 flags = 0x0) = 0;

			/*!
				\brief The method processes a window's update (executing messages, redraw content)

				\param[in] onFrameUpdate A callback that is invoked every time, when the window doesn't process a message

				\return The method returns false, if Quit method was invoked or the window was closed
				in other possible way, true in other cases
			*/

			TDE2_API virtual bool Run(const std::function<void()>& onFrameUpdate) = 0;

			/*!
				\brief The method stops the engine's main loop

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Quit() = 0;

			/*!
				\brief The method changes a window's title

				\param[in] title A new title's value 

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetTitle(const std::string& title) = 0;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				\return The method returns an object that contains internal handlers that are used by the system
			*/

			TDE2_API virtual const TWindowSystemInternalData& GetInternalData() const = 0;

			/*!
				\brief The method returns a width of a window's client area

				\return The method returns a width of a window's client area
			*/

			TDE2_API virtual U32 GetWidth() const = 0;

			/*!
				\brief The method returns a height of a window's client area

				\return The method returns a height of a window's client area
			*/

			TDE2_API virtual U32 GetHeight() const = 0;

			/*!
				\brief The method returns a pointer to a in-engine timer

				\return The method returns a pointer to a in-engine timer
			*/

			TDE2_API virtual ITimer* GetTimer() const = 0;

			/*!
				\brief The method returns a window's title

				\return The method returns a window's title
			*/

			TDE2_API virtual const std::string& GetTitle() const = 0;

			/*!
				\brief The method returns current set flags

				\return The method returns current set flags
			*/

			TDE2_API virtual U32 GetFlags() const = 0;

			/*!
				\brief The method returns a pointer to IDLLManager's implementation

				\return The method returns a pointer to IDLLManager's implementation
			*/

			TDE2_API virtual IDLLManager* GetDLLManagerInstance() const = 0;

			/*!
				\brief The method returns a pointer to IEventManager implementation

				\return The method returns a pointer to IEventManager implementation
			*/

			TDE2_API virtual IEventManager* GetEventManager() const = 0;

			/*!
				\brief The method returns an object of TRect type which contains
				information about sizes of the window and its position

				\return The method returns an object of TRect type which contains
				information about sizes of the window and its position
			*/

			TDE2_API virtual TRectU32 GetWindowRect() const = 0;

			/*!
				\brief The method returns sizes of internal area of the window. If
				the window is drawn without borders and title bar this output is the same
				as GetWindowRect()

				\return The method returns sizes of internal area of the window 
			*/

			TDE2_API virtual TRectU32 GetClientRect() const = 0;

			/*!
				\brief The method pass all the needed data into the immediate GUI context 
				
				\param[in, out] pVisitor A pointer to IImGUIContextVisitor implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureImGUIContext(IImGUIContextVisitor* pVisitor) const = 0;

#if TDE2_EDITORS_ENABLED
			/*!
				\brief The method displays platform specific dialog window that allows to select file to open

				\param[in] filters An array of filters, each filter is an extension

				\return Either an error code, or a path to the file
			*/

			TDE2_API virtual TResult<std::string> ShowOpenFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters) = 0;

			/*!
				\brief The method displays platform specific dialog window that allows to select place where the file should be saved 

				\param[in] filters An array of filters, each filter is an extension

				\return Either an error code, or a path to the file
			*/


			TDE2_API virtual TResult<std::string> ShowSaveFileDialog(const std::vector<std::tuple<std::string, std::string>>& filters) = 0;
#endif

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_WINDOW; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IWindowSystem)
	};
}
