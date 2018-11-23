/*!
	\file COGLShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShader.h>
#include <utils/Config.h>
#include <core/IResourceFactory.h>
#include <string>
#include <GL/glew.h>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	/*!
		\brief A factory function for creation objects of COGLShader's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] id An identifier of a resource

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLShader's implementation
	*/

	TDE2_API IShader* CreateOGLShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
									  TResourceId id, E_RESULT_CODE& result);

	/*!
		class COGLShader

		\brief The class is a common implementation for all platforms
	*/

	class COGLShader : public CBaseShader
	{
		public:
			friend TDE2_API IShader* CreateOGLShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
													 TResourceId id, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(COGLShader)

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method writes data into an internal uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer, in which data will be written

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetInternalUniformsBuffer(E_INTERNAL_UNIFORM_BUFFER_REGISTERS slot, const U8* pData, U32 dataSize) override;

			/*!
				\brief The method writes data into a user-defined uniforms buffer with specified register

				\param[in] slot A slot specifies a uniforms buffer

				\param[in] pData A pointer to data that should be written into a buffer

				\param[in] dataSize A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetUserUniformsBuffer(U8 slot, const U8* pData, U32 dataSize) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLShader)

			TDE2_API E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			TDE2_API E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);
		protected:
			GLuint mShaderHandler;
	};


	/*!
		\brief A factory function for creation objects of COGLShaderFactory's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLShaderFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateOGLShaderFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class COGLShaderFactory

		\brief The class is an abstract factory of COGLShader objects that
		is used by a resource manager
	*/

	class COGLShaderFactory : public IShaderFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateOGLShaderFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const TBaseResourceParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLShaderFactory)
		protected:
			bool              mIsInitialized;

			IGraphicsContext* mpGraphicsContext;
	};
}