/*!
	\file BasePipelines.h
	\date 25.05.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/CBaseObject.h"
#include "../core/Serialization.h"
#include "IPipeline.h"


namespace TDEngine2
{
	class IGraphicsObjectManager;


	enum class TShaderHandleId : U32;


	TDE2_API IGraphicsPipeline* CreateBaseGraphicsPipeline(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& pipelineConfig, E_RESULT_CODE& result);


	/*!
		class CBaseGraphicsPipeline
	*/
	
	class CBaseGraphicsPipeline : public CBaseObject, public IGraphicsPipeline
	{
		public:
			friend TDE2_API IGraphicsPipeline* CreateBaseGraphicsPipeline(IGraphicsContext*, const TGraphicsPipelineConfigDesc&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in, out] pipelineConfig A desc with parameters of a graphics pipeline

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& pipelineConfig) override;

			TDE2_API E_RESULT_CODE Bind() override;

			TDE2_API const TGraphicsPipelineConfigDesc& GetConfig() const override;
			TDE2_API E_PIPELINE_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseGraphicsPipeline)
		protected:
			IGraphicsContext*           mpGraphicsContext = nullptr;
			IGraphicsObjectManager*     mpGraphicsObjectManager = nullptr;
			TGraphicsPipelineConfigDesc mConfig;

			TBlendStateId               mBlendStateHandle = TBlendStateId::Invalid;
			TDepthStencilStateId        mDepthStencilStateHandle = TDepthStencilStateId::Invalid;
			TRasterizerStateId          mRasterizerStateHandle = TRasterizerStateId::Invalid;
			TShaderHandleId             mCachedShaderHandle;
	};


	// \note The reason why there are defined Serialize/Deserialize for TGraphicsPipelineConfigDesc instead of making CBaseGraphicsPipeline ISerializable
	// is that we should know config before we create a new instance of the type

	template <> TDE2_API E_RESULT_CODE Serialize<TGraphicsPipelineConfigDesc>(class IArchiveWriter* pWriter, const TGraphicsPipelineConfigDesc& value);
	template <> TDE2_API TResult<TGraphicsPipelineConfigDesc> Deserialize<TGraphicsPipelineConfigDesc>(IArchiveReader* pReader);


	TDE2_API IComputePipeline* CreateBaseComputePipeline(IGraphicsContext* pGraphicsContext, const std::string& shaderId, E_RESULT_CODE& result);


	/*!
		class CBaseComputePipeline
	*/

	class CBaseComputePipeline : public CBaseObject, public IComputePipeline
	{
		public:
			friend TDE2_API IComputePipeline* CreateBaseComputePipeline(IGraphicsContext*, const std::string&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in, out] shaderId

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const std::string& shaderId) override;

			TDE2_API E_RESULT_CODE Bind() override;

			TDE2_API TPtr<IShaderImpl> GetShaderPtr() const override;
			TDE2_API const std::string& GetShaderId() const override;
			TDE2_API E_PIPELINE_TYPE GetType() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseComputePipeline)
		protected:
			IGraphicsContext*       mpGraphicsContext = nullptr;
			IGraphicsObjectManager* mpGraphicsObjectManager = nullptr;
			
			mutable TShaderHandleId mCachedShaderHandle;
			std::string             mShaderIdStr;
	};
}