/*!
	\file CBaseGraphicsPipeline.h
	\date 25.05.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/CBaseObject.h"
#include "IGraphicsPipeline.h"


namespace TDEngine2
{
	class IGraphicsObjectManager;


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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseGraphicsPipeline)
		protected:
			IGraphicsContext*           mpGraphicsContext;
			IGraphicsObjectManager*     mpGraphicsObjectManager;
			TGraphicsPipelineConfigDesc mConfig;

			TBlendStateId               mBlendStateHandle = TBlendStateId::Invalid;
			TDepthStencilStateId        mDepthStencilStateHandle = TDepthStencilStateId::Invalid;
			TRasterizerStateId          mRasterizerStateHandle = TRasterizerStateId::Invalid;

	};
}