/*!
	\file IResourceManager.h
	\date 17.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IEngineSubsystem.h"
#include "./../utils/Utils.h"
#include "IResourceHandler.h"
#include <type_traits>


namespace TDEngine2
{
	class IResource;
	class IResourceHandler;
	class IResourceLoader;
	class IJobManager;


	/*!
		interface IResourceManager

		\brief The manager describes a functionality of a resource manager,
		that is used within the engine
	*/

	class IResourceManager : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an inner state of a resource manager
				
				\param[in] pJobManager A pointer to IJobManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IJobManager* pJobManager) = 0;

			/*!
				\brief The method registers specified resource loader within a manager

				\param[in] pResourceLoader A pointer to IResourceLoader's implementation

				\return The method returns an object, which contains a status of method's execution and
				an identifier of the registred loader
			*/

			TDE2_API virtual TRegisterLoaderResult RegisterLoader(const IResourceLoader* pResourceLoader) = 0;

			/*!
				\brief The method unregisters a resource loader with the specified identifier

				\param[in] resourceLoaderId An identifier of a resource loader

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE UnregisterLoader(const TResourceLoaderId& resourceLoaderId) = 0;

			/*!
				\brief The method loads specified type of a resource using its name

				\param[in] name A name of a resource that should be loaded

				\return A pointer to IResourceHandler, which encapsulates direct access to the resource
			*/

			template <typename T> 
			TDE2_API 
#if _HAS_CXX17
			std::enable_if_t<std::is_base_of_v<IResource, T>,IResourceHandler*>
#else
			typename std::enable_if<std::is_base_of<IResource, T>::value, IResourceHandler*>::type
#endif
			Load(const std::string& name)
			{
				return _loadResource(T::GetTypeId(), name);
			}

			/*!
				\brief The method returns a raw pointer to a resource based on specified handler

				\param[in] pResourceHandler A pointer to a resource's handler

				\return The method returns a raw pointer to a resource based on specified handler
			*/

			TDE2_API virtual IResource* GetResourceByHandler(const IResourceHandler* pResourceHandler) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceManager)

			TDE2_API virtual IResourceHandler* _loadResource(U32 resourceTypeId, const std::string& name) = 0;
	};
}
