/*!
	\file CDefaultEngineCoreBuilder.h
	\date 32.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include "IEngineCoreBuilder.h"


namespace TDEngine2
{
	class IWindowSystem;

	/*!
		class CDefaultEngineCoreBuilder

		\brief This class is a default implementation of IEngineCoreBuilder.
	*/

	class CDefaultEngineCoreBuilder: public IEngineCoreBuilder
	{
		public:
			friend TDE2_API IEngineCoreBuilder* CreateDefaultEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, E_RESULT_CODE& result);
		public:
			TDE2_API virtual ~CDefaultEngineCoreBuilder();

			/*!
				\brief The method initialized the builder's object

				\param[in] A callback to a factory's function of IEngineCore's objects

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method tries to configure graphics context based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ConfigureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type) override;

			/*!
				\brief The method tries to configure windows system based on specified parameters.
				The window system is chosen based on macro definitions, which are declared in Config.h file.

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ConfigureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags) override;

			/*!
				\brief The method returns a configured instances of an engine core

				\return The method returns a configured instances of an engine core
			*/

			TDE2_API IEngineCore* GetEngineCore() override;
		protected:
			TDE2_API CDefaultEngineCoreBuilder();
			TDE2_API CDefaultEngineCoreBuilder(const CDefaultEngineCoreBuilder& builder) = delete;
			TDE2_API virtual CDefaultEngineCoreBuilder& operator= (CDefaultEngineCoreBuilder& builder) = delete;
		protected:
			bool           mIsInitialized;
			IEngineCore*   mpEngineCoreInstance;
			IWindowSystem* mpWindowSystemInstance;
	};


	/*!
		\brief A factory function for creation objects of CDefaultEngineCoreBuilder's type

		\return A pointer to CDefaultEngineCoreBuilder's implementation
	*/

	TDE2_API IEngineCoreBuilder* CreateDefaultEngineCoreBuilder(TCreateEngineCoreCallback pEngineCoreFactoryCallback, E_RESULT_CODE& result);
}
