#include "./../include/CD3D11VertexDeclaration.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Shader.h"
#include "./../include/CD3D11VertexBuffer.h"
#include "./../include/CD3D11Utils.h"
#include <core/IGraphicsContext.h>
#include <graphics/IShader.h>
#include <graphics/IVertexBuffer.h>
#include <unordered_map>
#include <climits>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CD3D11VertexDeclaration::CD3D11VertexDeclaration() :
		CVertexDeclaration(), mpInputLayout(nullptr)
	{
	}

	E_RESULT_CODE CD3D11VertexDeclaration::_onFreeInternal()
	{
		return SafeReleaseCOMPtr<ID3D11InputLayout>(&mpInputLayout) | CVertexDeclaration::_onFreeInternal();
	}

	TResult<ID3D11InputLayout*> CD3D11VertexDeclaration::GetInputLayoutByShader(IGraphicsContext* pGraphicsContext, const IShader* pShader)
	{
		const CD3D11Shader* pD3D11Shader = dynamic_cast<const CD3D11Shader*>(pShader);

		if (!pGraphicsContext || !pD3D11Shader)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
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
		
		U32 currInstancingElementIndex = (std::numeric_limits<U32>::max)();
		U32 currInstancesPerData       = 0;

		TInstancingInfoArray::const_iterator instancingIter = mInstancingInfo.cbegin();

		if (!mInstancingInfo.empty())
		{
			std::tie(currInstancingElementIndex, currInstancesPerData) = *instancingIter;
		}
		
		U32 currIndex = 0;

		/// fill in elements vector
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{
			currElement.InstanceDataStepRate = (*iter).mIsPerInstanceData ? currElement.InstanceDataStepRate : 0;

			/// a new instancing division has found
			if ((currIndex == currInstancingElementIndex) /*&& (instancingIter + 1 != mInstancingInfo.cend())*/)
			{
				currOffset = 0;

				currElement.InstanceDataStepRate = currInstancesPerData;

				if (instancingIter + 1 == mInstancingInfo.cend())
				{
					currInstancingElementIndex = (std::numeric_limits<U32>::max)();
				}
				else
				{
					std::tie(currInstancingElementIndex, currInstancesPerData) = *(++instancingIter); /// retrieve next division's info
				}
			}

			currElement.SemanticName         = CD3D11Mappings::GetSemanticTypeName((*iter).mSemanticType);
			currElement.SemanticIndex        = usedSemanticIndex[(*iter).mSemanticType]++;
			currElement.Format               = CD3D11Mappings::GetDXGIFormat((*iter).mFormatType);
			currElement.InputSlot            = (*iter).mSource;
			currElement.AlignedByteOffset    = currOffset;
			currElement.InputSlotClass       = (*iter).mIsPerInstanceData ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;

			elements.push_back(currElement);

			currOffset += CD3D11Mappings::GetFormatSize((*iter).mFormatType);
		}

		auto&& vsBytecodeDesc = pD3D11Shader->GetVertexShaderBytecode();

		if (FAILED(p3dDevice->CreateInputLayout(&elements[0], static_cast<UINT>(elements.size()), vsBytecodeDesc.data(), vsBytecodeDesc.size(), &pInputLayout)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11InputLayout*>(pInputLayout);
	}

	void CD3D11VertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<IVertexBuffer*>& pVertexBuffersArray, IShader* pShader)
	{
		if (!mpInputLayout)
		{
			mpInputLayout = GetInputLayoutByShader(pGraphicsContext, pShader).Get();
		}

		ID3D11DeviceContext* p3dDeviceContext = nullptr;

#if _HAS_CXX17
		p3dDeviceContext = std::get<TD3D11CtxInternalData>(pGraphicsContext->GetInternalData()).mp3dDeviceContext;
#else
		p3dDeviceContext = pGraphicsContext->GetInternalData().mD3D11.mp3dDeviceContext;
#endif

		p3dDeviceContext->IASetInputLayout(mpInputLayout);
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