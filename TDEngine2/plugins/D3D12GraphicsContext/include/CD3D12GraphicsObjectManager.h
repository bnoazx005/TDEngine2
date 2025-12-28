/*!
	\file CD3D12GraphicsObjectManager.h
	\date 05.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseGraphicsObjectManager.h>
#include <unordered_map>
#include <vector>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform


namespace TDEngine2
{
	class CD3D12TextureImpl;
	class CD3D12Buffer;
	class CD3D12ShaderImpl;
	class IGraphicsPipeline;
	struct TD3D12ResourceDescriptor;


	TDE2_DECLARE_SCOPED_PTR(CD3D12TextureImpl)
	TDE2_DECLARE_SCOPED_PTR(CD3D12Buffer)
	TDE2_DECLARE_SCOPED_PTR(CD3D12ShaderImpl)
	TDE2_DECLARE_SCOPED_PTR(IGraphicsPipeline)


	/*!
		\brief A factory function for creation objects of CD3D12GraphicsObjectManager's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in, out] pFileSystem A pointer to implementation of IFileSystem interface

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D12GraphicsObjectManager's implementation
	*/

	IGraphicsObjectManager* CreateD3D12GraphicsObjectManager(IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CD3D12GraphicsObjectManager

		\brief The class is an implementation of IGraphicsObjectManager for the D3D12 graphics context
	*/

	class CD3D12GraphicsObjectManager : public CBaseGraphicsObjectManager
	{
		public:
			friend IGraphicsObjectManager* CreateD3D12GraphicsObjectManager(IGraphicsContext*, IFileSystem*, E_RESULT_CODE&);
		public:
			typedef std::vector<TPtr<CD3D12TextureImpl>>  TNativeTexturesArray;
			typedef std::vector<TPtr<CD3D12Buffer>>       TNativeBuffersArray;
			typedef std::vector<TPtr<CD3D12ShaderImpl>>   TNativeShadersArray;
			typedef std::vector<TD3D12ResourceDescriptor> TTextureSamplersArray;
		public:
			E_RESULT_CODE DestroyBuffer(TBufferHandleId bufferHandle) override;
			E_RESULT_CODE DestroyTexture(TTextureHandleId textureHandle) override;
			E_RESULT_CODE DestroyShader(TShaderHandleId shaderHandle) override;

			/*!
				\brief The method is a factory for creation objects of IVertexDeclaration's type

				\return The result object contains either a pointer to IVertexDeclaration or an error code
			*/

			TResult<IVertexDeclaration*> CreateVertexDeclaration() override;

			/*!
				\brief The method is a factory for creation of texture samplers objects

				\param[in] samplerDesc A structure that contains parameters which will be assigned to a new created sampler

				\return The result object contains either an identifier of created sampler or an error code
			*/

			TResult<TTextureSamplerId> CreateTextureSampler(const TTextureSamplerDesc& samplerDesc) override;

			TPtr<IBuffer> GetBufferPtr(TBufferHandleId handle) override;
			TPtr<CD3D12Buffer> GetD3D12BufferPtr(TBufferHandleId bufferHandle);

			TPtr<ITextureImpl> GetTexturePtr(TTextureHandleId handle) override;
			TPtr<CD3D12TextureImpl> GetD3D12TexturePtr(TTextureHandleId textureHandle);

			TPtr<IShaderImpl> GetShaderPtr(TShaderHandleId handle) override;
			TPtr<CD3D12ShaderImpl> GetD3D12ShaderPtr(TShaderHandleId handle);

			/*!
				\return The method returns VkSampler handle or an error code
			*/

			TResult<TD3D12ResourceDescriptor> GetTextureSampler(TTextureSamplerId texSamplerId) const;

			/*!
				\brief The method returns a string which contains full source code of default shader that is specific
				for the graphics context

				\param[in] type A type of a default shader

				\return The method returns a string which contains full source code of default shader that is specific
				for the graphics context
			*/

			std::string GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const override;

			/*!
				\brief The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates

				\return The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates
			*/

			std::array<TVector4, 3> GetScreenTriangleVertices() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12GraphicsObjectManager)

			TPtr<IBuffer> _createBufferInternal(const TInitBufferParams& params) override;
			TPtr<ITextureImpl> _createTextureInternal(const TInitTextureImplParams& params) override;
			TPtr<IShaderImpl> _createShaderImplInternal(const std::string& shaderId) override;

			TPtr<IGraphicsPipeline> _createGraphicsPipelineInternal(const TGraphicsPipelineConfigDesc& pipelineConfigDesc) override;
			TPtr<IComputePipeline> _createComputePipelineInternal(const std::string& shaderId) override;

			USIZE _insertBuffer(TPtr<IBuffer> pObject) override;
			USIZE _insertTexture(TPtr<ITextureImpl> pObject) override;
			USIZE _insertShaderImpl(TPtr<IShaderImpl> pObject) override;

			E_RESULT_CODE _freeTextureSamplers() override;

			E_RESULT_CODE _freeBlendStates() override;

			E_RESULT_CODE _freeDepthStencilStates() override;

			E_RESULT_CODE _freeRasterizerStates() override;

			const std::string _getShaderCacheFilePath() const override;
		protected:
			TNativeTexturesArray  mpTexturesArray;
			TNativeBuffersArray   mpBuffersArray;
			TNativeShadersArray   mpShadersArray;

			TTextureSamplersArray mTextureSamplersArray;
	};
}

#endif