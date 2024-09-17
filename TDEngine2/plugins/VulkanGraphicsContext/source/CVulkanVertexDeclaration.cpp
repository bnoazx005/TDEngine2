#include "../include/CVulkanVertexDeclaration.h"
#include "../include/CVulkanMappings.h"
#include <utils/Utils.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include <vector>


namespace TDEngine2
{
	/*!
		class CVulkanVertexDeclaration

		\brief The class implements a vertex declaration for Vulkan
	*/

	class CVulkanVertexDeclaration : public CVertexDeclaration
	{
		public:
			friend IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffersArray An array of  IVertexBuffer implementations

				\param[in, out] pShader A pointer to IShader implementation
			*/

			void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanVertexDeclaration)

			E_RESULT_CODE _compile();
		private:
			std::vector<VkVertexInputBindingDescription> mInputBindings;
			std::vector<VkVertexInputAttributeDescription> mAttributeDescs;
			VkPipelineVertexInputStateCreateInfo mVertexFormatInfo;
	};


	CVulkanVertexDeclaration::CVulkanVertexDeclaration() :
		CVertexDeclaration()
	{
	}

	void CVulkanVertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader)
	{
	}

	E_RESULT_CODE CVulkanVertexDeclaration::_compile()
	{
		E_RESULT_CODE result = RC_OK;

		if (!mAttributeDescs.empty())
		{
			return result;
		}

		U32 currInstancingElementIndex = (std::numeric_limits<U32>::max)();
		U32 currInstancesPerData = 0;

		TInstancingInfoArray::const_iterator instancingIter = mInstancingInfo.cbegin();

		if (!mInstancingInfo.empty())
		{
			std::tie(currInstancingElementIndex, currInstancesPerData) = *instancingIter;
		}

		VkVertexInputAttributeDescription currAttributeDesc;

		U32 currIndex = 0;
		U32 currOffset = 0;

		/// fill in elements vector
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{			
			if (currIndex == currInstancingElementIndex) /// a new instancing division has found
			{
				currOffset = 0;
				currIndex = 0;

				VkVertexInputBindingDescription inputBindings{};
				inputBindings.binding = currInstancingElementIndex;
				inputBindings.stride = GetStrideSize(iter->mSource);
				inputBindings.inputRate = iter->mIsPerInstanceData ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
				
				mInputBindings.emplace_back(inputBindings);

				if (instancingIter + 1 == mInstancingInfo.cend())
				{
					currInstancingElementIndex = (std::numeric_limits<U32>::max)();
				}
				else
				{
					std::tie(currInstancingElementIndex, currInstancesPerData) = *(++instancingIter); /// retrieve next division's info
				}
			}
			
			currAttributeDesc = {};

			currAttributeDesc.binding = static_cast<U32>(currInstancingElementIndex);
			currAttributeDesc.location = currIndex;
			currAttributeDesc.format = CVulkanMappings::GetInternalFormat(iter->mFormatType);
			currAttributeDesc.offset = currOffset;

			mAttributeDescs.emplace_back(currAttributeDesc);

			currOffset += CFormatUtils::GetFormatSize((*iter).mFormatType);
		}

		mVertexFormatInfo = {};
		mVertexFormatInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		mVertexFormatInfo.pVertexAttributeDescriptions = mAttributeDescs.data();
		mVertexFormatInfo.vertexAttributeDescriptionCount = static_cast<U32>(mAttributeDescs.size());

		mVertexFormatInfo.pVertexBindingDescriptions = mInputBindings.data();
		mVertexFormatInfo.vertexBindingDescriptionCount = static_cast<U32>(mInputBindings.size());

		return result;
	}


	IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IVertexDeclaration, CVulkanVertexDeclaration, result);
	}
}