#include "./../include/CD3D11VertexDeclaration.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Shader.h"
#include <core/IGraphicsContext.h>
#include <unordered_map>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11VertexDeclaration::CD3D11VertexDeclaration() :
		CVertexDeclaration()
	{
	}

	TResult<ID3D11InputLayout*> CD3D11VertexDeclaration::GetInputLayoutByShader(IGraphicsContext* pGraphicsContext, const IShader* pShader)
	{
		const CD3D11Shader* pD3D11Shader = dynamic_cast<const CD3D11Shader*>(pShader);

		if (!pGraphicsContext || !pD3D11Shader)
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		ID3D11Device* p3dDevice = nullptr;

		ID3D11InputLayout* pInputLayout = nullptr;

#if _HAS_CXX17
		p3dDevice = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData()).mp3dDevice;
#else
		p3dDevice = pGraphicsContext->GetInternalData().mD3D11.mp3dDevice;
#endif

		std::vector<D3D11_INPUT_ELEMENT_DESC> elements;

		D3D11_INPUT_ELEMENT_DESC currElement;

		std::unordered_map<E_VERTEX_ELEMENT_SEMANTIC_TYPE, U32> usedSemanticIndex;
		
		/// fill in elements vector
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter)
		{
			currElement.SemanticName         = CD3D11Mappings::GetSemanticTypeName((*iter).mSemanticType);
			currElement.SemanticIndex        = usedSemanticIndex[(*iter).mSemanticType]++;
			currElement.Format               = CD3D11Mappings::GetDXGIFormat((*iter).mFormatType);
			currElement.InputSlot            = (*iter).mSource;
			currElement.AlignedByteOffset    = 0;
			currElement.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
			currElement.InstanceDataStepRate = 0;

			elements.push_back(currElement);
		}

		TShaderBytecodeDesc vsBytecodeDesc = pD3D11Shader->GetVertexShaderBytecode();

		if (FAILED(p3dDevice->CreateInputLayout(&elements[0], elements.size(), vsBytecodeDesc.mpBytecode, vsBytecodeDesc.mLength, &pInputLayout)))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		return TOkValue<ID3D11InputLayout*>(pInputLayout);
	}


	IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result)
	{
		CD3D11VertexDeclaration* pVertexDeclInstance = new (std::nothrow) CD3D11VertexDeclaration();

		if (!pVertexDeclInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pVertexDeclInstance->Init();

		if (result != RC_OK)
		{
			delete pVertexDeclInstance;

			pVertexDeclInstance = nullptr;
		}

		return pVertexDeclInstance;
	}
}

#endif