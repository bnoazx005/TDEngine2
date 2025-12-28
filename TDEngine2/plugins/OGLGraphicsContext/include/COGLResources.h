/*!
	\file COGLResources.h
	\date 23.09.2024
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/IBuffer.h>
#include <graphics/CBaseShader.h>
#include <graphics/CBaseShaderCompiler.h>
#include <graphics/CBaseTexture2D.h>
#include <graphics/CVertexDeclaration.h>
#include <core/CBaseObject.h>
#include <GL/glew.h>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IGraphicsPipeline;


	/*!
		class COGLBuffer

		\brief The class is an implementation of a common OGL buffer, which
		can be used in different
	*/

	class COGLBuffer : public CBaseObject, public IBuffer
	{			
		public:
			friend IBuffer* CreateOGLBuffer(IGraphicsContext*, const TInitBufferParams&, E_RESULT_CODE&);
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

			const TInitBufferParams& GetParams() const override;

			E_RESOURCE_LAYOUT GetLayout() const override;

			GLuint GetOGLHandle();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLBuffer)

			E_RESULT_CODE _onFreeInternal() override;
		protected:
			GLuint                   mBufferHandler = 0;

			USIZE                    mBufferSize = 0;

			E_BUFFER_USAGE_TYPE      mBufferUsageType = E_BUFFER_USAGE_TYPE::DEFAULT;

			E_BUFFER_TYPE            mBufferType = E_BUFFER_TYPE::GENERIC;

			void*                    mpMappedBufferData = nullptr;

			TInitBufferParams        mInitParams{};
			TBufferHandleId          mHandle;

#if TDE2_DEBUG_MODE
			U8                       mLockChecker = 0;
#endif
	};


	/*!
		\brief A factory function for creation objects of COGLGraphicsContext's type

		\return A pointer to COGLGraphicsContext's implementation
	*/

	IBuffer* CreateOGLBuffer(IGraphicsContext* pGraphicsContext, const TInitBufferParams& params, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of COGLShaderImpl's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLShaderImpl's implementation
	*/

	IShaderImpl* CreateOGLShaderImpl(IGraphicsContext* pGraphicsContext, const std::string& shaderId, E_RESULT_CODE& result);


	/*!
		class COGLShaderImpl

		\brief The class implements shader object's functionality for GL GAPI
	*/

	class COGLShaderImpl : public CBaseShaderImpl
	{
		public:
			friend IShaderImpl* CreateOGLShaderImpl(IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		protected:
			typedef std::unordered_map<U32, U32> TUniformBuffersMap;
		public:
			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			void Unbind() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLShaderImpl)

			E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);

			void _bindUniformBuffer(U32 slot, TBufferHandleId uniformsBufferHandle) override;

			E_RESULT_CODE _createTexturesHashTable(const TShaderCompilerOutput* pCompilerData) override;

			E_RESULT_CODE _onFreeInternal() override;
		protected:
			GLuint             mShaderHandler = 0;

			TUniformBuffersMap mUniformBuffersMap{}; ///< \note the hash table stores information about buffers indices that were assigned to them by GLSL compiler
	};


	/*!
		struct TOGLShaderCompilerOutput

		\brief The structure contains shader compiler's output data for OGL 3.x GAPI
	*/

	typedef struct TOGLShaderCompilerOutput : public TShaderCompilerOutput
	{
		virtual ~TOGLShaderCompilerOutput() = default;

		GLuint mVertexShaderHandler;

		GLuint mFragmentShaderHandler;

		GLuint mGeometryShaderHandler;

		GLuint mComputeShaderHandler;
	} TOGLShaderCompilerOutput, * TOGLShaderCompilerOutputPtr;


	/*!
		\brief A factory function for creation objects of COGLShaderCompiler's type

		\param[in, out] pFileSystem A pointer to IFileSystem implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLShaderCompiler's implementation
	*/

	IShaderCompiler* CreateOGLShaderCompiler(IFileSystem* pFileSystem, E_RESULT_CODE& result);


	ITextureImpl* CreateOGLTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class COGLTextureImpl
	*/

	class COGLTextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend ITextureImpl* CreateOGLTextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;
			E_RESULT_CODE Transition(E_RESOURCE_LAYOUT newLayout) override;

			E_RESULT_CODE SetHandle(TTextureHandleId handle, const CPassKey<CBaseGraphicsObjectManager>& passkey) override;

			GLuint GetTextureHandle();

			Vector<U8> ReadBytes(U32 index) override;

			E_RESOURCE_LAYOUT GetLayout() const override;

			const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLTextureImpl)

				E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		protected:
			TInitTextureImplParams     mInitParams;
			TTextureHandleId           mHandle;
			GLuint                     mTextureHandle = 0;
	};


	/*!
		\brief A factory function for creation objects of COGLVertexDeclaration's type

		\return A pointer to COGLVertexDeclaration's implementation
	*/

	IVertexDeclaration* CreateOGLVertexDeclaration(E_RESULT_CODE& result);


	/*!
		class COGLVertexDeclaration

		\brief The class implements a vertex declaration for OGL
	*/

	class COGLVertexDeclaration : public CVertexDeclaration
	{
		public:
			friend IVertexDeclaration* CreateOGLVertexDeclaration(E_RESULT_CODE& result);
		public:
			typedef struct TVAORegistryNode
			{
				GLuint                                                     mVAOHandle;

				std::unordered_map<U32, std::unique_ptr<TVAORegistryNode>> mChildren;
			} TVAORegistryNode, * TVAORegistryNodePtr;
		public:
			/*!
				\brief The method creates a VAO for specified vertex declaration and binds it
				to a given vertex buffer

				\param[in] pVertexBuffersArray An array of  IVertexBuffer implementations

				\return An object that contains either a handler of created VAO or an error code
			*/

			TResult<GLuint> GetVertexArrayObject(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray);

			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffersArray An array of  IVertexBuffer implementations

				\param[in, out] pShader A pointer to IShader implementation
			*/

			void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShaderImpl* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLVertexDeclaration)

			E_RESULT_CODE _onFreeInternal() override;

			TVAORegistryNode* _insertNewNode(TVAORegistryNode* pCurrNode, U32 handle);
		protected:
			TVAORegistryNode mRootNode;
	};


	TDE2_API IGraphicsPipeline* CreateGLGraphicsPipeline(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& pipelineConfig, E_RESULT_CODE& result);
}