/*!
	\file COGLGraphicsContext.h
	\date 22.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include <core/IGraphicsContext.h>


namespace TDEngine2
{
	class IOGLContextFactory;

	/// A pointer to function that creates GL context based on platform
	typedef IOGLContextFactory* (*TCreateGLContextFactoryCallback)(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);


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
			friend TDE2_API IGraphicsContext* CreateOGLGraphicsContext(IWindowSystem* pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback, 
																		E_RESULT_CODE& result);
		public:
			TDE2_API virtual ~COGLGraphicsContext();

			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IWindowSystem* pWindowSystem) override;

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
				\brief The method sets up a viewport's parameters

				\param[in] x x position of left hand side of a viewport
				\param[in] y y position of left bottom corner of a viewport
				\param[in] width width of a viewport
				\param[in] height height of a viewport
				\param[in] minDepth minimum depth of a viewport
				\param[in] maxDepth maximum depth of a viewport
			*/

			TDE2_API void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth) override;

			/*!
				\brief The method computes a perspective projection matrix specific for a graphics context

				\param[in] fov A field of view

				\param[in] aspect An aspect ratio of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\return The method computes a perspective projection matrix specific for a graphics context
			*/

			TDE2_API TMatrix4 CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf) override;

			/*!
				\brief The method computes an orthographic projection matrix specific for a graphics context

				\param[in] left X coordinate of a left corner of a screen

				\param[in] top Y coordinate of a left top corner of a screen

				\param[in] right X coordinate of a right corner of a screen

				\param[in] bottom Y coordinate of a right bottom corner of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\return The method computes a orthographic projection matrix specific for a graphics context
			*/

			TDE2_API TMatrix4 CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf) override;

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

			/*!
				\brief The method returns a pointer to IGraphicsObjectManager implementation

				\return The method returns a pointer to IGraphicsObjectManager implementation
			*/

			TDE2_API IGraphicsObjectManager* GetGraphicsObjectManager() const override;
		protected:
			TDE2_API COGLGraphicsContext(TCreateGLContextFactoryCallback glContextFactoryCallback);
			TDE2_API COGLGraphicsContext(const COGLGraphicsContext& graphicsCtx) = delete;
			TDE2_API virtual COGLGraphicsContext& operator= (COGLGraphicsContext& graphicsCtx) = delete;
		protected:
			TGraphicsCtxInternalData        mInternalDataObject;
			bool                            mIsInitialized;
			IOGLContextFactory*             mpGLContextFactory;
			TCreateGLContextFactoryCallback mGLContextFactoryCallback;
			TWindowSystemInternalData       mWindowInternalData;

			IGraphicsObjectManager*         mpGraphicsObjectManager;
	};


	/*!
		\brief A factory function for creation objects of OGLGraphicsContext's type

		\return A pointer to OGLGraphicsContext's implementation
	*/

	TDE2_API IGraphicsContext* CreateOGLGraphicsContext(IWindowSystem* pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback,
														E_RESULT_CODE& result);
}