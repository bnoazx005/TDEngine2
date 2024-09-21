/*!
	\file CD3D12GCtxPlugin.h
	\date 21.09.2024
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <core/IPlugin.h>
#include <core/IGraphicsContext.h>


namespace TDEngine2
{
	class IEngineCore;
	class IGraphicsContext;


	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext)


	/*!
		class CD3D12GCtxPlugin

		\brief The class provides support of D3D12 GAPI
	*/

	class CD3D12GCtxPlugin : public CBaseObject, public IPlugin
	{
		public:
			friend TDE2_API TDEngine2::IPlugin* TDE2_APIENTRY ::CreatePlugin(TDEngine2::IEngineCore* pEngineCore, TDEngine2::E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a plugin

				\param[in] pEngineCore A pointer to IEngineCore's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			TDE2_API E_RESULT_CODE Init(IEngineCore* pEngineCore) override;

			/*!
				\brief The method returns an object, which contains all the information
				about the plugin. The structure TPluginInfo is declared in Types.h.

				\return The method returns an object, which contains all the information
				about the plugin
			*/

			TDE2_API const TPluginInfo& GetInfo() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12GCtxPlugin)
		protected:
			IEngineCore*           mpEngineCoreInstance;
			TPtr<IGraphicsContext> mpGraphicsContext;
	};
}