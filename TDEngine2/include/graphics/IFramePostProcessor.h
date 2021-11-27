/*!
	\file IFramePostProcessor.h
	\date 11.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IBaseObject.h"
#include <functional>


namespace TDEngine2
{
	class IPostProcessingProfile;
	class IRenderer;
	class IGraphicsObjectManager;
	class IWindowSystem;
	class IGlobalShaderProperties;


	typedef struct TFramePostProcessorParameters
	{
		IRenderer* mpRenderer;
		IGraphicsObjectManager* mpGraphicsObjectManager;
		IWindowSystem* mpWindowSystem;
	} TFramePostProcessorParameters, *TFramePostProcessorParametersPtr;


	/*!
		interface IFramePostProcessor

		\brief The interface describes a functionality of frames post-processor
	*/

	class IFramePostProcessor: public virtual IBaseObject
	{
		public:
			typedef std::function<void()> TRenderFrameCallback;
		public:
			/*!
				\brief The method initializes an internal state of the processor

				/param[in] desc A set of parameters that're needed to initialize the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TFramePostProcessorParameters& desc) = 0;

			/*!
				\brief The method assigns a processing profile into the processor

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetProcessingProfile(const IPostProcessingProfile* pProfileResource) = 0;

			/*!
				\brief The method implements all the logic that should be done before the actual frame'll be drawn

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PreRender() = 0;

			/*!
				\brief The method implements all the logic that should be done when all geometry is rendered and
				post processing effects are applied

				\param[in] onRenderFrameCallback A callback in which all the geometry of the scene should be rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Render(const TRenderFrameCallback& onRenderFrameCallback) = 0;

			/*!
				\brief The method implements all the logic that should be done after the actual frame'll be drawn

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PostRender() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFramePostProcessor)
	};


	TDE2_DECLARE_SCOPED_PTR(IFramePostProcessor)
}
