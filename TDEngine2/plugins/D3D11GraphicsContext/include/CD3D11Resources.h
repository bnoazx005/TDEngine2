/*!
	\file CD3D11Resources.h
	\date 22.09.2024
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IBuffer.h>
#include <graphics/CBaseShader.h>
#include <graphics/CBaseShaderCompiler.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/CVertexDeclaration.h>
#include <core/CBaseObject.h>


#if defined (TDE2_USE_WINPLATFORM)

#include <d3d11.h>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	/*!
		class CD3D11Buffer

		\brief The class is an implementation of a common D3D11 buffer, which 
		can be used in different 
	*/

	class CD3D11Buffer : public CBaseObject, public IBuffer
	{
		public:
			friend IBuffer* CreateD3D11Buffer(IGraphicsContext*, const TInitBufferParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params) override;

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

			/*!
				\brief The method returns a pointer to ID3D11DeviceContext implementation

				\return The method returns a pointer to ID3D11DeviceContext implementation
			*/

			ID3D11DeviceContext* GetDeviceContext() const;

			const TInitBufferParams& GetParams() const override;

			ID3D11Buffer* GetD3D11Buffer();

			ID3D11ShaderResourceView* GetShaderView();
			ID3D11UnorderedAccessView* GetWriteableShaderView();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11Buffer)
			
			E_RESULT_CODE _onInitInternal(const TInitBufferParams& params);
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			ID3D11DeviceContext*     mp3dDeviceContext;
			ID3D11Device*            mp3dDevice;

			ID3D11Buffer*            mpBufferInstance;
			
			USIZE                    mBufferSize;
			USIZE                    mElementStrideSize;

			E_BUFFER_USAGE_TYPE      mBufferUsageType;

			E_BUFFER_TYPE            mBufferType;

			U32                      mAccessFlags;

			D3D11_MAPPED_SUBRESOURCE mMappedBufferData;
			void*                    mpLockDataPtr;

			TInitBufferParams        mInitParams;

			ID3D11ShaderResourceView*  mpShaderView = nullptr;
			ID3D11UnorderedAccessView* mpWritableShaderView = nullptr;
	};


	/*!
		\brief A factory function for creation objects of CD3D11GraphicsContext's type

		\return A pointer to CD3D11GraphicsContext's implementation
	*/

	IBuffer* CreateD3D11Buffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of CD3D11Shader's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11Shader's implementation
	*/

	IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CD3D11Shader

		\brief The class is a common implementation for all platforms
	*/

	class CD3D11Shader : public CBaseShader
	{
		public:
			friend IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CD3D11Shader)

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

			/*!
				\brief The method returns a bytecode of a vertex shader, or nullptr if a shader hasn't compiled yet

				\return The method returns a bytecode of a vertex shader, or nullptr if a shader hasn't compiled yet
			*/

			const std::vector<U8>& GetVertexShaderBytecode() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11Shader)

			E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);
		protected:
			ID3D11DeviceContext* mp3dDeviceContext;

			ID3D11VertexShader* mpVertexShader;

			ID3D11PixelShader* mpPixelShader;

			ID3D11GeometryShader* mpGeometryShader;

			ID3D11ComputeShader* mpComputeShader;

			std::vector<U8>       mVertexShaderBytecode;
	};


	/*!
		\brief A factory function for creation objects of CD3D11ShaderFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderFactory's implementation
	*/

	IResourceFactory* CreateD3D11ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
	

	/*!
		struct TD3D11ShaderCompilerOutput

		\brief The structure contains shader compiler's output data for D3D11 GAPI
	*/

	typedef struct TD3D11ShaderCompilerOutput : public TShaderCompilerOutput
	{
		virtual ~TD3D11ShaderCompilerOutput() = default;
	} TD3D11ShaderCompilerOutput, * TD3D11ShaderCompilerOutputPtr;


	/*!
		\brief A factory function for creation objects of CD3D11ShaderCompiler's type

		\param[in, out] pFileSystem A pointer to IFileSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderCompiler's implementation
	*/

	IShaderCompiler* CreateD3D11ShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result);


	enum E_D3D11_TEXTURE_ACCESS_TYPE : U32
	{
		DTAT_CPU_READ = 0x1,
		DTAT_CPU_WRITE = 0x2,
		DTAT_GPU_READ = 0x4,
		DTAT_GPU_WRITE = 0x8,
	};


	ITextureImpl* CreateD3D11TextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class CD3D11TextureImpl
	*/

	class CD3D11TextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend ITextureImpl* CreateD3D11TextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;

			ID3D11Resource* GetTextureResource();
			ID3D11ShaderResourceView* GetShaderResourceView();

			ID3D11RenderTargetView* GetRenderTargetView();
			ID3D11DepthStencilView* GetDepthStencilView();
			ID3D11UnorderedAccessView* GetUnorderedAccessView();

			std::vector<U8> ReadBytes(U32 index) override;

			const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11TextureImpl)

			E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			TInitTextureImplParams     mInitParams;

			ID3D11Device* mp3dDevice = nullptr;
			ID3D11DeviceContext* mp3dDeviceContext = nullptr;

			ID3D11Resource* mpTextureResource = nullptr;

			ID3D11ShaderResourceView* mpShaderTextureView = nullptr;

			// optional views
			ID3D11UnorderedAccessView* mpUavTextureView = nullptr;
			ID3D11RenderTargetView* mpRenderTargetView = nullptr;
			ID3D11DepthStencilView* mpDepthStencilView = nullptr;
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(CD3D11TextureImpl)

	/*!
		\brief A factory function for creation objects of CD3D11VertexDeclaration's type

		\return A pointer to CD3D11VertexDeclaration's implementation
	*/

	IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result);


	/*!
		class CD3D11VertexDeclaration

		\brief The class implements a vertex declaration for D3D11
	*/

	class CD3D11VertexDeclaration : public CVertexDeclaration
	{
		public:
			friend IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method creates a new input layout object  based on a given shader's description and returns it

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in] pShader A pointer to IShader implementation

				\return The method creates a new input layout object  based on a given shader's description and returns it
			*/

			TResult<ID3D11InputLayout*> GetInputLayoutByShader(IGraphicsContext* pGraphicsContext, const IShader* pShader);

			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffersArray An array of  IVertexBuffer implementations

				\param[in, out] pShader A pointer to IShader implementation
			*/

			void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11VertexDeclaration)

				E_RESULT_CODE _onFreeInternal() override;
		protected:
			ID3D11InputLayout* mpInputLayout;
	};
}

#endif