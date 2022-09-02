/*!
	\file IResourceLoader.h
	\date 17.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "IBaseObject.h"


namespace TDEngine2
{
	class IResourceManager;
	class IResource;


	/*!
		interface IResourceLoader

		\brief The interface describes a functionality of a loader of a
		specific resource type
	*/

	class IResourceLoader: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method loads data into the specified resource based on its internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE LoadResource(IResource* pResource) const = 0;

			/*!
				\return The method returns an identifier of a resource's type, which the loader serves
			*/

			TDE2_API virtual TypeId GetResourceTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceLoader)
	};


	class IGraphicsContext;
	class IFileSystem;
	class IShaderCompiler;


	TDE2_DECLARE_SCOPED_PTR(IShaderCompiler)


	/*!
		\brief The template is used to declare interfaces of loaders for concrete resources types
	*/

	template <typename... TInitArgs>
	class IGenericResourceLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TInitArgs... args) = 0;
	};


	/*!
		interface IShaderLoader

		\brief The interface describes a functionality of a shaders loader
	*/

	class IShaderLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*, TPtr<IShaderCompiler>> {};
}
