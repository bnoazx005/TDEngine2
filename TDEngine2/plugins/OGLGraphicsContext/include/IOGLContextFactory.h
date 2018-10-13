/*!
	\file COGLContextFactory.h
	\date 23.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include <utils/Types.h>

#if defined (TDE2_USE_UNIXPLATFORM)
#include <GL/glxew.h>
#endif


namespace TDEngine2
{
	class IWindowSystem;

	
#if defined(TDE2_USE_WIN32PLATFORM)
	typedef HGLRC TOGLCtxHandler;
#elif defined (TDE2_USE_UNIXPLATFORM)
	typedef GLXContext TOGLCtxHandler;
#else
	typedef void* TOGLCtxHandler;
#endif


	/*!
		interface IOGLContextFactory

		\brief The interface is used to create abstraction over a process of
		creation OGL context for different platform. The MAIN task, which it
		solves is a context creation, all calls of glX functions can be 
		executed using IOGLGraphicsContext
	*/

	class IOGLContextFactory
	{
		public:
			TDE2_API virtual ~IOGLContextFactory() = default;

			TDE2_API virtual E_RESULT_CODE Init(IWindowSystem* pWindowSystem) = 0;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;

			/*!
				\brief The method sets up internal context's value as current using one

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetContext() = 0;

			/*!
				\brief The method resets current set context. It's equal to call of
				xglMakeCurrent(NULL, NULL);

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ResetContext() = 0;

			/*!
				\brief The method returns a context's handler

				\return The method returns a context's handler
			*/

			TDE2_API virtual TOGLCtxHandler GetContext() const = 0;
		protected:
			TDE2_API IOGLContextFactory() = default;
			TDE2_API IOGLContextFactory(const IOGLContextFactory& graphicsCtx) = delete;
			TDE2_API virtual IOGLContextFactory& operator= (IOGLContextFactory& graphicsCtx) = delete;
	};
}