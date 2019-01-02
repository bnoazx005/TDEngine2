#include "./../include/COGLVertexDeclaration.h"
#include "./../include/COGLMappings.h"
#include "./../include/COGLVertexBuffer.h"
#include <graphics/IVertexBuffer.h>
#include <climits>


namespace TDEngine2
{
	COGLVertexDeclaration::COGLVertexDeclaration() :
		CVertexDeclaration()
	{
	}

	TResult<GLuint> COGLVertexDeclaration::GetVertexArrayObject(const std::vector<IVertexBuffer*>& pVertexBuffersArray)
	{
		if (!pVertexBuffersArray.empty())
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		GLuint vaoHandler = 0;

		glGenVertexArrays(1, &vaoHandler);

		glBindVertexArray(vaoHandler);

		for (auto iter = pVertexBuffersArray.cbegin(); iter != pVertexBuffersArray.cend(); ++iter)
		{
			glBindBuffer(GL_ARRAY_BUFFER, (*iter)->GetInternalData().mGLBuffer);
		}

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// generate vertex attribute pointers
		GLuint currIndex = 0;

		E_FORMAT_TYPE currFormat = FT_UNKNOWN;

		U32 currOffset = 0;
		
		U32 instanceDivisorIndex = (std::numeric_limits<U32>::max)();
		U32 instancesPerData     = 0;

		TInstancingInfoArray::const_iterator instancingInfoIter = mInstancingInfo.cbegin();

		if (!mInstancingInfo.empty())
		{
			std::tie(instanceDivisorIndex, instancesPerData) = *instancingInfoIter;
		}

		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{
			currFormat = (*iter).mFormatType;

			/// instancing is used
			if ((instanceDivisorIndex == currIndex) && (instancingInfoIter + 1 != mInstancingInfo.cend()))
			{
				glVertexAttribDivisor(instanceDivisorIndex, instancesPerData);

				std::tie(instanceDivisorIndex, instancesPerData) = *(++instancingInfoIter); /// retrieve next division's info

				currOffset = 0; /// reset the offset if a new division began
			}
			
			glVertexAttribPointer(currIndex, COGLMappings::GetNumOfChannelsOfFormat(currFormat),
								  COGLMappings::GetBaseTypeOfFormat(currFormat),
								  COGLMappings::IsFormatNormalized(currFormat),
								  COGLMappings::GetFormatSize(currFormat), reinterpret_cast<void*>(currOffset));

			currOffset += COGLMappings::GetFormatSize(currFormat);
			
			glEnableVertexAttribArray(currIndex);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		return TOkValue<GLuint>(vaoHandler);
	}

	void COGLVertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const std::vector<IVertexBuffer*>& pVertexBuffersArray, IShader* pShader)
	{
		GLuint vaoHandler = GetVertexArrayObject(pVertexBuffersArray).Get();
		
		//CD3D11VertexBuffer* pD3D11VertexBuffer = nullptr;

		//for (auto iter = pVertexBuffersArray.begin(); iter != pVertexBuffersArray.end(); ++iter)
		//{
		//	pD3D11VertexBuffer = dynamic_cast<CD3D11VertexBuffer*>(*iter);

		//	if (!pD3D11VertexBuffer)
		//	{
		//		return;
		//	}

		//	//pD3D11VertexBuffer->SetInputLayout
		//}

		//if (!pD3D11VertexBuffer)
		//{
		//	return;
		//}

		//pGLVertexBuffer->SetVAOHandler(vaoHandler);

		glBindVertexArray(vaoHandler);
	}


	IVertexDeclaration* CreateOGLVertexDeclaration(E_RESULT_CODE& result)
	{
		COGLVertexDeclaration* pVertexDeclInstance = new (std::nothrow) COGLVertexDeclaration();

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