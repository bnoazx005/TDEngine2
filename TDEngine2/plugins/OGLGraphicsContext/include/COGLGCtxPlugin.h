/*!
	\file COGLGCtxPlugin.h
	\date 04.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "unix/CUnixGLContextFactory.h"
#include "win32/CWin32GLContextFactory.h"
#include "COGLBuffer.h"
#include "COGLConstantBuffer.h"
#include "COGLGraphicsContext.h"
#include "COGLIndexBuffer.h"
#include "COGLMappings.h"
#include "COGLShader.h"
#include "COGLShaderCompiler.h"
#include "COGLVertexBuffer.h"
#include "IOGLContextFactory.h"
#include "COGLTexture2D.h"
#include "COGLVertexDeclaration.h"
#include "COGLGraphicsObjectManager.h"
#include "COGLRenderTarget.h"
#include "COGLUtils.h"
#include "COGLCubemapTexture.h"
#include "COGLDepthBufferTarget.h"


#include <core/CBaseObject.h>
#include <core/IPlugin.h>


namespace TDEngine2
{
	class IEngineCore;
	class IGraphicsContext;


	/*!
		class COGLGCtxPlugin

		\brief The class implements a plugin, which provides
		a support of a OpenGL 3.3 graphics context
	*/

	class COGLGCtxPlugin : public CBaseObject, public IPlugin
	{
		public:
			friend TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY::CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a plugin

				\param[in] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			TDE2_API E_RESULT_CODE Init(IEngineCore* pEngineCore) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method returns an object, which contains all the information
				about the plugin. The structure TPluginInfo is declared in Types.h.

				\return The method returns an object, which contains all the information
				about the plugin
			*/

			TDE2_API const TPluginInfo& GetInfo() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLGCtxPlugin)

			TDE2_API E_RESULT_CODE _registerFactories(IEngineCore* pEngineCore);

			TDE2_API E_RESULT_CODE _registerResourceLoaders(IEngineCore* pEngineCore);
		protected:
			IEngineCore*      mpEngineCoreInstance;

			IGraphicsContext* mpGraphicsContext;

			TPluginInfo       mPluginInfo;
	};
}