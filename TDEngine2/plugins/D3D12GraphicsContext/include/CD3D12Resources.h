/*!
	\file CD3D12Resources.h
	\date 26.09.2024
	\authors Kasimov Ildar
*/
#pragma once


#include <core/CBaseObject.h>
#include <utils/CContainers.h>
#include <graphics/IBuffer.h>
#include <graphics/ITexture.h>
#include <graphics/CBaseShader.h>
#include <graphics/BasePipelines.h>
#include "CD3D12GraphicsContext.h"


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

#include <d3d12.h>
#include <wrl.h>


template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;


namespace D3D12MA
{
	class Allocation;
}


namespace TDEngine2
{
	class CD3D12GraphicsContext;
	class CD3D12GraphicsObjectManager;
	class IVertexDeclaration;


	/*!
		class CD3D12Buffer

		\brief The class is an implementation of a common D3D12 buffer, which can be used in different situations like
		vertex / index / uniform / storage / etc
	*/

	class CD3D12Buffer : public CBaseObject, public IBuffer
	{
		public:
			friend IBuffer* CreateD3D12Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params);

			/*!
				\brief The method locks a buffer to provide safe data reading/writing

				\param[in] mapType A way the data should be processed during a lock

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Map(E_BUFFER_MAP_TYPE mapType, USIZE offset = 0) override;

			/*!
				\brief The method unlocks a buffer, so GPU can access to it after that operation
			*/

			void Unmap() override;

			E_RESULT_CODE Transition(E_RESOURCE_LAYOUT newLayout) override;

			/*!
				\brief The method writes data into a buffer

				\param[in] pData A pointer to data, which should be written
				\param[in] count A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Write(const void* pData, USIZE size) override;

			/*!
				\brief The method returns a pointer to buffer's data

				\return The method returns a pointer to buffer's data
			*/

			void* Read() override;

			/*!
				\brief The method recreates a buffer with a new specified size all previous data will be discarded
			*/

			E_RESULT_CODE Resize(USIZE newSize) override;

			E_RESULT_CODE SetHandle(TBufferHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey) override;

			/*!
				\brief The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers

				\return The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers
			*/

			void* GetInternalData() override;

			/*!
				\brief The method returns buffer's size in bytes

				\return The method returns buffer's size in bytes
			*/

			USIZE GetSize() const override;

			ComPtr<ID3D12Resource> GetHandle() const;
			D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const;

			const TD3D12ResourceDescriptor& GetUnorderedAccessViewHandle() const;
			const TD3D12ResourceDescriptor& GetShaderResourceViewHandle() const;

			const TInitBufferParams& GetParams() const override;

			E_RESOURCE_LAYOUT GetLayout() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12Buffer)

			E_RESULT_CODE _discardCurrentBuffer(USIZE newSize, E_STRUCTURED_BUFFER_TYPE structuredBufferType, USIZE elementStrideSize);
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			USIZE                    mBufferSize = 0;

			E_BUFFER_USAGE_TYPE      mBufferUsageType = E_BUFFER_USAGE_TYPE::DEFAULT;

			E_BUFFER_TYPE            mBufferType = E_BUFFER_TYPE::GENERIC;

			void*                    mpMappedBufferData = nullptr;

			TBufferInternalData      mBufferInternalData{};

	#if TDE2_DEBUG_MODE
			U8                       mLockChecker = 0;
	#endif

			CD3D12GraphicsContext*   mpGraphicsContextImpl = nullptr;

			ComPtr<ID3D12Resource>   mpResource = nullptr;
			D3D12MA::Allocation*     mpAllocation = nullptr;

			bool                     mIsUnorderedAccessResource = false;

			TD3D12ResourceDescriptor mConstantBufferView{};
			TD3D12ResourceDescriptor mUnorderedAccessView{};
			TD3D12ResourceDescriptor mShaderResourceView{};

			TInitBufferParams        mInitParams{};
			TBufferHandleId          mHandle;

			E_RESOURCE_LAYOUT        mCurrLayout;
	};


	/*!
		\brief A factory function for creation objects of CD3D12GraphicsContext's type

		\return A pointer to CD3D12GraphicsContext's implementation
	*/

	IBuffer* CreateD3D12Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result);


	TDE2_DECLARE_SCOPED_PTR(CD3D12Buffer)


	/*!
		\brief A factory function for creation objects of CD3D12Shader's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D12Shader's implementation
	*/

	IShader* CreateD3D12Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	struct TD3D12PipelineLayoutInfo
	{
		struct TBindingInfo
		{
			enum class E_TYPE { BUFFER, RAW_BUFFER, TEXTURE };

			U32    mSlot = 0;
			E_TYPE mType = E_TYPE::TEXTURE;
		};

		std::vector<U32>          mCBVActiveSlots {};
		std::vector<TBindingInfo> mSRVActiveSlots {};
		std::vector<TBindingInfo> mUAVActiveSlots {};
		std::vector<U32>          mSamplersActiveSlots {};

		U32                       mResourcesTableRootIndex = std::numeric_limits<U32>::max();
		U32                       mSamplersTableRootIndex = std::numeric_limits<U32>::max();
	};


	/*!
		class CD3D12Shader

		\brief The class is a common implementation for all platforms
	*/

	class CD3D12Shader : public CBaseShader
	{
		public:
			friend IShader* CreateD3D12Shader(IResourceManager*, IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CD3D12Shader)

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Reset() override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			void Unbind() override;

			const D3D12_SHADER_BYTECODE& GetPipelineShaderStage(E_SHADER_STAGE_TYPE stageType) const;

			ComPtr<ID3D12RootSignature> GetRootSignature() const;

			const TD3D12PipelineLayoutInfo& GetLayoutInfo() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12Shader)

			E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;
			E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);
			E_RESULT_CODE _createRootSignature(const TShaderCompilerOutput* pCompilerData);

		protected:
			std::array<D3D12_SHADER_BYTECODE, SST_NONE>  mStagesBytecode {};
			ComPtr<ID3D12RootSignature>                  mpRootSignature = nullptr;
			TD3D12PipelineLayoutInfo                     mLayoutInfo{};
	};


	/*!
		\brief A factory function for creation objects of CD3D12ShaderFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D12ShaderFactory's implementation
	*/

	IResourceFactory* CreateD3D12ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	ITextureImpl* CreateD3D12TextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class CD3D12TextureImpl
	*/

	class CD3D12TextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend ITextureImpl* CreateD3D12TextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;
			E_RESULT_CODE Transition(E_RESOURCE_LAYOUT newLayout) override;

			E_RESULT_CODE SetHandle(TTextureHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey) override;

			ComPtr<ID3D12Resource> GetHandle() const;

			const TD3D12ResourceDescriptor& GetUnorderedAccessViewHandle(U32 subresourceIndex = (std::numeric_limits<U32>::max)()) const;
			const TD3D12ResourceDescriptor& GetShaderResourceDescriptor() const;
			const TD3D12ResourceDescriptor& GetRenderTargetDescriptor() const;
			const TD3D12ResourceDescriptor& GetDepthBufferDescriptor() const;

			Vector<U8> ReadBytes(U32 index) override;

			E_RESOURCE_LAYOUT GetLayout() const override;

			const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12TextureImpl)

			E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			CD3D12GraphicsContext*                 mpGraphicsContextImpl = nullptr;

			TInitTextureImplParams                 mInitParams;
			TTextureHandleId                       mHandle;

			ComPtr<ID3D12Resource>                 mpResource = nullptr;
			D3D12MA::Allocation*                   mpAllocation = nullptr;

			TD3D12ResourceDescriptor               mShaderResourceDescriptor{};
			CFixedVector<TD3D12ResourceDescriptor> mUnorderedAccessViewDescriptors{};
			TD3D12ResourceDescriptor               mRenderTargetDescriptor{}; // IsValid() is true if it's active. The condition is correct for all the descriptors
			TD3D12ResourceDescriptor               mDepthStencilDescriptor{};

			E_RESOURCE_LAYOUT                      mCurrLayout;

			TPtr<CD3D12Buffer>                     mpReadbackBuffer = nullptr;
	};


	/*!
		\brief A factory function for creation objects of CD3D12VertexDeclaration's type

		\return A pointer to CD3D12VertexDeclaration's implementation
	*/

	IVertexDeclaration* CreateD3D12VertexDeclaration(E_RESULT_CODE& result);


	class CD3D12BasePipeline : public virtual IPipeline
	{
		public:
			virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext);

			//virtual VkPipelineLayout GetPipelineLayout() const;
			virtual const TD3D12PipelineLayoutInfo& GetLayoutInfo() const;

			virtual ComPtr<ID3D12PipelineState> GetNativePSO() const;
			virtual ID3D12RootSignature* GetRootSignature() const;
			virtual U32 GetHash() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12BasePipeline)
		protected:
			CD3D12GraphicsObjectManager* mpD3D12GraphicsObjectManagerImpl = nullptr;
			CD3D12GraphicsContext*       mpD3D12GraphicsContext = nullptr;

			ComPtr<ID3D12PipelineState>  mpPipelineStateObject = nullptr;
			ComPtr<ID3D12RootSignature>  mpCachedRootSignature = nullptr;

			TD3D12PipelineLayoutInfo     mLayoutInfo{};
			U32                          mConfigHash = 0;
	};

	TDE2_API IGraphicsPipeline* CreateD3D12GraphicsPipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& pipelineConfig, E_RESULT_CODE& result);
	TDE2_API IComputePipeline* CreateD3D12ComputePipeline(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId, E_RESULT_CODE& result);


	/*!
		\brief CD3D12GraphicsPipeline's definition
	*/

	class CD3D12GraphicsPipeline : public CBaseGraphicsPipeline, public CD3D12BasePipeline
	{
		public:
			TDE2_API friend IGraphicsPipeline* CreateD3D12GraphicsPipeline(IGraphicsContext*, IResourceManager*, const TGraphicsPipelineConfigDesc&, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const TGraphicsPipelineConfigDesc& pipelineConfig) override;
			E_RESULT_CODE Bind() override;

			ComPtr<ID3D12PipelineState> GetPipelineForRenderPass(const TRenderPassInfo& renderPassInfo);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12GraphicsPipeline)
	};


	/*!
		\brief CD3D12ComputePipeline's definition
	*/

	class CD3D12ComputePipeline : public CBaseComputePipeline, public CD3D12BasePipeline
	{
		public:
			TDE2_API friend IComputePipeline* CreateD3D12ComputePipeline(IGraphicsContext*, IResourceManager*, const std::string&, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, const std::string& shaderId) override;
			E_RESULT_CODE Bind() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12ComputePipeline)
		private:
	};
}

#endif