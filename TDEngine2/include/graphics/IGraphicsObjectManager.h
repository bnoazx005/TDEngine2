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
	class IGraphicsContext;
	class IVertexDeclaration;
	class IDebugUtility;
	class IRenderer;
	class IResourceManager;
	class IShaderCache;
	class IFileSystem;
	class IBuffer;
	class ITextureImpl;
	class IGraphicsPipeline;
	struct TStructuredBuffersInitParams;
	struct TInitBufferParams;
	struct TInitTextureImplParams;
	struct TGraphicsPipelineConfigDesc;


	TDE2_DECLARE_SCOPED_PTR(IShaderCache);
	TDE2_DECLARE_SCOPED_PTR(IBuffer);
	TDE2_DECLARE_SCOPED_PTR(ITextureImpl);
	TDE2_DECLARE_SCOPED_PTR(IGraphicsPipeline);


	/*!
		enum class E_DEFAULT_SHADER_TYPE

		\brief The enumeration contains all available default shaders types
	*/

	enum class E_DEFAULT_SHADER_TYPE : U16
	{
		DST_BASIC,
	};


	TDE2_DECLARE_HANDLE_TYPE(TBufferHandleId);
	TDE2_DECLARE_HANDLE_TYPE(TTextureHandleId);


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
			TDE2_API virtual E_RESULT_CODE DestroyBuffer(TBufferHandleId bufferHandle) = 0;

			TDE2_API virtual TResult<TTextureHandleId> CreateTexture(const TInitTextureImplParams& params) = 0;
			TDE2_API virtual E_RESULT_CODE DestroyTexture(TTextureHandleId textureHandle) = 0;

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

			TDE2_API virtual TResult<TGraphicsPipelineStateId> CreateGraphicsPipelineState(const TGraphicsPipelineConfigDesc& pipelineConfigDesc) = 0;

			/*!
				\brief The method is a factory for creation of a debug helper object

				\param[in, out] pRenderer A pointer to implementation of IRenderer interface

				\return The result object contains either a pointer to IDebugUtility or an error code
			*/

			TDE2_API virtual TResult<IDebugUtility*> CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer) = 0;

			TDE2_API virtual TResult<TPtr<IShaderCache>> CreateShaderCache(IFileSystem* pFileSystem, bool isReadOnly = true) = 0;

			TDE2_API virtual TPtr<IBuffer> GetBufferPtr(TBufferHandleId handle) = 0;
			TDE2_API virtual TPtr<ITextureImpl> GetTexturePtr(TTextureHandleId handle) = 0;

			TDE2_API virtual TPtr<IGraphicsPipeline> GetGraphicsPipeline(TGraphicsPipelineStateId handle) = 0;

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
				Prefer to use CreateSampler if you need unique set of parameters for a texture

				\return The method returns one of prebuilt texture samplers based on given filtering type
			*/

			TDE2_API virtual TTextureSamplerId GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE filterType) = 0;

			TDE2_API virtual IVertexDeclaration* GetDefaultPositionOnlyVertexDeclaration() = 0;

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
