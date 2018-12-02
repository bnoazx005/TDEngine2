#include "./../include/CD3D11VertexDeclaration.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Shader.h"
#include "./../include/CD3D11VertexBuffer.h"
#include <core/IGraphicsContext.h>
#include <graphics/IShader.h>
#include <graphics/IVertexBuffer.h>
#include <unordered_map>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11VertexDeclaration::CD3D11VertexDeclaration() :
		CVertexDeclaration(), mpInputLayout(nullptr)
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
		
		U32 currOffset = 0;

		/// fill in elements vector
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter)
		{
			currElement.SemanticName         = CD3D11Mappings::GetSemanticTypeName((*iter).mSemanticType);
			currElement.SemanticIndex        = usedSemanticIndex[(*iter).mSemanticType]++;
			currElement.Format               = CD3D11Mappings::GetDXGIFormat((*iter).mFormatType);
			currElement.InputSlot            = (*iter).mSource;
			currElement.AlignedByteOffset    = currOffset;
			currElement.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
			currElement.InstanceDataStepRate = 0;

			elements.push_back(currElement);

			currOffset += CD3D11Mappings::GetFormatSize((*iter).mFormatType);
		}

		TShaderBytecodeDesc vsBytecodeDesc = pD3D11Shader->GetVertexShaderBytecode();

		if (FAILED(p3dDevice->CreateInputLayout(&elements[0], elements.size(), vsBytecodeDesc.mpBytecode, vsBytecodeDesc.mLength, &pInputLayout)))
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		return TOkValue<ID3D11InputLayout*>(pInputLayout);
	}

	void CD3D11VertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, IVertexBuffer* pVertexBuffer, IShader* pShader)
	{
		if (!mpInputLayout)
		{
			mpInputLayout = GetInputLayoutByShader(pGraphicsContext, pShader).Get();
		}

		CD3D11VertexBuffer* pD3D11VertexBuffer = dynamic_cast<CD3D11VertexBuffer*>(pVertexBuffer);

		if (!pD3D11VertexBuffer)
		{
			return;
		}

		pD3D11VertexBuffer->SetInputLayout(mpInputLayout, GetStrideSize());
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