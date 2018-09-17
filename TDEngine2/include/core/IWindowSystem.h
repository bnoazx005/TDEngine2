/*!
	\file IWindowSystem.h
	\date 17.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IEngineSubsystem.h"
#include <string>


namespace TDEngine2
{
	/*!
		interface IWindowSystem

		\brief The interface contains methods which help to configure,
		create and manage application's windows
	*/

	class TDE2_API IWindowSystem : public IEngineSubsystem
	{
		public:
			virtual ~IWindowSystem() = default;

			/*!
				\brief The method initializes a main window

				\param[in] name A name of a main window
				\param[in] width A window's width
				\param[in] height A window's height
				\param[in] flags An additional flags (bitwise value) lets configure
				additional settings of a window is created

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			virtual E_RESULT_CODE Init(const std::string& name, U32 width, U32 height, U32 flags = 0x0) = 0;

			/*!
				\brief The method processes a window's update (executing messages, redraw content)

				\return The method returns false, if Quit method was invoked or the window was closed
				in other possible way, true in other cases
			*/

			virtual bool Run() = 0;

			/*!
				\brief The method stops the engine's main loop

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			virtual E_RESULT_CODE Quit() = 0;
		protected:
			IWindowSystem() = default;
			IWindowSystem(const IWindowSystem& windowSystem) = delete;
			virtual IWindowSystem& operator= (IWindowSystem& windowSystem) = delete;
	};
}
