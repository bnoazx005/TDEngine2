/*!
	\file CUnixGLContextFactory.h
	\date 13.10.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "./../IOGLContextFactory.h"
#include <utils/Utils.h>


#if defined(TDE2_USE_UNIXPLATFORM)


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CUnixGLContextFactory's type

		\return A pointer to CUnixGLContextFactory's implementation
	*/

	TDE2_API IOGLContextFactory* CreateUnixGLContextFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);


	/*!
		class CUnixGLContextFactory

		\brief The implementation of IOGLContextFactory for UNIX platform
	*/

	class CUnixGLContextFactory : public IOGLContextFactory
	{
		public:
			friend TDE2_API IOGLContextFactory* CreateUnixGLContextFactory(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
		protected:
			typedef GLXContext (*TGLXCreateContextAttribsARBCallback)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
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

			TDE2_API E_RESULT_CODE CreateContextForWorkerThread() override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixGLContextFactory)
		protected:
			TOGLCtxHandler mCurrGLHandler;
			Display*       mpDisplayHandler;
			Window         mWindowHandler;
			bool           mIsInitialized;
	};
}

#endif