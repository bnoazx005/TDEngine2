/*!
	\file IGraphicsContext.h
	\date 20.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IEngineSubsystem.h"


namespace TDEngine2
{
	class IWindowSystem;


	/*!
		interface IGraphicsContext

		\brief The interface represents functionality of a low-level wrapper over GAPI
	*/

	class IGraphicsContext : public IEngineSubsystem
	{
		public:
			TDE2_API virtual ~IGraphicsContext() = default;

			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const IWindowSystem* pWindowSystem) = 0;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			TDE2_API virtual const TGraphicsCtxInternalData& GetInternalData() const = 0;
		protected:
			TDE2_API IGraphicsContext() = default;
			TDE2_API IGraphicsContext(const IGraphicsContext& graphicsCtx) = delete;
			TDE2_API virtual IGraphicsContext& operator= (IGraphicsContext& graphicsCtx) = delete;
	};
}