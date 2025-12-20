/*!
	\file CBaseShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseResource.h"
#include "IShader.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <tuple>


namespace TDEngine2
{
	class IGraphicsObjectManager;
	struct TShaderCompilerOutput;


	enum class TBufferHandleId : U32;
	enum class TShaderHandleId : U32;


	TDE2_API IShaderResource* CreateShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,  E_RESULT_CODE& result);

	/*!
		class CBaseShader

		\brief The class is a common implementation for all platforms
	*/
	
	class CBaseShader: public IShaderResource, public CBaseResource
	{
		public:
			typedef std::unordered_map<std::string, USIZE> TResourcesHashTable;
		public:
			friend TDE2_API IShaderResource* CreateShader(IResourceManager*, IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CBaseShader)
			TDE2_REGISTER_TYPE(CBaseShader)

			/*!
				\brief The method initializes an internal state of a shader

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method initiates the process of shader's compilation. Firstly its source code is loaded if there is no information in the shader cache.
				Then the text is compiled and a new instance of a shader is stored within IGraphicsObjectManager's instance

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RequestShaderImpl() override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			TDE2_API void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			TDE2_API void Unbind() override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;
			
			/*!
				\brief The method writes data into a user-defined uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetUserUniformsBuffer(U8 slot, const U8* pData, USIZE dataSize) override;

			/*!
				\brief The method assigns a given texture to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code

				\param[in, out] pTexture A pointer to ITexture implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetTextureResource(const std::string& resourceName, ITexture* pTexture) override;

			/*!
				\brief The method assings a given structured buffer to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code
				\param[in, out] pBuffer A pointer to IStructuredBuffer implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetStructuredBufferResource(const std::string& resourceName, TBufferHandleId bufferHandle) override;

			/*!
				\brief The method returns an additional information about the shader

				\return A pointer to the type that contains all the information about internal uniform buffers and
				used resources
			*/

			TDE2_API const TShaderCompilerOutput* GetShaderMetaData() const override;

			TDE2_API U32 GetResourceBindingSlot(const std::string& resourceName) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShader)

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		protected:
			IGraphicsObjectManager* mpGraphicsObjectManager = nullptr;
			TPtr<IShaderImpl>       mpShaderImpl = nullptr;
			TShaderHandleId         mHandle;
	};


	TDE2_API IResourceFactory* CreateBaseShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CBaseShaderFactory
	*/

	class CBaseShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateBaseShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShaderFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		class CBaseShaderImpl
	*/

	class CBaseShaderImpl : public IShaderImpl, public CBaseObject
	{
		public:
			typedef std::unordered_map<std::string, USIZE> TResourcesHashTable;
		public:
			/*!
				\brief The method initializes an internal state of a shader

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] shaderId An identifier of a shader

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const std::string& shaderId) override;

			/*!
				\brief The method compiles specified source code into shader's bytecode using IShaderCompiler's
				implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation
				\param[in] sourceCode A string that contains a source code of a shader (including all its stages)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Compile(const IShaderCompiler* pShaderCompiler, const std::string& sourceCode) override;

			/*!
				\brief The method tries to load bytecode from pShaderCache storage based on given shader's metadata. If some error
				appears the shader will be loaded using Compile method or some default instance will be created
			*/

			TDE2_API E_RESULT_CODE LoadFromShaderCache(IShaderCache* pShaderCache) override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			TDE2_API void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			TDE2_API void Unbind() override;

			/*!
				\brief The method writes data into a user-defined uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetUserUniformsBuffer(U8 slot, const U8* pData, USIZE dataSize) override;

			/*!
				\brief The method assigns a given texture to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code

				\param[in, out] pTexture A pointer to ITexture implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetTextureResource(const std::string& resourceName, ITexture* pTexture) override;

			/*!
				\brief The method assings a given structured buffer to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code
				\param[in, out] pBuffer A pointer to IStructuredBuffer implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetStructuredBufferResource(const std::string& resourceName, TBufferHandleId bufferHandle) override;

			TDE2_API E_RESULT_CODE SetHandle(TShaderHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey) override;

			/*!
				\brief The method returns an additional information about the shader

				\return A pointer to the type that contains all the information about internal uniform buffers and
				used resources
			*/

			TDE2_API const TShaderCompilerOutput* GetShaderMetaData() const override;

			TDE2_API U32 GetResourceBindingSlot(const std::string& resourceName) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShaderImpl)

			TDE2_API virtual E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) = 0;

			TDE2_API E_RESULT_CODE _initShaderInternal(TShaderCompilerOutput* pShaderMetaData);

			TDE2_API virtual E_RESULT_CODE _createTexturesHashTable(const TShaderCompilerOutput* pCompilerData);
			TDE2_API virtual E_RESULT_CODE _createStructuredBuffersHashTable(const TShaderCompilerOutput* pCompilerData);

			TDE2_API virtual void _bindUniformBuffer(U32 slot, TBufferHandleId uniformsBufferHandle);
		protected:
			IGraphicsContext*                                   mpGraphicsContext = nullptr;

			std::string                                         mSourceCode;
			std::string                                         mName;

			std::vector<TBufferHandleId>                        mUniformBuffers{};

			TResourcesHashTable                                 mTexturesHashTable{};
			TResourcesHashTable                                 mStructuredBuffersHashTable{};

			std::vector<std::tuple<ITexture*, U16, bool>>       mpTextures{};
			std::vector<std::tuple<TBufferHandleId, U16, bool>> mBufferHandles{};

			std::unique_ptr<TShaderCompilerOutput>              mpShaderMeta = nullptr;

			TShaderHandleId                                     mHandle;
	};


	/*!
		\brief A factory function for creation objects of CShaderCache's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CShaderCache's implementation
	*/

	TDE2_API IShaderCache* CreateShaderCache(IBinaryFileReader* pCacheReader, IBinaryFileWriter* pCacheWriter, E_RESULT_CODE& result);


	/*!
		class CShaderCache
	*/

	class CShaderCache : public IShaderCache, public CBaseObject
	{
		public:
			friend TDE2_API IShaderCache* CreateShaderCache(IBinaryFileReader*, IBinaryFileWriter*, E_RESULT_CODE&);
		public:
			typedef std::unordered_map<std::string, std::unique_ptr<TShaderCompilerOutput>> TShadersDataTable;
		public:
			/*!
				\brief The method initializes internal state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IBinaryFileReader* pCacheReader, IBinaryFileWriter* pCacheWriter) override;

			TDE2_API E_RESULT_CODE Dump() override;

			TDE2_API E_RESULT_CODE AddShaderEntity(const std::string& shaderId, const TShaderCompilerOutput* pShaderCompiledData) override;

			TDE2_API TShaderCompilerOutput* GetShaderMetaData(const std::string& shaderId) override;
			TDE2_API bool HasShaderMetaData(const std::string& shaderId) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CShaderCache)

			E_RESULT_CODE _readShadersMetaTable();
		private:
			IBinaryFileReader* mpCacheFileReader = nullptr;
			IBinaryFileWriter* mpCacheFileWriter = nullptr;

			TShadersDataTable mShadersInfoTable;

			bool mIsDirty = false;
	};


	/*!
		\brief A factory function for creation objects of CBaseShaderLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseShaderLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateBaseShaderLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CBaseShaderLoader

		\brief The class is a common implementation of a shader loaded
	*/

	class CBaseShaderLoader : public CBaseObject, public IShaderLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBaseShaderLoader(IResourceManager*, IGraphicsContext*, IFileSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

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
			IResourceManager* mpResourceManager = nullptr;
			IFileSystem*      mpFileSystem = nullptr;
			IGraphicsContext* mpGraphicsContext = nullptr;
	};
}
