/*!
	\file CFramePostProcessor.h
	\date 11.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IFramePostProcessor.h"
#include "./../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CFramePostProcessor's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFramePostProcessor's implementation
	*/

	TDE2_API IFramePostProcessor* CreateFramePostProcessor(E_RESULT_CODE& result);


	/*!
		class CFramePostProcessor

		\brief The class is a basic implementation of default in-engine frame post-processor
	*/

	class CFramePostProcessor : public CBaseObject, public IFramePostProcessor
	{
		public:
			friend TDE2_API IFramePostProcessor* CreateFramePostProcessor(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of the processor

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method assigns a processing profile into the processor

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetProcessingProfile(const IPostProcessingProfile* pProfileResource) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFramePostProcessor)
	};
}
