/*!
	\file IResourceLoader.h
	\date 17.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
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
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE LoadResource(IResource* pResource) const = 0;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API virtual U32 GetResourceTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceLoader)
	};


	class IGraphicsContext;
	class IFileSystem;
	class IShaderCompiler;


	/*!
		interface IShaderLoader

		\brief The interface describes a functionality of a shaders loader
	*/

	class IShaderLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object
				
				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			
			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, 
												const IShaderCompiler* pShaderCompiler) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShaderLoader)
	};
}
