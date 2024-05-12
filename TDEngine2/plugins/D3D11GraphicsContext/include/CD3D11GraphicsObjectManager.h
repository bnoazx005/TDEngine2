/*!
	\file CD3D11GraphicsObjectManager.h
	\date 30.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseGraphicsObjectManager.h>
#include <unordered_map>
#include <vector>


#if defined (TDE2_USE_WINPLATFORM)


namespace TDEngine2
{
	class CD3D11TextureImpl;
	class CD3D11Buffer;


	TDE2_DECLARE_SCOPED_PTR(CD3D11TextureImpl)
	TDE2_DECLARE_SCOPED_PTR(CD3D11Buffer)


	/*!
		\brief A factory function for creation objects of CD3D11GraphicsObjectManager's type
		
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11GraphicsObjectManager's implementation
	*/

	TDE2_API IGraphicsObjectManager* CreateD3D11GraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CD3D11GraphicsObjectManager

		\brief The class is an implementation of IGraphicsObjectManager for the D3D11 graphics context
	*/

	class CD3D11GraphicsObjectManager : public CBaseGraphicsObjectManager
	{
		public:
			friend TDE2_API IGraphicsObjectManager* CreateD3D11GraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		protected:
			typedef std::vector<ID3D11SamplerState*>             TTextureSamplersArray;
			typedef CResourceContainer<ID3D11BlendState*>        TBlendStatesArray;
			typedef CResourceContainer<ID3D11DepthStencilState*> TDepthStencilStatesArray;
			typedef CResourceContainer<ID3D11RasterizerState*>   TRasterizerStatesArray;
			template <typename T> using                          TStateHashesTable = std::unordered_map<U32, T>;
			typedef TStateHashesTable<TDepthStencilStateId>      TDepthStencilStatesTable;
			typedef TStateHashesTable<TRasterizerStateId>        TRasterizerStatesTable;
			typedef std::vector<TPtr<CD3D11TextureImpl>>         TNativeTexturesArray;
			typedef std::vector<TPtr<CD3D11Buffer>>              TNativeBuffersArray;
		public:
			TDE2_API TResult<TBufferHandleId> CreateBuffer(const TInitBufferParams& params) override;
			TDE2_API TResult<TTextureHandleId> CreateTexture(const TInitTextureImplParams& params) override;

			TDE2_API E_RESULT_CODE DestroyBuffer(TBufferHandleId bufferHandle) override;
			TDE2_API E_RESULT_CODE DestroyTexture(TTextureHandleId textureHandle) override;

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

			TDE2_API TResult<TBlendStateId> CreateBlendState(const TBlendStateDesc& blendStateDesc) override;

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
				\brief The method returns a pointer to ID3D11SamplerState which is related with a given identifier

				\param[in] texSamplerId An identifier of a texture sampler

				\return Either a pointer to ID3D11SamplerState object or an error code
			*/

			TDE2_API TResult<ID3D11SamplerState*> GetTextureSampler(TTextureSamplerId texSamplerId) const;

			/*!
				\brief The method returns a parameters of a blend state with a given identifier

				\param[in] blendStateId An identifier of a blend state

				\return Either a result object with state's object or an error code
			*/

			TDE2_API TResult<ID3D11BlendState*> GetBlendState(TBlendStateId blendStateId) const;

			/*!
				\brief The method returns a parameters of a depth-stencil state with a given identifier

				\param[in] stateId An identifier of a depth-stencil state

				\return Either a result object with state's object or an error code
			*/

			TDE2_API TResult<ID3D11DepthStencilState*> GetDepthStencilState(TDepthStencilStateId stateId) const;
			
			/*!
				\brief The method returns a parameters of a rasterizer state with a given identifier

				\param[in] rasterizerStateId An identifier of a rasterizer state

				\return Either a result object with state's description or an error code
			*/

			TDE2_API TResult<ID3D11RasterizerState*> GetRasterizerState(TRasterizerStateId rasterizerStateId) const;

			/*!
				\brief The method returns a string which contains full source code of default shader that is specific
				for the graphics context

				\param[in] type A type of a default shader

				\return The method returns a string which contains full source code of default shader that is specific
				for the graphics context
			*/

			TDE2_API std::string GetDefaultShaderCode(const E_DEFAULT_SHADER_TYPE& type) const override;

			TDE2_API TPtr<IBuffer> GetBufferPtr(TBufferHandleId handle) override;
			TDE2_API TPtr<CD3D11Buffer> GetD3D11BufferPtr(TBufferHandleId bufferHandle);

			TDE2_API TPtr<ITextureImpl> GetTexturePtr(TTextureHandleId handle) override;
			TDE2_API TPtr<CD3D11TextureImpl> GetD3D11TexturePtr(TTextureHandleId textureHandle);

			/*!
				\brief The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates

				\return The method returns vertices of a screen-quad triangle specific for the current GAPI.
				XY of each element mean the position and ZW are texture coordinates
			*/

			TDE2_API std::array<TVector4, 3> GetScreenTriangleVertices() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11GraphicsObjectManager)
			
			TDE2_API E_RESULT_CODE _freeTextureSamplers() override;

			TDE2_API E_RESULT_CODE _freeBlendStates() override;

			TDE2_API E_RESULT_CODE _freeDepthStencilStates() override;

			TDE2_API E_RESULT_CODE _freeRasterizerStates() override;

			TDE2_API const std::string _getShaderCacheFilePath() const override;
		protected:			
			TTextureSamplersArray    mpTextureSamplersArray;
			TBlendStatesArray        mpBlendStates;
			TDepthStencilStatesArray mpDepthStencilStatesArray;
			TRasterizerStatesArray   mpRasterizerStatesArray; 
			TDepthStencilStatesTable mDepthStencilStatesTable;
			TRasterizerStatesTable   mRasterizerStatesTable;

			TNativeTexturesArray     mpTexturesArray;
			TNativeBuffersArray      mpBuffersArray;

			std::unordered_map<U32, std::vector<TTextureHandleId>> mTransientTexturesPool;
			std::unordered_map<U32, std::vector<TBufferHandleId>>  mTransientBuffersPool;
	};
}

#endif