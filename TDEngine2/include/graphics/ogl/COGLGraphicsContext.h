/*!
	\file COGLGraphicsContext.h
	\date 22.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "./../../core/IGraphicsContext.h"


namespace TDEngine2
{
	class IOGLContextFactory;

	/// A pointer to function that creates GL context based on platform
	typedef IOGLContextFactory* (*TCreateGLContextFactoryCallback)(const IWindowSystem* pWindowSystem, E_RESULT_CODE& result);


	/*!
		class COGLGraphicsContext

		\brief The class is an implementation of IGraphicsContext interface that
		wraps up low-level calls of OpenGL library

		\todo Not the best idea to pass GL context's factory function into constructor was used, should resolve this injection
		some another way
	*/

	class COGLGraphicsContext : public IGraphicsContext
	{
	public:
		friend TDE2_API IGraphicsContext* CreateOGLGraphicsContext(const IWindowSystem* pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback, 
																	E_RESULT_CODE& result);
	public:
		TDE2_API virtual ~COGLGraphicsContext();

		/*!
			\brief The method initializes an initial state of the object

			\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Init(const IWindowSystem* pWindowSystem) override;

		/*!
			\brief The method frees all memory occupied by the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Free() override;

		/*!
			\brief The method clears up back buffer with specified color

			\param[in] color The new color of a back buffer
		*/

		TDE2_API void ClearBackBuffer(const TColor32F& color) override;	

		/*!
			\brief The method swaps a current buffer with a back one
		*/

		TDE2_API void Present() override;

		/*!
			\brief The method returns an object that contains internal handlers that are used by the system.

			The structure of the object and its members can vary on different platforms.

			return The method returns an object that contains internal handlers that are used by the system
		*/

		TDE2_API const TGraphicsCtxInternalData& GetInternalData() const override;

		/*!
			\brief The method returns a type of the subsystem

			\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
		*/

		TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;
	protected:
		TDE2_API COGLGraphicsContext(TCreateGLContextFactoryCallback glContextFactoryCallback);
		TDE2_API COGLGraphicsContext(const COGLGraphicsContext& graphicsCtx) = delete;
		TDE2_API virtual COGLGraphicsContext& operator= (COGLGraphicsContext& graphicsCtx) = delete;
	protected:
		TGraphicsCtxInternalData        mInternalDataObject;
		bool                            mIsInitialized;
		IOGLContextFactory*             mGLContextFactory;
		TCreateGLContextFactoryCallback mGLContextFactoryCallback;
		TWindowSystemInternalData       mWindowInternalData;
	};


	/*!
		\brief A factory function for creation objects of OGLGraphicsContext's type

		\return A pointer to OGLGraphicsContext's implementation
	*/

	TDE2_API IGraphicsContext* CreateOGLGraphicsContext(const IWindowSystem* pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback,
														E_RESULT_CODE& result);
}