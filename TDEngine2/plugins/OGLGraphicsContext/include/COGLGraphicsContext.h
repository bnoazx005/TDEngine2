/*!
	\file COGLGraphicsContext.h
	\date 22.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include <core/IGraphicsContext.h>
#include <core/Event.h>
#include <core/CBaseObject.h>


namespace TDEngine2
{
	class IOGLContextFactory;
	class IEventManager;
	class IRenderTarget;
	class COGLGraphicsObjectManager;


	TDE2_DECLARE_SCOPED_PTR(IGraphicsObjectManager)


	/// A pointer to function that creates GL context based on platform
	typedef IOGLContextFactory* (*TCreateGLContextFactoryCallback)(IWindowSystem* pWindowSystem, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of OGLGraphicsContext's type

		\return A pointer to OGLGraphicsContext's implementation
	*/

	IGraphicsContext* CreateOGLGraphicsContext(TPtr<IWindowSystem> pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback,
														E_RESULT_CODE& result);
}