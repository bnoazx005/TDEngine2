/*!
	\file ILogger.h
	\date 25.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "Types.h"
#include "../core/IBaseObject.h"
#include <string>


namespace TDEngine2
{
	/*!
		interface ILogger

		\brief The interface represents a functionality of a logging system
	*/

	class ILogger: public virtual IBaseObject
	{
		public:
			TDE2_API virtual ~ILogger() = default;

			TDE2_API virtual void LogMessage(const std::string& message) = 0;

			TDE2_API virtual void LogStatus(const std::string& message, const std::string& status, C8 delimiter, U16 delimitersCount) = 0;

			TDE2_API virtual void LogError(const std::string& message) = 0;

			TDE2_API virtual void LogWarning(const std::string& message) = 0;
		protected:
			TDE2_API ILogger() = default;
			TDE2_API ILogger(const ILogger& builder) = delete;
			TDE2_API virtual ILogger& operator= (ILogger& builder) = delete;
	};
}