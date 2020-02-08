/*!
	\file CDevConsoleWindow.h
	\date 07.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IEditorWindow.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CDevConsoleWindow's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IEditorWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateDevConsoleWindow(E_RESULT_CODE& result);


	/*!
		class CDevConsoleWindow

		\brief The class is an implementation of a window for in-game level editor
	*/

	class CDevConsoleWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateDevConsoleWindow(E_RESULT_CODE& result);
		public:
			typedef std::vector<std::string> TStringsArray;
			typedef std::vector<std::tuple<std::string, bool>> TConsoleLogArray;
			typedef std::function<std::string(const TStringsArray&)> TCommandHandler;
			typedef std::unordered_map<std::string, TCommandHandler> TCommandsTable;
		public:
			/*!
				\brief The method initializes internal state of the editor

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method registers given function as a command with 'commandName' name

				\param[in] commandName A name of the command
				\param[in] commandHandler A function which implements command's behaviour over a given arguments

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterCommand(const std::string& commandName, const TCommandHandler& commandHandler);

			/*!
				\brief The method removes handle of the command with the given name if the latter one exists

				\param[in] commandName A name of the command

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UnregisterCommand(const std::string& commandName);

			/*!
				\brief The method evaluates input string line and executes a command if it exists

				\param[in] command A name of the command
				\param[in] args An array of arguments

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ExecuteCommand(const std::string& command, const TStringsArray& args);

			/*!
				\brief The method clears up existing log of the console window
			*/

			TDE2_API void ClearHistory();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDevConsoleWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;
		private:
			TDE2_API void _writeToLog(const std::string& message, bool isError = false);
		protected:
			TConsoleLogArray mLog;

			TCommandsTable   mRegisteredCommands;

			std::string      mCurrInputBuffer;

			U16              mCurrPos = 0;
	};
}

#endif