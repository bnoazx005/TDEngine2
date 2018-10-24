/*!
	\file CD3D11Shader.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShader.h>
#include <utils/Config.h>


#if defined (TDE2_USE_WIN32PLATFORM)

#include <d3d11.h>


namespace TDEngine2
{
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
}

#endif