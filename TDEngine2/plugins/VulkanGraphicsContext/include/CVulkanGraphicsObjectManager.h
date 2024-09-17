/*!
	\file CVulkanGraphicsObjectManager.h
	\date 05.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseGraphicsObjectManager.h>
#include <unordered_map>
#include <vector>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>


namespace TDEngine2
{
	class CVulkanTextureImpl;
	class CVulkanBuffer;


	TDE2_DECLARE_SCOPED_PTR(CVulkanTextureImpl)
	TDE2_DECLARE_SCOPED_PTR(CVulkanBuffer)


	/*!
		\brief A factory function for creation objects of CVulkanGraphicsObjectManager's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanGraphicsObjectManager's implementation
	*/

	IGraphicsObjectManager* CreateVulkanGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CVulkanGraphicsObjectManager

		\brief The class is an implementation of IGraphicsObjectManager for the Vulkan graphics context
	*/

	class CVulkanGraphicsObjectManager : public CBaseGraphicsObjectManager
	{
		public:
			friend IGraphicsObjectManager* CreateVulkanGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			typedef std::vector<TPtr<CVulkanTextureImpl>> TNativeTexturesArray;
			typedef std::vector<TPtr<CVulkanBuffer>>      TNativeBuffersArray;
			typedef std::vector<VkSampler>                TTextureSamplersArray;
		public:
			TResult<TBufferHandleId> CreateBuffer(const TInitBufferParams& params) override;
			TResult<TTextureHandleId> CreateTexture(const TInitTextureImplParams& params) override;

			E_RESULT_CODE DestroyBuffer(TBufferHandleId bufferHandle) override;
			E_RESULT_CODE DestroyTexture(TTextureHandleId textureHandle) override;

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

			/*!
				\brief The method creates a new blend state which is configured via given paramters

				\param[in] blendStateDesc A structure that contains parameters which will be assigned to a new created state

				\return The result object contains either an identifier of created blend state or an error code
			*/

			 TResult<TBlendStateId> CreateBlendState(const TBlendStateDesc& blendStateDesc) override;

			/*!
				\brief The method creates a new state which sets up depth and stencil buffers operations and values

				\param[in] depthStencilDesc A structure that contains settings, which the state should set up, each time it's used

				\return The result object contains either an identifier of a created state or an error code
			*/

			TResult<TDepthStencilStateId> CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc) override;

			/*!
				\brief The method create a new state which specifies how a geometry should be actually drawn

				\param[in] rasterizerStateDesc A type that contains settings for a rasterizer

				\return The result object contains either an identifier of a created state or an error code
			*/

			TResult<TRasterizerStateId> CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc) override;

			TPtr<IBuffer> GetBufferPtr(TBufferHandleId handle) override;
			TPtr<CVulkanBuffer> GetVulkanBufferPtr(TBufferHandleId bufferHandle);

			TPtr<ITextureImpl> GetTexturePtr(TTextureHandleId handle) override;
			TPtr<CVulkanTextureImpl> GetVulkanTexturePtr(TTextureHandleId textureHandle);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanGraphicsObjectManager)

			E_RESULT_CODE _freeTextureSamplers() override;

			E_RESULT_CODE _freeBlendStates() override;

			E_RESULT_CODE _freeDepthStencilStates() override;

			E_RESULT_CODE _freeRasterizerStates() override;

			const std::string _getShaderCacheFilePath() const override;
		protected:
			TNativeTexturesArray  mpTexturesArray;
			TNativeBuffersArray   mpBuffersArray;

			TTextureSamplersArray mTextureSamplersArray;
	};
}