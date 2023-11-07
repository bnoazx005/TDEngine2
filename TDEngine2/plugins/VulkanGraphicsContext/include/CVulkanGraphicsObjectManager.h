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
#include "vk_mem_alloc.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CVulkanGraphicsObjectManager's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CVulkanGraphicsObjectManager's implementation
	*/

	TDE2_API IGraphicsObjectManager* CreateVulkanGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CVulkanGraphicsObjectManager

		\brief The class is an implementation of IGraphicsObjectManager for the Vulkan graphics context
	*/

	class CVulkanGraphicsObjectManager : public CBaseGraphicsObjectManager
	{
		public:
			friend TDE2_API IGraphicsObjectManager* CreateVulkanGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\param[in] pGraphicsContext A pointer to implementation of IGraphicsContext interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method is a factory for creation objects of IVertexBuffer's type

				\param[in] usageType A usage type of a buffer

				\param[in] totalBufferSize Total size of a buffer

				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return The result object contains either a pointer to IVertexBuffer or an error code
			*/

			TDE2_API TResult<IVertexBuffer*> CreateVertexBuffer(E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr) override;

			/*!
				\brief The method is a factory for creation objects of IIndexBuffer's type

				\param[in] usageType A usage type of a buffer

				\param[in] totalBufferSize Total size of a buffer

				\param[in] indexFormatType A value, which defines single index's stride size

				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return The result object contains either a pointer to IIndexBuffer or an error code
			*/

			TDE2_API TResult<IIndexBuffer*> CreateIndexBuffer(E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
				USIZE totalBufferSize, const void* pDataPtr) override;

			/*!
				\brief The method is a factory for creation objects of IConstantBuffer's type

				\param[in] usageType A usage type of a buffer

				\param[in] totalBufferSize Total size of a buffer

				\param[in] indexFormatType A value, which defines single index's stride size

				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return The result object contains either a pointer to IConstantBuffer or an error code
			*/

			TDE2_API TResult<IConstantBuffer*> CreateConstantBuffer(E_BUFFER_USAGE_TYPE usageType, USIZE totalBufferSize, const void* pDataPtr) override;

			/*!
				\brief The method is a factory for creation of structured buffers objects

				\return The result object that contains either a pointer to a structured buffer or an error code
			*/

			TDE2_API TResult<IStructuredBuffer*> CreateStructuredBuffer(const TStructuredBuffersInitParams& params) override;

			/*!
				\brief The method is a factory for creation objects of IVertexDeclaration's type

				\return The result object contains either a pointer to IVertexDeclaration or an error code
			*/

			TDE2_API TResult<IVertexDeclaration*> CreateVertexDeclaration() override;

			/*!
				\brief The method is a factory for creation of texture samplers objects

				\param[in] samplerDesc A structure that contains parameters which will be assigned to a new created sampler

				\return The result object contains either an identifier of created sampler or an error code
			*/

			TDE2_API TResult<TTextureSamplerId> CreateTextureSampler(const TTextureSamplerDesc& samplerDesc) override;

			/*!
				\brief The method creates a new blend state which is configured via given paramters

				\param[in] blendStateDesc A structure that contains parameters which will be assigned to a new created state

				\return The result object contains either an identifier of created blend state or an error code
			*/

			TDE2_API  TResult<TBlendStateId> CreateBlendState(const TBlendStateDesc& blendStateDesc) override;

			/*!
				\brief The method creates a new state which sets up depth and stencil buffers operations and values

				\param[in] depthStencilDesc A structure that contains settings, which the state should set up, each time it's used

				\return The result object contains either an identifier of a created state or an error code
			*/

			TDE2_API TResult<TDepthStencilStateId> CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc) override;

			/*!
				\brief The method create a new state which specifies how a geometry should be actually drawn

				\param[in] rasterizerStateDesc A type that contains settings for a rasterizer

				\return The result object contains either an identifier of a created state or an error code
			*/

			TDE2_API TResult<TRasterizerStateId> CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc) override;

			/*!
				\brief The method returns a string which contains full source code of default shader that is specific
				for the graphics context

				\param[in] type A type of a default shader

				\return The method returns a string which contains full source code of default shader that is specific
				for the graphics context
			*/

			TDE2_API std::string GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const override;

			/*!
				\brief The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates

				\return The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates
			*/

			TDE2_API std::array<TVector4, 3> GetScreenTriangleVertices() const override;

			TDE2_API VmaAllocator GetAllocator() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanGraphicsObjectManager)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;

			TDE2_API E_RESULT_CODE _freeTextureSamplers() override;

			TDE2_API E_RESULT_CODE _freeBlendStates() override;

			TDE2_API E_RESULT_CODE _freeDepthStencilStates() override;

			TDE2_API E_RESULT_CODE _freeRasterizerStates() override;

			TDE2_API const std::string _getShaderCacheFilePath() const override;
		protected:
			VmaAllocator mMainAllocator;
	};
}