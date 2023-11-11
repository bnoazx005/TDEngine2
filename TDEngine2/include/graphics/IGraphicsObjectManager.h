/*!
	\file IGraphicsObjectManager.h
	\date 30.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/IBaseObject.h"
#include "../graphics/IBuffer.h"
#include "../math/TVector4.h"
#include <array>


namespace TDEngine2
{
	class IVertexBuffer;
	class IConstantBuffer;
	class IStructuredBuffer;
	class IGraphicsContext;
	class IVertexDeclaration;
	class IDebugUtility;
	class IRenderer;
	class IResourceManager;
	class IShaderCache;
	class IFileSystem;
	class IBuffer;
	struct TStructuredBuffersInitParams;
	struct TInitBufferParams;


	TDE2_DECLARE_SCOPED_PTR(IShaderCache);
	TDE2_DECLARE_SCOPED_PTR(IBuffer);


	/*!
		enum class E_DEFAULT_SHADER_TYPE

		\brief The enumeration contains all available default shaders types
	*/

	enum class E_DEFAULT_SHADER_TYPE : U16
	{
		DST_BASIC,
	};


	TDE2_DECLARE_HANDLE_TYPE(TBufferHandleId);


	/*!
		interface IGraphicsObjectManager

		\brief The interface describes a functionality of a manager, which
		creates new instances of low-level graphics objects (buffers, states, etc)
		It shouldn't be used by end user, for internal usage only!
	*/

	class IGraphicsObjectManager : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\param[in] pGraphicsContext A pointer to implementation of IGraphicsContext interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) = 0;

			TDE2_API virtual TResult<TBufferHandleId> CreateBuffer(const TInitBufferParams& params) = 0;

			/*!
				\brief The method is a factory for creation objects of IVertexDeclaration's type

				\return The result object contains either a pointer to IVertexDeclaration or an error code
			*/

			TDE2_API virtual TResult<IVertexDeclaration*> CreateVertexDeclaration() = 0;

			/*!
				\brief The method is a factory for creation of texture samplers objects

				\param[in] samplerDesc A structure that contains parameters which will be assigned to a new created sampler

				\return The result object contains either an identifier of created sampler or an error code
			*/

			TDE2_API virtual TResult<TTextureSamplerId> CreateTextureSampler(const TTextureSamplerDesc& samplerDesc) = 0;

			/*!
				\brief The method creates a new blend state which is configured via given parameters

				\param[in] blendStateDesc A structure that contains parameters which will be assigned to a new created state

				\return The result object contains either an identifier of created blend state or an error code
			*/

			TDE2_API virtual TResult<TBlendStateId> CreateBlendState(const TBlendStateDesc& blendStateDesc) = 0;

			/*!
				\brief The method creates a new state which sets up depth and stencil buffers operations and values

				\param[in] depthStencilDesc A structure that contains settings, which the state should set up, each time it's used

				\return The result object contains either an identifier of a created state or an error code
			*/

			TDE2_API virtual TResult<TDepthStencilStateId> CreateDepthStencilState(const TDepthStencilStateDesc& depthStencilDesc) = 0;

			/*!
				\brief The method create a new state which specifies how a geometry should be actually drawn
				
				\param[in] rasterizerStateDesc A type that contains settings for a rasterizer
				
				\return The result object contains either an identifier of a created state or an error code
			*/

			TDE2_API virtual TResult<TRasterizerStateId> CreateRasterizerState(const TRasterizerStateDesc& rasterizerStateDesc) = 0;

			/*!
				\brief The method is a factory for creation of a debug helper object

				\param[in, out] pRenderer A pointer to implementation of IRenderer interface

				\return The result object contains either a pointer to IDebugUtility or an error code
			*/

			TDE2_API virtual TResult<IDebugUtility*> CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer) = 0;

			TDE2_API virtual TResult<TPtr<IShaderCache>> CreateShaderCache(IFileSystem* pFileSystem, bool isReadOnly = true) = 0;

			TDE2_API virtual TPtr<IBuffer> GetBufferPtr(TBufferHandleId handle) = 0;

			/*!
				\brief The method returns a pointer to IGraphicsContext
				\return The method returns a pointer to IGraphicsContext
			*/

			TDE2_API virtual IGraphicsContext* GetGraphicsContext() const = 0;

			/*!
				\brief The method returns a string which contains full source code of default shader that is specific
				for the graphics context 

				\param[in] type A type of a default shader

				\return The method returns a string which contains full source code of default shader that is specific
				for the graphics context 
			*/

			TDE2_API virtual std::string GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const = 0;

			/*!
				\brief The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates

				\return The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates
			*/

			TDE2_API virtual std::array<TVector4, 3> GetScreenTriangleVertices() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGraphicsObjectManager)
	};
}
