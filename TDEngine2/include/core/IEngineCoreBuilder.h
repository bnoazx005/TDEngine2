/*!
\file IEngineCoreBuilder.h
\date 32.09.2018
\authors Kasimov Ildar
*/
#pragma once


#include "./../utils/Types.h"
#include <string>


namespace TDEngine2
{
	class IEngineCore;


	typedef IEngineCore* (*TCreateEngineCoreCallback)(E_RESULT_CODE& result); /// A callback type of IEngineCore's factory function 


	/*!
		interface IEngineCoreBuilder

		\brief The interface describes a functionality of IEngineCore's builder that
		is intended to simplify configuration process of an engine's core
	*/

	class IEngineCoreBuilder
	{
		public:
			TDE2_API virtual ~IEngineCoreBuilder() = default;
			
			/*!
				\brief The method initialized the builder's object

				\param[in] A callback to a factory's function of IEngineCore's objects

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TCreateEngineCoreCallback pEngineCoreFactoryCallback) = 0;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Free() = 0;

			/*!
				\brief The method tries to configure graphics context based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureGraphicsContext(E_GRAPHICS_CONTEXT_GAPI_TYPE type) = 0;

			/*!
				\brief The method tries to configure windows system based on specified parameters

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureWindowSystem(const std::string& name, U32 width, U32 height, U32 flags) = 0;

			/*!
				\brief The method returns a configured instances of an engine core

				\return The method returns a configured instances of an engine core
			*/

			TDE2_API virtual IEngineCore* GetEngineCore() = 0;
		protected:
			TDE2_API IEngineCoreBuilder() = default;
			TDE2_API IEngineCoreBuilder(const IEngineCoreBuilder& builder) = delete;
			TDE2_API virtual IEngineCoreBuilder& operator= (IEngineCoreBuilder& builder) = delete;
	};
}
