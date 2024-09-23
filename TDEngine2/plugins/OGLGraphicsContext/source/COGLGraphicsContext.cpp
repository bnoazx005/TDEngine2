#include "../include/COGLGraphicsContext.h"
#include "../include/IOGLContextFactory.h"
#include "../include/COGLGraphicsObjectManager.h"
#include "../include/COGLMappings.h"
#include "../include/COGLRenderTarget.h"
#include "../include/COGLDepthBufferTarget.h"
#include "../include/COGLResources.h"
#include "../include/COGLUtils.h"
#include <utils/CFileLogger.h>
#include <core/IEventManager.h>
#include <core/IWindowSystem.h>
#include <string>
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"


#if defined (TDE2_USE_WINPLATFORM)
	#include <GL/glew.h>
	#pragma comment(lib, "glew32.lib")
#elif defined (TDE2_USE_UNIXPLATFORM)
	#include <GL/glxew.h>
#else
#endif


namespace TDEngine2
{
	/*!
		class COGLGraphicsContext

		\brief The class is an implementation of IGraphicsContext interface that
		wraps up low-level calls of OpenGL library

		\todo Not the best idea to pass GL context's factory function into constructor was used, should resolve this injection
		some another way
	*/

	class COGLGraphicsContext : public IGraphicsContext, public IEventHandler, public CBaseObject
	{
		public:
			friend IGraphicsContext* CreateOGLGraphicsContext(TPtr<IWindowSystem>, TCreateGLContextFactoryCallback, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(COGLGraphicsContext)

				/*!
					\brief The method initializes an initial state of the object

					\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

					\return RC_OK if everything went ok, or some other code, which describes an error
				*/

				E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) override;

			/*!
				\brief The method clears up back buffer with specified color

				\param[in] color The new color of a back buffer
			*/

			void ClearBackBuffer(const TColor32F& color) override;

			void BeginFrame() override;

			/*!
				\brief The method swaps a current buffer with a back one
			*/

			void Present() override;

			/*!
				\brief The method clears up render target with specified color. This method has high overhead
				for OGL implementation, try to use another overloading

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
				\param[in] color The new color of a render target
			*/

			void ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color) override;

			/*!
				\brief The method clears up render target with specified color

				\param[in] slot A slot into which the render target that should be cleared up is bound
				\param[in] color The new color of a render target
			*/

			void ClearRenderTarget(U8 slot, const TColor32F& color) override;

			/*!
				\brief The method clears up given depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
				\param[in] stencilValue The stencil buffer will be cleared with this value
			*/

			void ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue) override;

			/*!
				\brief The method clears up depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
			*/

			void ClearDepthBuffer(F32 value) override;

			/*!
				\brief The method clears up stencil buffer with specified values

				\param[in] value The stencil buffer will be cleared with this value
			*/

			void ClearStencilBuffer(U8 value) override;

			/*!
				\brief The method sets up a viewport's parameters

				\param[in] x x position of left hand side of a viewport
				\param[in] y y position of left bottom corner of a viewport
				\param[in] width width of a viewport
				\param[in] height height of a viewport
				\param[in] minDepth minimum depth of a viewport
				\param[in] maxDepth maximum depth of a viewport
			*/

			void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth) override;

			/*!
				\brief The method specifies rectangle for scissor testing, all the geometry outside of which
				is rejected

				\param[in] scissorRect A rectangle for scissor test
			*/

			void SetScissorRect(const TRectU32& scissorRect) override;

			/*!
				\brief The method computes a perspective projection matrix specific for a graphics context

				\param[in] fov A field of view

				\param[in] aspect An aspect ratio of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\return The method computes a perspective projection matrix specific for a graphics context
			*/

			TMatrix4 CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf) override;

			/*!
				\brief The method computes an orthographic projection matrix specific for a graphics context

				\param[in] left X coordinate of a left corner of a screen

				\param[in] top Y coordinate of a left top corner of a screen

				\param[in] right X coordinate of a right corner of a screen

				\param[in] bottom Y coordinate of a right bottom corner of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\param[in] isDepthless The flag determines whether the output matrix will process z axis or just to project onto some constant, which
				equals to average position between zn and zf

				\return The method computes a orthographic projection matrix specific for a graphics context
			*/

			TMatrix4 CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless = false) override;

			E_RESULT_CODE SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize) override;
			E_RESULT_CODE SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset) override;
			E_RESULT_CODE SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle) override;
			E_RESULT_CODE SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled = false) override;

			E_RESULT_CODE SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled = false) override;
			E_RESULT_CODE SetSampler(U32 slot, TTextureSamplerId samplerHandle) override;

			E_RESULT_CODE UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			E_RESULT_CODE UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			E_RESULT_CODE UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			E_RESULT_CODE UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;

			// Blit one texture to another
			E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle) override;
			// Upload data into texture
			E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle) override;
			// Readback data from texture into a buffer
			E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle) override;
			// Copy buffer to buffer
			E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle) override;

			/*!
				\brief The method copies counter of sourceHandle buffer into destHandle's one

				\param[in] sourceHandle A handle to structured appendable buffer resource that should be created with unordered access write flag
				\param[in] destHandle A handle to any buffer resource
				\param[in] offset An offset within dest buffer
			*/

			E_RESULT_CODE CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset) override;

			/*!
				\brief The method generates all mip levels for the specified texture
			*/

			E_RESULT_CODE GenerateMipMaps(TTextureHandleId textureHandle) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] numOfVertices A total number of vertices should be drawn
			*/

			void Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data using
				index buffer to group primitives

				\param[in] topology A primitive topology's type

				\param[in] indexFormatType A format of a single index

				\param[in] baseVertex A value that will be added to each index

				\param[in] startIndex A first index that will be read by GPU

				\param[in] numOfIndices A total number of indices
			*/

			void DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data that uses instancing

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] verticesPerInstance A number of vertices per instance

				\param[in] startInstance An offset value which is added to every index before read the data

				\param[in] numOfInstances A total number of instances should be drawn
			*/

			void DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data that uses instancing

				\param[in] topology A primitive topology's type

				\param[in] indexFormatType A format of a single index

				\param[in] baseVertex A value that will be added to each index

				\param[in] startIndex A first index that will be read by GPU

				\param[in] startInstance An offset value which is added to every index before read the data

				\param[in] indicesPerInstance A number of indices per instance

				\param[in] numOfInstances A total number of instances should be drawn
			*/

			void DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
				U32 startInstance, U32 indicesPerInstance, U32 numOfInstances) override;

			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			void DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] indexFormatType A format of a single index
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			void DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] groupsCountX A number of groups over X axis
				\param[in] groupsCountY A number of groups over Y axis
				\param[in] groupsCountZ A number of groups over Z axis
			*/

			void DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ) override;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			void DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method binds a given blend state to rendering pipeline

				\param[in] blendStateId An identifier of a blend state
			*/

			void BindBlendState(TBlendStateId blendStateId) override;

			/*!
				\brief The method binds a given depth-stencil state to rendering pipeline

				\param[in] depthStencilStateId An identifier of a depth-stencil state
				\param[in] stencilRef A reference value to perform against when doing a depth-stencil test
			*/

			void BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef = 0x0) override;

			/*!
				\brief The method binds a given rasteriazer state to rendering pipeline

				\param[in] rasterizerStateId An identifier of a rasterizer state
			*/

			void BindRasterizerState(TRasterizerStateId rasterizerStateId) override;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
			*/

			void BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget) override;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in] targetHandle Handle to texture object that's created as a render target
			*/

			void BindRenderTarget(U8 slot, TTextureHandleId targetHandle) override;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in, out] pDepthBufferTarget A pointer to IDepthBufferTarget implementation
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			void BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite = false) override;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in] targetHandle Handle to texture object that's created as a depth buffer
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			void BindDepthBufferTarget(TTextureHandleId targetHandle, bool disableRTWrite = false) override;

			/*!
				\brief The method disables or enables a depth buffer usage

				\param[in] value If true the depth buffer will be used, false turns off it
			*/

			void SetDepthBufferEnabled(bool value) override;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			const TGraphicsCtxInternalData& GetInternalData() const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns a pointer to IGraphicsObjectManager implementation

				\return The method returns a pointer to IGraphicsObjectManager implementation
			*/

			IGraphicsObjectManager* GetGraphicsObjectManager() const override;

			/*!
				\brief The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case

				\return The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case
			*/

			F32 GetPositiveZAxisDirection() const override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE OnEvent(const TBaseEvent* pEvent);

	#if TDE2_DEBUG_MODE
			void BeginSectionMarker(const std::string& id) override;
			void EndSectionMarker() override;
	#endif
			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TEventListenerId GetListenerId() const;

			/*!
				\brief The method returns an information about currently using video adapter

				\return The method returns an information about currently using video adapter
			*/

			TVideoAdapterInfo GetInfo() const override;

			/*
				\bief The method returns information about GAPI environment

				\return The method returns data that describes all GAPI related information
			*/

			const TGraphicsContextInfo& GetContextInfo() const override;

			/*!
				\return The method returns a pointer to IWindowSystem
			*/

			TPtr<IWindowSystem> GetWindowSystem() const override;

			std::vector<U8> GetBackBufferData() const override;
		protected:
			COGLGraphicsContext(TCreateGLContextFactoryCallback glContextFactoryCallback);
			COGLGraphicsContext(const COGLGraphicsContext& graphicsCtx) = delete;
			virtual COGLGraphicsContext& operator= (COGLGraphicsContext& graphicsCtx) = delete;
			virtual ~COGLGraphicsContext();

			E_RESULT_CODE _initFBO(const IWindowSystem* pWindowSystem);

			E_RESULT_CODE _onFreeInternal() override;
		protected:
			constexpr static U8             mMaxNumOfRenderTargets = 8;

			TGraphicsCtxInternalData        mInternalDataObject;
			IOGLContextFactory* mpGLContextFactory;
			TCreateGLContextFactoryCallback mGLContextFactoryCallback;
			TWindowSystemInternalData       mWindowInternalData;

			TPtr<IGraphicsObjectManager>    mpGraphicsObjectManager = nullptr;
			COGLGraphicsObjectManager* mpGraphicsObjectManagerImpl = nullptr;

			TPtr<IWindowSystem>             mpWindowSystem;
			TPtr<IEventManager>             mpEventManager;

			GLuint                          mMainFBOHandler; /// \todo Replace it with FBOManager later
			GLuint                          mMainDepthStencilRenderbuffer;
			GLuint                          mCurrDepthBufferHandle;

			GLuint                          mRenderTargets[mMaxNumOfRenderTargets];
	};


	static void TDE2_APIENTRY DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar* msg, const void* data)
	{
		const char* sourceTypeStr = "Unknown";

		switch (source) 
		{
			case GL_DEBUG_SOURCE_API:
				sourceTypeStr = "API";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				sourceTypeStr = "Window System";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				sourceTypeStr = "Shader Compiler";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				sourceTypeStr = "Third Party";
				break;
			case GL_DEBUG_SOURCE_APPLICATION:
				sourceTypeStr = "App";
				break;
		}

		const char* typeStr = "Unknown";

		if (GL_DEBUG_TYPE_OTHER == type)
		{
			return;
		}

		switch (type) 
		{
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				typeStr = "Deprecated Behaviour";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				typeStr = "Undefined Behaviour";
				break;
			case GL_DEBUG_TYPE_PORTABILITY:
				typeStr = "Portability";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				typeStr = "Performance";
				break;
		}

		const char* severityStr = "Unknown";

		if (GL_DEBUG_SEVERITY_NOTIFICATION == severity)
		{
			return;
		}

		switch (severity) 
		{
			case GL_DEBUG_SEVERITY_HIGH:
				severityStr = "High";
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				severityStr = "Medium";
				break;
			case GL_DEBUG_SEVERITY_LOW:
				severityStr = "Low";
				break;
		}

		if (GL_DEBUG_TYPE_ERROR == type)
		{
			LOG_ERROR(Wrench::StringUtils::Format("[COGLGraphicsContext] {0}: {1} severity, raised from {2} : {3}\n",
				id, severityStr, sourceTypeStr, msg));
			return;
		}

		LOG_MESSAGE(Wrench::StringUtils::Format("[COGLGraphicsContext] {0}: {1} of {2} severity, raised from {3} : {4}", 
			id, typeStr, severityStr, sourceTypeStr, msg));
	}


	COGLGraphicsContext::COGLGraphicsContext(TCreateGLContextFactoryCallback glContextFactoryCallback):
		CBaseObject(), mGLContextFactoryCallback(glContextFactoryCallback)
	{
	}

	COGLGraphicsContext::~COGLGraphicsContext()
	{
	}

	E_RESULT_CODE COGLGraphicsContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpWindowSystem = pWindowSystem;

		if (!mGLContextFactoryCallback)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		/// creating GL context
		mpGLContextFactory = mGLContextFactoryCallback(pWindowSystem.Get(), result);

		///< Should be called after initialization of GL context factory, because the latter
		/// can initialize a window object (i.e this feature is used on UNIX platform)
		mWindowInternalData = pWindowSystem->GetInternalData();

		if (result != RC_OK)
		{
			return result;
		}

		result = mpGLContextFactory->SetContext();

		if (result != RC_OK)
		{
			return result;
		}

		/// initialization of GLEW
		glewExperimental = true;

		GLenum err = glewInit();
		
		if (err != GLEW_OK)
		{
			return RC_FAIL;
		}

#if TDE2_DEBUG_MODE
		GL_SAFE_CALL(glEnable(GL_DEBUG_OUTPUT));
		GL_SAFE_CALL(glDebugMessageCallback(DebugMessageCallback, nullptr));
#endif

		mpGraphicsObjectManager = TPtr<IGraphicsObjectManager>(CreateOGLGraphicsObjectManager(this, result));
		mpGraphicsObjectManagerImpl = dynamic_cast<COGLGraphicsObjectManager*>(mpGraphicsObjectManager.Get());

		if (result != RC_OK)
		{
			return result;
		}

		U32 creationFlags = pWindowSystem->GetFlags();

		/// Enable a depth buffer if it's needed
		if (creationFlags & P_ZBUFFER_ENABLED)
		{
			GL_SAFE_CALL(glEnable(GL_DEPTH_TEST));
			GL_SAFE_CALL(glDepthFunc(GL_LESS));
		}

		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);

		if ((result = _initFBO(pWindowSystem.Get())) != RC_OK)
		{
			return result;
		}

		SetViewport(0, 0, static_cast<F32>(pWindowSystem->GetWidth()), static_cast<F32>(pWindowSystem->GetHeight()), 0.0f, 1.0f);

		mIsInitialized = true;
		
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::_onFreeInternal()
	{
		GL_SAFE_CALL(glDeleteRenderbuffers(1, &mMainDepthStencilRenderbuffer));
		GL_SAFE_CALL(glDeleteFramebuffers(1, &mMainFBOHandler));

		mpGraphicsObjectManager = nullptr;

		E_RESULT_CODE result = RC_OK;
		result = result | mpGLContextFactory->Free();

		return result;
	}
	
	void COGLGraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		GL_SAFE_VOID_CALL(glClearColor(color.r, color.g, color.b, color.a));
		GL_SAFE_VOID_CALL(glClear(GL_COLOR_BUFFER_BIT));
	}

	void COGLGraphicsContext::ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color)
	{
		if (!pRenderTarget)
		{
			LOG_WARNING("[COGLGraphicsContext] Try to clear the invalid render target");
			return;
		}

		U8 renderTargetBoundSlot = (std::numeric_limits<U8>::max)();

		COGLRenderTarget* pOGLRenderTarget = dynamic_cast<COGLRenderTarget*>(pRenderTarget);

		for (U8 currSlotIndex = 0; currSlotIndex < mMaxNumOfRenderTargets; ++currSlotIndex)
		{
			if (mRenderTargets[currSlotIndex] == pOGLRenderTarget->GetInternalHandler())
			{
				renderTargetBoundSlot = currSlotIndex;
				break;
			}
		}

		// \note The render target isn't bound to the main FBO
		if (renderTargetBoundSlot >= mMaxNumOfRenderTargets)
		{
			return;
		}

		ClearRenderTarget(renderTargetBoundSlot, color);
	}
	
	void COGLGraphicsContext::ClearRenderTarget(U8 slot, const TColor32F& color)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			return;
		}

		if (!mRenderTargets[slot])
		{
			LOG_WARNING(Wrench::StringUtils::Format("[COGLGraphicsContext] Try to clear the render target in slot {0}, but it's empty", slot));
			return;
		}

		const F32 clearColorArray[4]{ color.r, color.g, color.b, color.a };
		GL_SAFE_VOID_CALL(glClearBufferfv(GL_COLOR, 0, clearColorArray));
	}

	void COGLGraphicsContext::ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue)
	{
		TDE2_UNIMPLEMENTED();
	}

	void COGLGraphicsContext::ClearDepthBuffer(F32 value)
	{
		GL_SAFE_VOID_CALL(glClearDepthf(value));
		GL_SAFE_VOID_CALL(glClear(GL_DEPTH_BUFFER_BIT));
	}
	
	void COGLGraphicsContext::ClearStencilBuffer(U8 value)
	{
		GL_SAFE_VOID_CALL(glClearStencil(value));
		GL_SAFE_VOID_CALL(glClear(GL_STENCIL_BUFFER_BIT));
	}

	void COGLGraphicsContext::BeginFrame()
	{
	}

	void COGLGraphicsContext::Present()
	{
#if defined (TDE2_USE_WINPLATFORM)
		SwapBuffers(mWindowInternalData.mDeviceContextHandler);
#elif defined (TDE2_USE_UNIXPLATFORM)
		glXSwapBuffers(mWindowInternalData.mpDisplayHandler, mWindowInternalData.mWindowHandler);
#else
#endif
	}

	void COGLGraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		GL_SAFE_VOID_CALL(glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
	}
	
	void COGLGraphicsContext::SetScissorRect(const TRectU32& scissorRect)
	{
		auto&& clientRect = mpWindowSystem->GetClientRect();

		/// convert from D3D like viewport to GL (Y axis is inverted)
		GL_SAFE_VOID_CALL(
			glScissor(static_cast<GLint>(scissorRect.x),
				static_cast<GLint>(clientRect.height - (scissorRect.y + scissorRect.height)),
				static_cast<GLsizei>(scissorRect.width),
				static_cast<GLsizei>(scissorRect.height)));
	}

	TMatrix4 COGLGraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, -1.0f, 1.0f, -1.0f);
	}

	TMatrix4 COGLGraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, -1.0f, 1.0f, -1.0f, isDepthless);
	}

	E_RESULT_CODE COGLGraphicsContext::SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize)
	{
		auto pBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(vertexBufferHandle);
		if (!pBuffer)
		{
			GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
			return RC_FAIL;
		}

		GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, pBuffer->GetOGLHandle()));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset)
	{
		if (TBufferHandleId::Invalid == indexBufferHandle)
		{
			GL_SAFE_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
			return RC_OK;
		}

		auto pBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(indexBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pBuffer->GetOGLHandle()));
		
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle)
	{
		if (TBufferHandleId::Invalid == constantsBufferHandle)
		{
			GL_SAFE_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, slot, 0));
			return RC_OK;
		}

		auto pBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(constantsBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, slot, pBuffer->GetOGLHandle()));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled)
	{
		if (TBufferHandleId::Invalid == bufferHandle)
		{
			GL_SAFE_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, 0));
			return RC_OK;
		}

		auto pBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(bufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, pBuffer->GetOGLHandle()));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled)
	{
		if (TTextureHandleId::Invalid == textureHandle)
		{
			return RC_OK;
		}

		auto pTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		if (pTexture->GetParams().mIsWriteable)
		{
			if (isWriteEnabled)
			{
				GL_SAFE_VOID_CALL(glBindImageTexture(slot, pTexture->GetTextureHandle(), 0, GL_FALSE, 0, GL_WRITE_ONLY, 
					COGLMappings::GetWritableTextureFormat(pTexture->GetParams().mFormat)));

				return RC_OK;
			}
		}

		GL_SAFE_VOID_CALL(glActiveTexture(GL_TEXTURE0 + slot));
		GL_SAFE_VOID_CALL(glBindTexture(COGLMappings::GetTextureType(pTexture->GetParams().mType), pTexture->GetTextureHandle()));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::SetSampler(U32 slot, TTextureSamplerId samplerHandle)
	{
		if (samplerHandle == TTextureSamplerId::Invalid)
		{
			GL_SAFE_CALL(glBindSampler(slot, 0));
			return RC_OK;
		}

		GLuint internalSamplerId = mpGraphicsObjectManagerImpl->GetTextureSampler(samplerHandle).Get();
		GL_SAFE_CALL(glBindSampler(slot, internalSamplerId));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		const E_FORMAT_TYPE textureFormat = pTexture->GetParams().mFormat;

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, pTexture->GetTextureHandle()));

		GL_SAFE_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		GL_SAFE_CALL(glTexSubImage2D(GL_TEXTURE_2D, mipLevel, regionRect.x, regionRect.y, regionRect.width, regionRect.height, 
			COGLMappings::GetPixelDataFormat(textureFormat), COGLMappings::GetBaseTypeOfFormat(textureFormat), pData));

		if (pTexture->GetParams().mNumOfMipLevels > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D, 0));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D_ARRAY, pTexture->GetTextureHandle()));
		GL_SAFE_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		GL_SAFE_CALL(glTexSubImage2D(GL_TEXTURE_2D_ARRAY, index, regionRect.x, regionRect.y, regionRect.width, regionRect.height,
			COGLMappings::GetPixelDataFormat(pTexture->GetParams().mFormat), GL_UNSIGNED_BYTE, pData));

		if (pTexture->GetParams().mNumOfMipLevels > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_2D_ARRAY));
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, pTexture->GetTextureHandle()));
		GL_SAFE_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		auto&& params = pTexture->GetParams();

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		GL_SAFE_CALL(glTexSubImage2D(COGLMappings::GetCubemapFace(face), 0, regionRect.x, regionRect.y, regionRect.width, regionRect.height,
			COGLMappings::GetPixelDataFormat(params.mFormat), GL_UNSIGNED_BYTE, pData));

		if (params.mNumOfMipLevels > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_3D, pTexture->GetTextureHandle()));

		GL_SAFE_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		/// GL_UNSIGNED_BYTE is used explicitly, because of stb_image stores data as unsigned char array
		GL_SAFE_CALL(glTexSubImage3D(GL_TEXTURE_3D, 0, 
			regionRect.x, regionRect.y, static_cast<GLint>(depthFrom),
			regionRect.width, regionRect.height, static_cast<GLint>(depthTo - depthFrom),
			COGLMappings::GetPixelDataFormat(pTexture->GetParams().mFormat), GL_UNSIGNED_BYTE, pData));

		if (pTexture->GetParams().mNumOfMipLevels > 1)
		{
			GL_SAFE_CALL(glGenerateMipmap(GL_TEXTURE_3D));
		}

		GL_SAFE_CALL(glBindTexture(GL_TEXTURE_3D, 0));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle)
	{
		auto pSourceTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(sourceHandle);
		auto pDestTexture   = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(destHandle);

		if (!pSourceTexture || !pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		GLuint sourceTextureHandle = pSourceTexture->GetTextureHandle();
		GLuint destTextureHandle   = pDestTexture->GetTextureHandle();

		const TInitTextureImplParams& sourceTextureParams = pSourceTexture->GetParams();

		if (GLEW_ARB_copy_image)
		{
			GL_SAFE_CALL(glCopyImageSubData(
				sourceTextureHandle, GL_TEXTURE_2D, 0, 0, 0, 0,
				destTextureHandle, GL_TEXTURE_2D, 0, 0, 0, 0, 
				static_cast<GLsizei>(sourceTextureParams.mWidth), static_cast<GLsizei>(sourceTextureParams.mHeight), static_cast<GLsizei>(sourceTextureParams.mDepth)));

			return RC_OK;
		}
		
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle)
	{
		auto pSourceBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(sourceHandle);
		auto pDestBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(destHandle);

		if (!pSourceBuffer || !pDestBuffer)
		{
			return RC_FAIL;
		}

		GL_SAFE_CALL(glBindBuffer(GL_COPY_READ_BUFFER, pSourceBuffer->GetOGLHandle()));
		GL_SAFE_CALL(glBindBuffer(GL_COPY_WRITE_BUFFER, pDestBuffer->GetOGLHandle()));
		GL_SAFE_CALL(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, std::min(pSourceBuffer->GetSize(), pDestBuffer->GetSize())));

		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE COGLGraphicsContext::GenerateMipMaps(TTextureHandleId textureHandle)
	{
		if (TTextureHandleId::Invalid == textureHandle)
		{
			return RC_FAIL;
		}

		auto&& pTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		const GLenum textureType = COGLMappings::GetTextureType(pTexture->GetParams().mType);

		GL_SAFE_CALL(glActiveTexture(GL_TEXTURE0));
		GL_SAFE_CALL(glBindTexture(textureType, pTexture->GetTextureHandle()));

		GL_SAFE_CALL(glGenerateMipmap(textureType));

		GL_SAFE_CALL(glBindTexture(textureType, 0));

		return RC_OK;
	}

	void COGLGraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
		GL_SAFE_VOID_CALL(glDrawArrays(COGLMappings::GetPrimitiveTopology(topology), startVertex, numOfVertices));
	}

	void COGLGraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		intptr_t indexOffset = startIndex * ((indexFormatType == E_INDEX_FORMAT_TYPE::INDEX16) ? 2 : 4);
		GL_SAFE_VOID_CALL(glDrawElementsBaseVertex(COGLMappings::GetPrimitiveTopology(topology), numOfIndices, COGLMappings::GetIndexFormat(indexFormatType), reinterpret_cast<void*>(indexOffset), baseVertex));
	}

	void COGLGraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		GL_SAFE_VOID_CALL(glDrawArraysInstanced(COGLMappings::GetPrimitiveTopology(topology), startVertex, verticesPerInstance, numOfInstances));
	}

	void COGLGraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
												   U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		GL_SAFE_VOID_CALL(glDrawElementsInstanced(COGLMappings::GetPrimitiveTopology(topology), indicesPerInstance, COGLMappings::GetIndexFormat(indexFormatType), 0, numOfInstances));
	}

	void COGLGraphicsContext::DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		auto pArgsBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(argsBufferHandle);
		if (!pArgsBuffer)
		{
			return;
		}

		GL_SAFE_VOID_CALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, pArgsBuffer->GetOGLHandle()));
		GL_SAFE_VOID_CALL(glDrawArraysIndirect(COGLMappings::GetPrimitiveTopology(topology), reinterpret_cast<void*>(alignedOffset)));
		GL_SAFE_VOID_CALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	void COGLGraphicsContext::DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		auto pArgsBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(argsBufferHandle);
		if (!pArgsBuffer)
		{
			return;
		}

		GL_SAFE_VOID_CALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, pArgsBuffer->GetOGLHandle()));
		GL_SAFE_VOID_CALL(glDrawElementsIndirect(COGLMappings::GetPrimitiveTopology(topology), COGLMappings::GetIndexFormat(indexFormatType), reinterpret_cast<void*>(alignedOffset)));
		GL_SAFE_VOID_CALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	void COGLGraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
		GL_SAFE_VOID_CALL(glDispatchCompute(groupsCountX, groupsCountY, groupsCountZ));
		GL_SAFE_VOID_CALL(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
	}

	void COGLGraphicsContext::DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		auto pArgsBuffer = mpGraphicsObjectManagerImpl->GetOGLBufferPtr(argsBufferHandle);
		if (!pArgsBuffer)
		{
			return;
		}

		GL_SAFE_VOID_CALL(glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, pArgsBuffer->GetOGLHandle()));
		GL_SAFE_VOID_CALL(glDispatchComputeIndirect(0)); // \todo Replace with alignedOffset
		GL_SAFE_VOID_CALL(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
		GL_SAFE_VOID_CALL(glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0));
	}

	void COGLGraphicsContext::BindBlendState(TBlendStateId blendStateId)
	{
		if (blendStateId == TBlendStateId::Invalid)
		{
			GL_SAFE_VOID_CALL(glDisable(GL_BLEND));
			return;
		}

		const TBlendStateDesc& blendStateDesc = mpGraphicsObjectManagerImpl->GetBlendState(blendStateId).Get();

		auto stateFunction = blendStateDesc.mIsEnabled ? glEnable : glDisable;
		GL_SAFE_VOID_CALL(stateFunction(GL_BLEND));

		GL_SAFE_VOID_CALL(glBlendFuncSeparate(COGLMappings::GetBlendFactorValue(blendStateDesc.mScrValue),
											  COGLMappings::GetBlendFactorValue(blendStateDesc.mDestValue),
											  COGLMappings::GetBlendFactorValue(blendStateDesc.mScrAlphaValue),
											  COGLMappings::GetBlendFactorValue(blendStateDesc.mDestAlphaValue)));

		GL_SAFE_VOID_CALL(glBlendEquationSeparate(COGLMappings::GetBlendOpType(blendStateDesc.mOpType),
								COGLMappings::GetBlendOpType(blendStateDesc.mAlphaOpType)));

		GL_SAFE_VOID_CALL(glColorMask(blendStateDesc.mWriteMask & 0x8, blendStateDesc.mWriteMask & 0x4, blendStateDesc.mWriteMask & 0x2, blendStateDesc.mWriteMask & 0x1));
	}

	void COGLGraphicsContext::BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef)
	{
		if (depthStencilStateId == TDepthStencilStateId::Invalid)
		{
			// \note set up default values for depth-stencil state
			GL_SAFE_VOID_CALL(glEnable(GL_DEPTH_TEST));
			GL_SAFE_VOID_CALL(glDepthMask(GL_TRUE));
			GL_SAFE_VOID_CALL(glDepthFunc(GL_LESS));

			// \note default settings for stencil test
			GL_SAFE_VOID_CALL(glDisable(GL_STENCIL_TEST));
			GL_SAFE_VOID_CALL(glStencilMask(0x0));

			return;
		}

		const TDepthStencilStateDesc& depthStencilState = mpGraphicsObjectManagerImpl->GetDepthStencilState(depthStencilStateId).Get();

		auto stateActivationFunction = depthStencilState.mIsDepthTestEnabled ? glEnable : glDisable;

		GL_SAFE_VOID_CALL(stateActivationFunction(GL_DEPTH_TEST));
		GL_SAFE_VOID_CALL(glDepthMask(depthStencilState.mIsDepthWritingEnabled));
		GL_SAFE_VOID_CALL(glDepthFunc(COGLMappings::GetComparisonFunc(depthStencilState.mDepthCmpFunc)));

		// \note stencil's parameters
		stateActivationFunction = depthStencilState.mIsStencilTestEnabled ? glEnable : glDisable;

		GL_SAFE_VOID_CALL(stateActivationFunction(GL_STENCIL_TEST));
		GL_SAFE_VOID_CALL(glStencilMask(depthStencilState.mStencilWriteMaskValue));
		GL_SAFE_VOID_CALL(glStencilOpSeparate(GL_FRONT, COGLMappings::GetStencilOpType(depthStencilState.mStencilFrontFaceOp.mFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilFrontFaceOp.mDepthFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilFrontFaceOp.mPassOp)));
		GL_SAFE_VOID_CALL(glStencilOpSeparate(GL_BACK, COGLMappings::GetStencilOpType(depthStencilState.mStencilBackFaceOp.mFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilBackFaceOp.mDepthFailOp),
							COGLMappings::GetStencilOpType(depthStencilState.mStencilBackFaceOp.mPassOp)));
	}

	void COGLGraphicsContext::BindRasterizerState(TRasterizerStateId rasterizerStateId)
	{
		if (rasterizerStateId == TRasterizerStateId::Invalid)
		{
			GL_SAFE_VOID_CALL(glEnable(GL_CULL_FACE));

			return;
		}

		const TRasterizerStateDesc& stateDesc = mpGraphicsObjectManagerImpl->GetRasterizerState(rasterizerStateId).Get();

		auto stateActivationFunction = stateDesc.mCullMode != E_CULL_MODE::NONE ? glEnable : glDisable;
		
		// \note Culling
		GL_SAFE_VOID_CALL(stateActivationFunction(GL_CULL_FACE));
		if (stateDesc.mCullMode != E_CULL_MODE::NONE)
		{
			GL_SAFE_VOID_CALL(glCullFace(COGLMappings::GetCullMode(stateDesc.mCullMode)));
		}
		GL_SAFE_VOID_CALL(glFrontFace(stateDesc.mIsFrontCCWEnabled ? GL_CCW : GL_CW));
		
		// \note Scissor testing
		stateActivationFunction = stateDesc.mIsScissorTestEnabled ? glEnable : glDisable;
		GL_SAFE_VOID_CALL(stateActivationFunction(GL_SCISSOR_TEST));

		GL_SAFE_VOID_CALL(glPolygonMode(GL_FRONT_AND_BACK, stateDesc.mIsWireframeModeEnabled ? GL_LINE : GL_FILL));

		// \todo Implement support of depth bias and clipping
	}

	void COGLGraphicsContext::BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			LOG_WARNING("[COGLGraphicsContext] Render target's slot goes out of limits");
			return;
		}

		if (!slot && !pRenderTarget)
		{
			GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));

			for (U8 i = 0; i < mMaxNumOfRenderTargets; ++i)
			{
				GL_SAFE_VOID_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0));
			}

			GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

			return;
		}
				
		COGLRenderTarget* pOGLRenderTarget = dynamic_cast<COGLRenderTarget*>(pRenderTarget);
		mRenderTargets[slot] = pOGLRenderTarget->GetInternalHandler();

		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));
		GL_SAFE_VOID_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, mRenderTargets[slot], 0));
		GL_SAFE_VOID_CALL(glDrawBuffer(GL_COLOR_ATTACHMENT0 + slot));
		GL_SAFE_VOID_CALL(glReadBuffer(GL_COLOR_ATTACHMENT0 + slot));

		TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
	}

	void COGLGraphicsContext::BindRenderTarget(U8 slot, TTextureHandleId targetHandle)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			LOG_WARNING("[COGLGraphicsContext] Render target's slot goes out of limits");
			return;
		}

		if (!slot && TTextureHandleId::Invalid == targetHandle)
		{
			GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));

			for (U8 i = 0; i < mMaxNumOfRenderTargets; ++i)
			{
				GL_SAFE_VOID_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0));
			}

			GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

			return;
		}

		auto pRenderTargetTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(targetHandle);
		if (!pRenderTargetTexture)
		{
			return;
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET != (pRenderTargetTexture->GetParams().mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			TDE2_ASSERT_MSG(false, "[COGLGraphicsContext] Try to bind texture that is not a render target to render target slot");
			return;
		}

		mRenderTargets[slot] = pRenderTargetTexture->GetTextureHandle();

		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));
		GL_SAFE_VOID_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, mRenderTargets[slot], 0));
		GL_SAFE_VOID_CALL(glDrawBuffer(GL_COLOR_ATTACHMENT0 + slot));
		GL_SAFE_VOID_CALL(glReadBuffer(GL_COLOR_ATTACHMENT0 + slot));

		TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
	}

	void COGLGraphicsContext::BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite)
	{
		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));

		if (!pDepthBufferTarget)
		{
			mCurrDepthBufferHandle = mMainDepthStencilRenderbuffer;

			GL_SAFE_VOID_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0));
			GL_SAFE_VOID_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mCurrDepthBufferHandle));

			TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
			return;
		}

		COGLDepthBufferTarget* pGLDepthBuffer = dynamic_cast<COGLDepthBufferTarget*>(pDepthBufferTarget);

		GL_SAFE_VOID_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0));
		GL_SAFE_VOID_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mCurrDepthBufferHandle = pGLDepthBuffer->GetInternalHandler(), 0));

		if (disableRTWrite)
		{
			GL_SAFE_VOID_CALL(glDrawBuffer(GL_NONE));
			GL_SAFE_VOID_CALL(glReadBuffer(GL_NONE));
		}

		const auto checkStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == checkStatus);
	}

	void COGLGraphicsContext::BindDepthBufferTarget(TTextureHandleId targetHandle, bool disableRTWrite)
	{
		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));

		if (TTextureHandleId::Invalid == targetHandle)
		{
			mCurrDepthBufferHandle = mMainDepthStencilRenderbuffer;

			GL_SAFE_VOID_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0));
			GL_SAFE_VOID_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mCurrDepthBufferHandle));

			TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
			return;
		}

		auto pDepthBufferTexture = mpGraphicsObjectManagerImpl->GetOGLTexturePtr(targetHandle);
		if (!pDepthBufferTexture)
		{
			return;
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER != (pDepthBufferTexture->GetParams().mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			TDE2_ASSERT_MSG(false, "[COGLGraphicsContext] Try to bind texture that is not a depth buffer to depth buffer slot");
			return;
		}

		GL_SAFE_VOID_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0));
		GL_SAFE_VOID_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mCurrDepthBufferHandle = pDepthBufferTexture->GetTextureHandle(), 0));

		if (disableRTWrite)
		{
			GL_SAFE_VOID_CALL(glDrawBuffer(GL_NONE));
			GL_SAFE_VOID_CALL(glReadBuffer(GL_NONE));
		}

		const auto checkStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == checkStatus);
	}

	void COGLGraphicsContext::SetDepthBufferEnabled(bool value)
	{
		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));
		GL_SAFE_VOID_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, value ? mMainDepthStencilRenderbuffer : 0));
	}

	const TGraphicsCtxInternalData& COGLGraphicsContext::GetInternalData() const
	{
		return mInternalDataObject;
	}

	E_ENGINE_SUBSYSTEM_TYPE COGLGraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}
	
	IGraphicsObjectManager* COGLGraphicsContext::GetGraphicsObjectManager() const
	{
		return mpGraphicsObjectManagerImpl;
	}

	TPtr<IWindowSystem> COGLGraphicsContext::GetWindowSystem() const
	{
		return mpWindowSystem;
	}

	std::vector<U8> COGLGraphicsContext::GetBackBufferData() const
	{
		std::vector<U8> imageData;

		const size_t imageDataSize = mpWindowSystem->GetWidth() * mpWindowSystem->GetHeight() * sizeof(U32);
		imageData.resize(imageDataSize);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());

		constexpr I32 numOfChannels = 4;
		C8 rgb[numOfChannels];

		U8* pData = imageData.data();

		for (I32 y = 0; y < static_cast<I32>(mpWindowSystem->GetHeight()) / 2; ++y)
		{
			for (I32 x = 0; x < static_cast<I32>(mpWindowSystem->GetWidth()); ++x)
			{
				const I32 top = (x + y * mpWindowSystem->GetWidth()) * numOfChannels;
				const I32 bottom = (x + (mpWindowSystem->GetHeight() - y - 1) * mpWindowSystem->GetWidth()) * numOfChannels;

				memcpy(rgb, pData + top, sizeof(rgb));
				memcpy(pData + top, pData + bottom, sizeof(rgb));
				memcpy(pData + bottom, rgb, sizeof(rgb));
			}
		}

		return std::move(imageData);
	}

	F32 COGLGraphicsContext::GetPositiveZAxisDirection() const
	{
		return 1.0f;
	}

	E_RESULT_CODE COGLGraphicsContext::OnEvent(const TBaseEvent* pEvent)
	{
		if (pEvent->GetEventType() != TOnWindowResized::GetTypeId())
		{
			return RC_OK;
		}

		const TOnWindowResized* pOnWindowResizedEvent = dynamic_cast<const TOnWindowResized*>(pEvent);

		SetViewport(0.0f, 0.0f, static_cast<F32>(pOnWindowResizedEvent->mWidth), static_cast<F32>(pOnWindowResizedEvent->mHeight), 0.0f, 1.0f);

		GL_SAFE_CALL(glDeleteRenderbuffers(1, &mMainDepthStencilRenderbuffer)); /// Remove previously created depth-stencil buffer
		GL_SAFE_CALL(glGenRenderbuffers(1, &mMainDepthStencilRenderbuffer));

		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));
		GL_SAFE_VOID_CALL(glBindRenderbuffer(GL_RENDERBUFFER, mMainDepthStencilRenderbuffer));
		{
			defer([] { glBindRenderbuffer(GL_RENDERBUFFER, 0); glBindFramebuffer(GL_FRAMEBUFFER, 0); });

			/// \note Create a new depth-stencil with new sizes
			GL_SAFE_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pOnWindowResizedEvent->mWidth, pOnWindowResizedEvent->mHeight));
			GL_SAFE_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mMainDepthStencilRenderbuffer));

			TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

		return RC_OK;
	}

#if TDE2_DEBUG_MODE

	void COGLGraphicsContext::BeginSectionMarker(const std::string& id)
	{
		if (!GLEW_KHR_debug)
		{
			return;
		}

		GL_SAFE_VOID_CALL(glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, id.c_str()));
	}

	void COGLGraphicsContext::EndSectionMarker()
	{
		if (!GLEW_KHR_debug)
		{
			return;
		}

		GL_SAFE_VOID_CALL(glPopDebugGroup());
	}

#endif

	TEventListenerId COGLGraphicsContext::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TVideoAdapterInfo COGLGraphicsContext::GetInfo() const
	{
		std::string vendorName { reinterpret_cast<const C8*>(glGetString(GL_VENDOR)) };

		TVideoAdapterInfo::E_VENDOR_TYPE vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::UNKNOWN;

		if (Wrench::StringUtils::StartsWith(vendorName, "NVIDIA"))
		{
			vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::NVIDIA;
		}
		else if (Wrench::StringUtils::StartsWith(vendorName, "ATI"))
		{
			vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::AMD;
		}
		else if (Wrench::StringUtils::StartsWith(vendorName, "INTEL"))
		{
			vendorType = TVideoAdapterInfo::E_VENDOR_TYPE::INTEL;
		}

		TVideoAdapterInfo outputInfo;
		//outputInfo.mAvailableVideoMemory = adapterInfo.DedicatedVideoMemory;
		outputInfo.mVendorType = vendorType;
		
		return outputInfo;
	}

	const TGraphicsContextInfo& COGLGraphicsContext::GetContextInfo() const
	{
		const static TGraphicsContextInfo infoData
		{
			{ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } },
			true,
			E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_OPENGL3X
		};

		return infoData;
	}

	E_RESULT_CODE COGLGraphicsContext::_initFBO(const IWindowSystem* pWindowSystem)
	{
		// \todo move this code into FBO manager later
		U32 creationFlags = pWindowSystem->GetFlags();
		
		GL_SAFE_CALL(glGenFramebuffers(1, &mMainFBOHandler));

		if (!(creationFlags & P_ZBUFFER_ENABLED))
		{
			return RC_OK;
		}

		GL_SAFE_CALL(glGenRenderbuffers(1, &mMainDepthStencilRenderbuffer));

		GL_SAFE_VOID_CALL(glBindFramebuffer(GL_FRAMEBUFFER, mMainFBOHandler));
		GL_SAFE_VOID_CALL(glBindRenderbuffer(GL_RENDERBUFFER, mMainDepthStencilRenderbuffer));
		{
			LOG_MESSAGE(Wrench::StringUtils::Format("[COGLGraphicsContext] SRGB framebuffer status, enabled: {0}", !(creationFlags & P_HARDWARE_GAMMA_CORRECTION)));
			GL_SAFE_CALL(((creationFlags & P_HARDWARE_GAMMA_CORRECTION) ? glDisable : glEnable)(GL_FRAMEBUFFER_SRGB));

			defer([] { glBindRenderbuffer(GL_RENDERBUFFER, 0); glBindFramebuffer(GL_FRAMEBUFFER, 0); });

			GL_SAFE_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pWindowSystem->GetWidth(), pWindowSystem->GetHeight()));
			GL_SAFE_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mMainDepthStencilRenderbuffer));
			
			TDE2_ASSERT(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

		mCurrDepthBufferHandle = mMainDepthStencilRenderbuffer;

		return RC_OK;
	}


	IGraphicsContext* CreateOGLGraphicsContext(TPtr<IWindowSystem> pWindowSystem, TCreateGLContextFactoryCallback glContextFactoryCallback, E_RESULT_CODE& result)
	{
		COGLGraphicsContext* pGraphicsContextInstance = new (std::nothrow) COGLGraphicsContext(glContextFactoryCallback);

		if (!pGraphicsContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGraphicsContextInstance->Init(pWindowSystem);

		if (result != RC_OK)
		{
			delete pGraphicsContextInstance;

			pGraphicsContextInstance = nullptr;
		}

		return dynamic_cast<IGraphicsContext*>(pGraphicsContextInstance);
	}
}