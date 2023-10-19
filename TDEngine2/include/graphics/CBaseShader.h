/*!
	\file CBaseShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseResource.h"
#include "IShader.h"
#include <vector>


namespace TDEngine2
{
	struct TShaderCompilerOutput;

	class IConstantBuffer;


	/*!
		class CBaseShader

		\brief The class is a common implementation for all platforms
	*/
	
	class CBaseShader: public IShader, public CBaseResource
	{
		public:
			typedef std::unordered_map<std::string, U8> TResourcesHashTable;
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

			TDE2_API E_RESULT_CODE LoadFromShaderCache(IShaderCache* pShaderCache, const TShaderParameters* pShaderMetaData) override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			TDE2_API void Bind() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;
			
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

			TDE2_API E_RESULT_CODE SetStructuredBufferResource(const std::string& resourceName, IStructuredBuffer* pBuffer) override;

			/*!
				\brief The method returns an additional information about the shader

				\return A pointer to the type that contains all the information about internal uniform buffers and
				used resources
			*/

			TDE2_API const TShaderCompilerOutput* GetShaderMetaData() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShader)

			TDE2_API virtual E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) = 0;

			TDE2_API virtual TShaderCompilerOutput* _createMetaDataFromShaderParams(IShaderCache* pShaderCache, const TShaderParameters* pShaderParams) = 0;

			TDE2_API E_RESULT_CODE _initShaderInternal(TShaderCompilerOutput* pShaderMetaData);

			TDE2_API virtual E_RESULT_CODE _freeUniformBuffers();

			TDE2_API virtual void _bindUniformBuffer(U32 slot, IConstantBuffer* pBuffer) = 0;

			TDE2_API virtual E_RESULT_CODE _createTexturesHashTable(const TShaderCompilerOutput* pCompilerData);
			TDE2_API virtual E_RESULT_CODE _createStructuredBuffersHashTable(const TShaderCompilerOutput* pCompilerData);

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		protected:
			IGraphicsContext*               mpGraphicsContext;

			std::string                     mSourceCode;

			std::vector<IConstantBuffer*>   mUniformBuffers;

			TResourcesHashTable             mTexturesHashTable;
			TResourcesHashTable             mStructuredBuffersHashTable;

			std::vector<ITexture*>          mpTextures;
			std::vector<IStructuredBuffer*> mpBuffers;

			TShaderCompilerOutput*          mpShaderMeta;
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
			/*!
				\brief The method initializes internal state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IBinaryFileReader* pCacheReader, IBinaryFileWriter* pCacheWriter) override;

			TDE2_API E_RESULT_CODE Dump() override;

			TDE2_API TResult<TShaderCacheBytecodeEntry> AddShaderBytecode(const std::vector<U8>& bytecode) override;

			TDE2_API std::vector<U8> GetBytecode(const TShaderCacheBytecodeEntry& info) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CShaderCache)
		private:
			IBinaryFileReader* mpCacheFileReader = nullptr;
			IBinaryFileWriter* mpCacheFileWriter = nullptr;

			std::vector<U8> mIntermediateCacheBuffer;

			bool mIsDirty = false;
	};
}
