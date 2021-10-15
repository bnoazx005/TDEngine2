/*!
	\file CD3D11Shader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseShader.h>
#include <utils/Config.h>
#include <string>


#if defined (TDE2_USE_WINPLATFORM)

#include <d3d11.h>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	/*!
		\brief The structure describes a common representation of a shader's bytecode
	*/

	struct TShaderBytecodeDesc
	{
		U8* mpBytecode;

		U32 mLength;
	};


	/*!
		\brief A factory function for creation objects of CD3D11Shader's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11Shader's implementation
	*/

	TDE2_API IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);

	/*!
		class CD3D11Shader

		\brief The class is a common implementation for all platforms
	*/

	class CD3D11Shader : public CBaseShader
	{
		public:
			friend TDE2_API IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CD3D11Shader)

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

			/*!
				\brief The method returns a bytecode of a vertex shader, or nullptr if a shader hasn't compiled yet

				\return The method returns a bytecode of a vertex shader, or nullptr if a shader hasn't compiled yet
			*/

			TDE2_API const TShaderBytecodeDesc& GetVertexShaderBytecode() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11Shader)

			TDE2_API E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			TDE2_API E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);

			TDE2_API void _bindUniformBuffer(U32 slot, IConstantBuffer* pBuffer) override;
		protected:
			ID3D11DeviceContext*  mp3dDeviceContext;

			ID3D11VertexShader*   mpVertexShader;
			
			ID3D11PixelShader*    mpPixelShader;

			ID3D11GeometryShader* mpGeometryShader;

			TShaderBytecodeDesc   mVertexShaderBytecode;
	};


	/*!
		\brief A factory function for creation objects of CD3D11ShaderFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateD3D11ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CD3D11ShaderFactory

		\brief The class is an abstract factory of CD3D11Shader objects that
		is used by a resource manager
	*/

	class CD3D11ShaderFactory : public CBaseObject, public IShaderFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateD3D11ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11ShaderFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}

#endif