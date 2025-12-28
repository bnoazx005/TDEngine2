/*!
\file CBaseGraphicsObjectManager.h
\date 30.11.2018
\authors Kasimov Ildar
*/

#pragma once


#include "IGraphicsObjectManager.h"
#include "../core/CBaseObject.h"
#include "../utils/CResourceContainer.h"
#include <vector>
#include <list>


namespace TDEngine2
{
	class IBinaryFileReader;
	class IGraphicsPipeline;
	class IShaderCache;


	TDE2_DECLARE_SCOPED_PTR(IGraphicsPipeline);
	TDE2_DECLARE_SCOPED_PTR(IShaderCache);


	/*!
		class CBaseGraphicsObjectManager

		\brief The class is a common implementation for all graphics contexts
	*/

	class CBaseGraphicsObjectManager : public IGraphicsObjectManager, public CBaseObject
	{
		protected:
			typedef std::list<U32>                              TFreeEntitiesRegistry;
			typedef std::vector<IVertexDeclaration*>            TVertexDeclarationsArray;
			typedef std::unordered_map<U32, USIZE>              THashTable;
			template <typename T> using                         TStateHashesTable = std::unordered_map<U32, T>;
			typedef TStateHashesTable<TGraphicsPipelineStateId> TGraphicsPipelineStatesTable;
			typedef TStateHashesTable<TComputePipelineStateId>  TComputePipelineStatesTable;
			typedef TStateHashesTable<TShaderHandleId>          TShadersHashTable;
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\param[in, out] pGraphicsContext A pointer to implementation of IGraphicsContext interface
				\param[in, out] pFileSystem A pointer to implementation of IFileSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

			/*!
				\brief The method is a factory for creation of a debug helper object

				\param[in, out] pResourceManager A pointer to implementation of IResourceManager interface
				\param[in, out] pRenderer A pointer to implementation of IRenderer interface

				\return The result object contains either a pointer to IDebugUtility or an error code
			*/

			TDE2_API TResult<IDebugUtility*> CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer) override;

			TDE2_API TResult<TBufferHandleId> CreateBuffer(const TInitBufferParams& params) override;
			TDE2_API TResult<TTextureHandleId> CreateTexture(const TInitTextureImplParams& params) override;
			
			TDE2_API TResult<TGraphicsPipelineStateId> CreateGraphicsPipelineState(const TGraphicsPipelineConfigDesc& pipelineConfigDesc) override;
			TDE2_API TResult<TComputePipelineStateId> CreateComputePipelineState(const std::string& shaderId) override;

			TDE2_API TResult<TPtr<IShaderCache>> CreateShaderCache(bool isReadOnly = true) override;

			TDE2_API TResult<TShaderHandleId> LoadShader(const std::string& shaderId) override;

			TDE2_API E_RESULT_CODE SetShaderCompiler(TPtr<IShaderCompiler> pShaderCompiler) override;

			/*!
				\brief The method returns a pointer to IGraphicsContext
				\return The method returns a pointer to IGraphicsContext
			*/

			TDE2_API IGraphicsContext* GetGraphicsContext() const override;

			TDE2_API TPtr<IGraphicsPipeline> GetGraphicsPipeline(TGraphicsPipelineStateId handle) override;
			TDE2_API TPtr<IComputePipeline> GetComputePipeline(TComputePipelineStateId handle) override;

			/*!
				\brief The method convert input shader's name into E_DEFAULT_SHADER_TYPE's value

				\return The method returns a type of a default shader if its name corresponds to one of predefined strings
			*/

			TDE2_API static E_DEFAULT_SHADER_TYPE GetDefaultShaderTypeByName(const std::string& name);

			/*!
				Prefer to use CreateSampler if you need unique set of parameters for a texture

				\return The method returns one of prebuilt texture samplers based on given filtering type
			*/

			TDE2_API TTextureSamplerId GetDefaultTextureSampler(E_TEXTURE_FILTER_TYPE filterType) override;

			TDE2_API IVertexDeclaration* GetDefaultPositionOnlyVertexDeclaration() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseGraphicsObjectManager)

			TDE2_API virtual TPtr<IBuffer> _createBufferInternal(const TInitBufferParams& params) = 0;
			TDE2_API virtual TPtr<ITextureImpl> _createTextureInternal(const TInitTextureImplParams& params) = 0;
			TDE2_API virtual TPtr<IShaderImpl> _createShaderImplInternal(const std::string& shaderId) = 0;

			TDE2_API virtual TPtr<IGraphicsPipeline> _createGraphicsPipelineInternal(const TGraphicsPipelineConfigDesc& pipelineConfigDesc);
			TDE2_API virtual TPtr<IComputePipeline> _createComputePipelineInternal(const std::string& shaderId);

			TDE2_API virtual USIZE _insertBuffer(TPtr<IBuffer> pObject) = 0;
			TDE2_API virtual USIZE _insertTexture(TPtr<ITextureImpl> pObject) = 0;
			TDE2_API virtual USIZE _insertShaderImpl(TPtr<IShaderImpl> pObject) = 0;

			TDE2_API void _insertVertexDeclaration(IVertexDeclaration* pVertDecl);

			TDE2_API E_RESULT_CODE _freeVertexDeclarations();

			TDE2_API virtual E_RESULT_CODE _freeTextureSamplers() = 0;

			TDE2_API virtual E_RESULT_CODE _freeBlendStates() = 0;

			TDE2_API virtual E_RESULT_CODE _freeDepthStencilStates() = 0;

			TDE2_API virtual E_RESULT_CODE _freeRasterizerStates() = 0;

			TDE2_API virtual const std::string _getShaderCacheFilePath() const = 0;

			TDE2_API CPassKey<CBaseGraphicsObjectManager> _getPassKey() const;

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			IGraphicsContext*                                      mpGraphicsContext = nullptr;
			IFileSystem*                                           mpFileSystem = nullptr;

			TVertexDeclarationsArray                               mVertexDeclarationsArray;

			TFreeEntitiesRegistry                                  mFreeVertDeclsSlots;

			THashTable                                             mTextureSamplesHashTable;
			THashTable                                             mBlendStatesHashTable;

			IDebugUtility*                                         mpDebugUtility;

			IVertexDeclaration*                                    mpDefaultPositionOnlyVertDeclaration = nullptr;

			std::vector<TPtr<IGraphicsPipeline>>                   mpGraphicsPipelines;
			std::vector<TPtr<IComputePipeline>>                    mpComputePipelines;

			TShadersHashTable                                      mLoadedShadersTable{};

			TGraphicsPipelineStatesTable                           mGraphicsPipelinesHashTable;
			TComputePipelineStatesTable                            mComputePipelinesHashTable;

			TPtr<IShaderCompiler>                                  mpShaderCompiler;
			TPtr<IShaderCache>                                     mpShaderCache;

			std::unordered_map<U32, std::vector<TTextureHandleId>> mTransientTexturesPool{};
			std::unordered_map<U32, std::vector<TBufferHandleId>>  mTransientBuffersPool{};
	};
}