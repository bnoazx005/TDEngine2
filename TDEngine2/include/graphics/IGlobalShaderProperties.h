/*!
	\file IGlobalShaderProperties.h
	\date 20.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../core/IBaseObject.h"
#include "./../utils/Types.h"
#include "./../utils/Utils.h"


namespace TDEngine2
{
	class IGraphicsObjectManager;


	/*!
		interface IGlobalShaderProperties

		\brief The interface describes a manager of a global shader properties.
		It is used to set up internal engine's uniforms which is available in every shader
	*/

	class IGlobalShaderProperties : public virtual IBaseObject
	{
		public:
			/*!
			\brief The method writes data into an internal uniforms buffer with specified register

			\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

			\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsObjectManager* pGraphicsObjectManager) = 0;

			/*!
				\brief The method writes data into an internal uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer, in which data will be written

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetInternalUniformsBuffer(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot, const U8* pData, U32 dataSize) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGlobalShaderProperties)
	};
}