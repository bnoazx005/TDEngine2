/*!
	\file CVulkanVertexDeclaration.h
	\date 05.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CVertexDeclaration.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include <vector>


namespace TDEngine2
{
	class IVertexBuffer;


	/*!
		\brief A factory function for creation objects of CVulkanVertexDeclaration's type

		\return A pointer to CVulkanVertexDeclaration's implementation
	*/

	TDE2_API IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result);


	/*!
		class CVulkanVertexDeclaration

		\brief The class implements a vertex declaration for Vulkan
	*/

	class CVulkanVertexDeclaration : public CVertexDeclaration
	{
		public:
			friend TDE2_API IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffersArray An array of  IVertexBuffer implementations

				\param[in, out] pShader A pointer to IShader implementation
			*/

			TDE2_API void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CVulkanVertexDeclaration)

			E_RESULT_CODE _compile();
		private:
			std::vector<VkVertexInputBindingDescription> mInputBindings;
			std::vector<VkVertexInputAttributeDescription> mAttributeDescs;
			VkPipelineVertexInputStateCreateInfo mVertexFormatInfo;
	};
}