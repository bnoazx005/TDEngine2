/*!
	\file IPipeline.h
	\date 25.05.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"


namespace TDEngine2
{
	class IGraphicsContext;
	class IShaderImpl;


	enum class E_PIPELINE_TYPE : U32
	{
		GRAPHICS,
		COMPUTE
	};


	class IPipeline: public virtual IBaseObject
	{
		public:
			TDE2_API virtual E_RESULT_CODE Bind() = 0;

			TDE2_API virtual E_PIPELINE_TYPE GetType() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPipeline)
	};


	/*!
		interface IGraphicsPipeline
	*/

	class IGraphicsPipeline: public virtual IPipeline
	{
		public:
			/*!
				\brief The method initializes an internal state of a pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in, out] pipelineConfig A desc with parameters of a graphics pipeline

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TGraphicsPipelineConfigDesc& pipelineConfig) = 0;

			TDE2_API virtual const TGraphicsPipelineConfigDesc& GetConfig() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGraphicsPipeline)
	};


	/*!
		interface IComputePipeline
	*/

	class IComputePipeline : public virtual IPipeline
	{
		public:
			/*!
				\brief The method initializes an internal state of a pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in, out] shaderId 

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const std::string& shaderId) = 0;

			TDE2_API virtual TPtr<IShaderImpl> GetShaderPtr() const = 0;
			TDE2_API virtual const std::string& GetShaderId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IComputePipeline)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IGraphicsPipeline)
	TDE2_DECLARE_SCOPED_PTR_INLINED(IComputePipeline)
}