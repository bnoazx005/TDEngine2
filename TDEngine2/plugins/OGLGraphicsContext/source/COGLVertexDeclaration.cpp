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
		if (pVertexBuffersArray.empty())
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		GLuint vaoHandler = 0;

		glGenVertexArrays(1, &vaoHandler);

		glBindVertexArray(vaoHandler);
		
		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// generate vertex attribute pointers
		GLuint currIndex = 0;

		E_FORMAT_TYPE currFormat = FT_UNKNOWN;
				
		U32 currInstanceDivisorIndex = (std::numeric_limits<U32>::max)();
		U32 nextInstanceDivisorIndex = currInstanceDivisorIndex;
		U32 currInstancesPerData     = 0;
		U32 nextInstancesPerData     = 0;
		U32 currOffset               = 0;
		U32 vertexStrideSize         = GetStrideSize(0);

		TInstancingInfoArray::const_iterator instancingInfoIter = mInstancingInfo.cbegin();

		/// extract the information about next position where the new instancing block begins at
		if (!mInstancingInfo.empty())
		{
			std::tie(nextInstanceDivisorIndex, nextInstancesPerData) = *instancingInfoIter;
		}

		glBindBuffer(GL_ARRAY_BUFFER, pVertexBuffersArray.front()->GetInternalData().mGLBuffer); /// bind the first VBO by default as a main vertex buffer
		
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{
			currFormat = (*iter).mFormatType;

			/// provide information about a per instance rate
			if (currIndex >= currInstanceDivisorIndex && currIndex <= nextInstanceDivisorIndex)
			{
				glVertexAttribDivisor(currIndex, currInstancesPerData);
			}
			else if (currIndex == nextInstanceDivisorIndex) /// reach the next instancing group
			{
				currOffset = 0; /// reset the current offset because of a new block begins

				/// bind the buffer that is related with the group
				glBindBuffer(GL_ARRAY_BUFFER, pVertexBuffersArray[(*iter).mSource]->GetInternalData().mGLBuffer);

				currInstanceDivisorIndex = nextInstanceDivisorIndex;
				currInstancesPerData     = nextInstancesPerData;

				glVertexAttribDivisor(currIndex, currInstancesPerData);

				vertexStrideSize = GetStrideSize((*iter).mSource);

				if (instancingInfoIter + 1 != mInstancingInfo.cend())
				{
					std::tie(nextInstanceDivisorIndex, nextInstancesPerData) = *(instancingInfoIter++);
				}
				else
				{
					nextInstanceDivisorIndex = (std::numeric_limits<U32>::max)();
				}
			}
						
			glVertexAttribPointer(currIndex, COGLMappings::GetNumOfChannelsOfFormat(currFormat),
								  COGLMappings::GetBaseTypeOfFormat(currFormat),
								  COGLMappings::IsFormatNormalized(currFormat),
								  vertexStrideSize, reinterpret_cast<void*>(currOffset));

			currOffset += COGLMappings::GetFormatSize(currFormat);
			
			glEnableVertexAttribArray(currIndex);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		return TOkValue<GLuint>(vaoHandler);
	}

	void COGLVertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const std::vector<IVertexBuffer*>& pVertexBuffersArray, IShader* pShader)
	{
		glBindVertexArray(GetVertexArrayObject(pVertexBuffersArray).Get());
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