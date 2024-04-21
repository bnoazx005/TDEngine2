/*!
	\file CGlobalShaderProperties.h
	\date 20.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IGlobalShaderProperties.h"
#include "../core/CBaseObject.h"
#include "IBuffer.h"
#include <array>
#include <unordered_map>


namespace TDEngine2
{
	class IConstantBuffer;


	enum class TBufferHandleId : U32;


	/*!
		\brief A factory function for creation objects of CGlobalShaderProperties's type

		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CGlobalShaderProperties's implementation
	*/

	TDE2_API IGlobalShaderProperties* CreateGlobalShaderProperties(IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CGlobalShaderProperties

		\brief The class is an implementation of IGlobalShaderProperties interface which
		is used to manage uniforms that are globally available within all shaders
	*/

	class CGlobalShaderProperties : public CBaseObject, public IGlobalShaderProperties
	{
		public:
			friend TDE2_API IGlobalShaderProperties* CreateGlobalShaderProperties(IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method writes data into an internal uniforms buffer with specified register

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsObjectManager* pGraphicsObjectManager) override;

			/*!
				\brief The method writes data into an internal uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer, in which data will be written

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetInternalUniformsBuffer(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot, const U8* pData, U32 dataSize) override;

			/*!
				\brief The method writes data into one of the predefined shader typed buffer

				\param[in] slot A slot specifies the buffer, in which data will be written
				\param[in] pData A pointer to data that should be written into a buffer
				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetInternalShaderBuffer(E_INTERNAL_SHADER_BUFFERS_REGISTERS slot, const U8* pData, U32 dataSize) override;

			TDE2_API E_RESULT_CODE Bind() override;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGlobalShaderProperties)

			TDE2_API E_RESULT_CODE _initializeUniformsBuffers(IGraphicsObjectManager* pGraphicsObjectManager, U8 numOfBuffers);
			E_RESULT_CODE _initializeShaderBuffers(IGraphicsObjectManager* pGraphicsObjectManager);
		private:
			IGraphicsObjectManager*                                                  mpGraphicsObjectManager = nullptr;
			std::array<TBufferHandleId, TotalNumberOfInternalConstantBuffers>        mInternalEngineUniforms{};
			std::unordered_map<E_INTERNAL_SHADER_BUFFERS_REGISTERS, TBufferHandleId> mInternalShaderBuffers{};
	};
}