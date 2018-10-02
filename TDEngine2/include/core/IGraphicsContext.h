/*!
	\file IGraphicsContext.h
	\date 20.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IEngineSubsystem.h"
#include "./../utils/Color.h"


namespace TDEngine2
{
	class IWindowSystem;


	/*!
		interface IGraphicsContext

		\brief The interface represents functionality of a low-level wrapper over GAPI

		\todo More proper set up of context's initial state should be done (depth/stencil buffer configuration,
		MSAA, etc)
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
				\brief The method clears up back buffer with specified color

				\param[in] color The new color of a back buffer
			*/

			TDE2_API virtual void ClearBackBuffer(const TColor32F& color) = 0;

			/*!
				\brief The method swaps a current buffer with a back one
			*/

			TDE2_API virtual void Present() = 0;

			/*!
				\brief The method sets up a viewport's parameters

				\param[in] x x position of left hand side of a viewport
				\param[in] y y position of left (top/bottom varies on GAPI) corner of a viewport
				\param[in] width width of a viewport
				\param[in] height height of a viewport
				\param[in] minDepth minimum depth of a viewport
				\param[in] maxDepth maximum depth of a viewport

			*/

			TDE2_API virtual void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth) = 0;

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