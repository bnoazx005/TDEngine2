/*!
	\file CD3D11Shader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShader.h>
#include <utils/Config.h>
#include <core/IResourceFactory.h>


#if defined (TDE2_USE_WIN32PLATFORM)

#include <d3d11.h>


namespace TDEngine2
{
	class IGraphicsContext;


	/*!
		class CD3D11Shader

		\brief The class is a common implementation for all platforms
	*/

	class CD3D11Shader : public IShader, public CBaseShader
	{
		public:
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11Shader)

			TDE2_API E_RESULT_CODE _createInternalHandlers(const TShaderCompilerResult& shaderBytecode) override;
		protected:
			ID3D11VertexShader*   mpVertexShader;
			ID3D11PixelShader*    mpPixelShader;
			ID3D11GeometryShader* mpGeometryShader;
	};


	/*!
		\brief A factory function for creation objects of CD3D11ShaderFactory's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11ShaderFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateD3D11ShaderFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CD3D11ShaderFactory

		\brief The class is an abstract factory of CD3D11Shader objects that
		is used by a resource manager
	*/

	class CD3D11ShaderFactory : public IShaderFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateD3D11ShaderFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11ShaderFactory)
		protected:
			bool mIsInitialized;
	};
}

#endif