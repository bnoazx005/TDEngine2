/*!
	\file CUnixInputCtxPlugin.h
	\date 29.01.2019
	\authors Kasimov Ildar
*/

#pragma once


#include <core/IPlugin.h>
#include <core/CBaseObject.h>


namespace TDEngine2
{
	class IEngineCore;
	class IInputContext;


	/*!
		class CUnixInputCtxPlugin

		\brief The class implements a plugin, which provides
		a support of a Xlib API based input context
	*/

	class CUnixInputCtxPlugin : public CBaseObject, public IPlugin
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUnixInputCtxPlugin)
		protected:
			IEngineCore*   mpEngineCoreInstance;

			IInputContext* mpInputContext;
	};
}