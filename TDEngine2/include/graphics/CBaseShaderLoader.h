/*!
	\file CBaseShaderLoader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/CBaseObject.h"
#include "./../core/IResourceLoader.h"
#include "./../utils/Types.h"


namespace TDEngine2
{
	class IFileSystem;
	class IShaderCompiler;
	class IShaderCache;


	TDE2_DECLARE_SCOPED_PTR(IShaderCompiler)
	TDE2_DECLARE_SCOPED_PTR(IShaderCache)


	/*!
		\brief A factory function for creation objects of CBaseShaderLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseShaderLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateBaseShaderLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, 
													 TPtr<IShaderCompiler> pShaderCompiler, TPtr<IShaderCache> pShaderCache, E_RESULT_CODE& result);


	/*!
		class CBaseShaderLoader

		\brief The class is a common implementation of a shader loaded
	*/

	class CBaseShaderLoader : public CBaseObject, public IShaderLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBaseShaderLoader(IResourceManager*, IGraphicsContext*, IFileSystem*, TPtr<IShaderCompiler>, TPtr<IShaderCache>, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, 
										TPtr<IShaderCompiler> pShaderCompiler, TPtr<IShaderCache> pShaderCache) override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShaderLoader)
		protected:
			IResourceManager*      mpResourceManager = nullptr;

			IFileSystem*           mpFileSystem = nullptr;

			IGraphicsContext*      mpGraphicsContext = nullptr;

			TPtr<IShaderCompiler>  mpShaderCompiler = nullptr;

			mutable TPtr<IShaderCache> mpShaderCache = nullptr;
	};
}