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
		\brief A factory function for creation objects of CD3D11Shader's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11Shader's implementation
	*/

	IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CD3D11Shader

		\brief The class is a common implementation for all platforms
	*/

	class CD3D11Shader : public CBaseShader
	{
		public:
			friend IShader* CreateD3D11Shader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CD3D11Shader)

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Reset() override;

			/*!
				\brief The method binds a shader to a rendering pipeline
			*/

			void Bind() override;

			/*!
				\brief The method rejects a shader from a rendering pipeline
			*/

			void Unbind() override;

			/*!
				\brief The method returns a bytecode of a vertex shader, or nullptr if a shader hasn't compiled yet

				\return The method returns a bytecode of a vertex shader, or nullptr if a shader hasn't compiled yet
			*/

			const std::vector<U8>& GetVertexShaderBytecode() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11Shader)

			E_RESULT_CODE _createInternalHandlers(const TShaderCompilerOutput* pCompilerData) override;

			E_RESULT_CODE _createUniformBuffers(const TShaderCompilerOutput* pCompilerData);
		protected:
			ID3D11DeviceContext*  mp3dDeviceContext;

			ID3D11VertexShader*   mpVertexShader;
			
			ID3D11PixelShader*    mpPixelShader;

			ID3D11GeometryShader* mpGeometryShader;

			ID3D11ComputeShader*  mpComputeShader;

			std::vector<U8>       mVertexShaderBytecode;
	};


	/*!
		\brief A factory function for creation objects of CD3D11ShaderFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderFactory's implementation
	*/

	IResourceFactory* CreateD3D11ShaderFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
}

#endif