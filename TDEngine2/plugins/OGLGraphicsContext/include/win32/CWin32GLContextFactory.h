/*!
	\file CWin32GLContextFactory.h
	\date 23.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "./../IOGLContextFactory.h"
#include <utils/Utils.h>
#include <mutex>
#include <vector>
#include <atomic>


#if defined(TDE2_USE_WINPLATFORM)


namespace TDEngine2
{
	/*!
		class CWin32GLContextFactory

		\brief The implementation of IOGLContextFactory for Win32 platform
	*/

	class CWin32GLContextFactory: public IOGLContextFactory
	{
		public:
			friend TDE2_API IOGLContextFactory* CreateWin32GLContextFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
		protected:
			typedef TOGLCtxHandler (APIENTRY *TWGLCreateContextWithAttribsCallback)(HDC, HGLRC, const int *);
		public:
			/*!
				\brief The method initializes an object's state

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IWindowSystem* pWindowSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			TDE2_API E_RESULT_CODE CreateContextsForWorkerThreads(U32 maxThreadsCount) override;

			TDE2_API E_RESULT_CODE SetContextForWorkerThread() override;

			/*!
				\brief The method sets up internal context's value as current using one

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetContext() override;

			/*!
				\brief The method resets current set context. It's equal to call of
				xglMakeCurrent(NULL, NULL);

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ResetContext() override;

			/*!
				\brief The method returns a context's handler

				\return The method returns a context's handler
			*/

			TDE2_API TOGLCtxHandler GetContext() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CWin32GLContextFactory)

			TDE2_API TOGLCtxHandler _getTempContext(const HDC& hdc, E_RESULT_CODE& result);
		protected:
			TOGLCtxHandler              mCurrGLHandler;
			HDC                         mDeviceContextHandler;
			bool                        mIsInitialized;

			std::vector<TOGLCtxHandler> mWorkerThreadsContexts;
			std::atomic_uint            mLastUsedWorkerThreadContextCounter {0};

			mutable std::mutex          mMutex;
	};


	/*!
		\brief A factory function for creation objects of CWin32GLContextFactory's type

		\return A pointer to CWin32GLContextFactory's implementation
	*/

	TDE2_API IOGLContextFactory* CreateWin32GLContextFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
}

#endif