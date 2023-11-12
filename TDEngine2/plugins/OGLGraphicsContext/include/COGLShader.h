/*!
	\file COGLShader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseShader.h>
#include <utils/Config.h>
#include <string>
#include <unordered_map>
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
		
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLShader's implementation
	*/

	TDE2_API IShader* CreateOGLShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);

	/*!
		class COGLShader

		\brief The class is a common implementation for all platforms
	*/

	class COGLShader : public CBaseShader
	{
		public:
			friend TDE2_API IShader* CreateOGLShader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<U32, U32> TUniformBuffersMap;
		public:
			TDE2_REGISTER_TYPE(COGLShader)

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			TDE2_API void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			TDE2_API void Unbind() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLShader)

			TDE2_API E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			TDE2_API E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);

			TDE2_API E_RESULT_CODE _createTexturesHashTable(const TShaderCompilerOutput* pCompilerData) override;

			TDE2_API void _bindUniformBuffer(U32 slot, TBufferHandleId uniformsBufferHandle) override;
		protected:
			GLuint             mShaderHandler;

			TUniformBuffersMap mUniformBuffersMap; ///< \note the hash table stores information about buffers indices that were assigned to them by GLSL compiler
	};


	/*!
		\brief A factory function for creation objects of COGLShaderFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLShaderFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateOGLShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class COGLShaderFactory

		\brief The class is an abstract factory of COGLShader objects that
		is used by a resource manager
	*/

	class COGLShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateOGLShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLShaderFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}