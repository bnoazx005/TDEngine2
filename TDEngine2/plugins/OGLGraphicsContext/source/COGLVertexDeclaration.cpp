#include "./../include/COGLVertexDeclaration.h"
#include "./../include/COGLMappings.h"
#include <graphics/IVertexBuffer.h>


namespace TDEngine2
{
	COGLVertexDeclaration::COGLVertexDeclaration() :
		CVertexDeclaration()
	{
	}

	TResult<GLuint> COGLVertexDeclaration::GetVertexArrayObject(const IVertexBuffer* pVertexBuffer)
	{
		if (!pVertexBuffer)
		{
			return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		GLuint vaoHandler = 0;

		glGenVertexArrays(1, &vaoHandler);

		glBindVertexArray(vaoHandler);

		glBindBuffer(GL_VERTEX_ARRAY, pVertexBuffer->GetInternalData().mGLBuffer);

		if (glGetError() != GL_NO_ERROR)
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// generate vertex attribute pointers
		GLuint currIndex = 0;

		E_FORMAT_TYPE currFormat = FT_UNKNOWN;

		U32 currOffset = 0;

		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter)
		{
			currFormat = (*iter).mFormatType;
			
			glVertexAttribPointer(currIndex, COGLMappings::GetNumOfChannelsOfFormat(currFormat),
								  COGLMappings::GetBaseTypeOfFormat(currFormat),
								  COGLMappings::IsFormatNormalized(currFormat),
								  COGLMappings::GetFormatSize(currFormat), reinterpret_cast<void*>(currOffset));

			currOffset += COGLMappings::GetFormatSize(currFormat);

			glEnableVertexAttribArray(currIndex++);
		}

		glBindBuffer(GL_VERTEX_ARRAY, 0);

		glBindVertexArray(0);

		return TOkValue<GLuint>(vaoHandler);
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