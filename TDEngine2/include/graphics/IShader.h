/*!
	\file IShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IResourceFactory.h"
#include "../core/IBaseObject.h"
#include "IShaderCompiler.h"
#include <unordered_map>
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IShaderCompiler;
	class ITexture;
	class IStructuredBuffer;
	class IShaderCache;
	class IBinaryFileReader;

	struct TShaderCompilerOutput;
	struct TShaderParameters;



	/*!
		interface IShader

		\brief The interface describes a functionality of a shader instance,
		 which unites all programmable stages of the graphics pipeline
	*/

	class IShader: public virtual IBaseObject
	{
		public:
			TDE2_REGISTER_TYPE(IShader);

			/*!
				\brief The method initializes an internal state of a shader

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method compiles specified source code into shader's bytecode using IShaderCompiler's 
				implementation

				\param[in] pShaderCompiler A pointer to IShaderCompiler's implementation
				\param[in] sourceCode A string that contains a source code of a shader (including all its stages)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Compile(const IShaderCompiler* pShaderCompiler, const std::string& sourceCode) = 0;

			/*!
				\brief The method tries to load bytecode from pShaderCache storage based on given shader's metadata. If some error
				appears the shader will be loaded using Compile method or some default instance will be created
			*/

			TDE2_API virtual E_RESULT_CODE LoadFromShaderCache(IShaderCache* pShaderCache, const TShaderParameters* pShaderMetaData) = 0;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			TDE2_API virtual void Bind() = 0;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			TDE2_API virtual void Unbind() = 0;
			
			/*!
				\brief The method writes data into a user-defined uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetUserUniformsBuffer(U8 slot, const U8* pData, USIZE dataSize) = 0;

			/*!
				\brief The method assigns a given texture to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code

				\param[in, out] pTexture A pointer to ITexture implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetTextureResource(const std::string& resourceName, ITexture* pTexture) = 0;

			/*!
				\brief The method assings a given structured buffer to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code
				\param[in, out] pBuffer A pointer to IStructuredBuffer implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetStructuredBufferResource(const std::string& resourceName, IStructuredBuffer* pBuffer) = 0;

			/*!
				\brief The method returns an additional information about the shader

				\return A pointer to the type that contains all the information about internal uniform buffers and
				used resources
			*/

			TDE2_API virtual const TShaderCompilerOutput* GetShaderMetaData() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShader)
	};


	typedef struct TShaderStageInfo
	{
		TShaderCacheBytecodeEntry mBytecodeInfo;
		std::string               mEntrypoint;
	} TShaderStageInfo, *TShaderStageInfoPtr;


	/*!
		struct TShaderParameters

		\brief The stucture contains fields for creation IShader objects
	*/

	typedef struct TShaderParameters : TBaseResourceParameters
	{
		/*!
			\brief The method deserializes object's state from given reader

			\param[in, out] pReader An input stream of data that contains information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

		/*!
			\brief The method serializes object's state into given stream

			\param[in, out] pWriter An output stream of data that writes information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

		std::unordered_map<std::string, TUniformBufferDesc>       mUniformBuffersInfo;		/// first key is a buffer's name, the value is the buffer's slot index and its size
		std::unordered_map<std::string, TShaderResourceDesc>      mShaderResourcesInfo;	/// the key is a resource's name, the value is an information about resource
		std::unordered_map<E_SHADER_STAGE_TYPE, TShaderStageInfo> mStages;
	} TShaderParameters, *TShaderParametersPtr;


	/*!
		interface IShaderFactory

		\brief The interface describes a functionality of IShader objects' factory
	*/

	class IShaderFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};


	/*!
		\brief The interface describes functionality of a shaders library
	*/

	class IShaderCache : public virtual IBaseObject
	{
		public:
			TDE2_API virtual E_RESULT_CODE Init(IBinaryFileReader* pCacheReader) = 0;

			TDE2_API virtual std::vector<U8> GetBytecode(const TShaderCacheBytecodeEntry& info) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShaderCache)
	};
}
