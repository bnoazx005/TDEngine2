#include "../include/COGLVertexDeclaration.h"
#include "../include/COGLMappings.h"
#include "../include/COGLUtils.h"
#include <graphics/IGraphicsObjectManager.h>
#include <climits>


namespace TDEngine2
{
	COGLVertexDeclaration::COGLVertexDeclaration() :
		CVertexDeclaration()
	{
	}


	static TResult<GLuint> DoesHandleExist(IGraphicsContext* pGraphicsContext, const COGLVertexDeclaration::TVAORegistryNode& registry, const CStaticArray<TBufferHandleId>& pVertexBuffersArray)
	{
		auto pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		const COGLVertexDeclaration::TVAORegistryNode* pCurrNode = &registry;

		U32 internalBufferHandle = 0x0;

		for (U32 i = 0; i < pVertexBuffersArray.GetSize(); ++i)
		{
			internalBufferHandle = *reinterpret_cast<GLuint*>(pGraphicsObjectManager->GetBufferPtr(pVertexBuffersArray[i])->GetInternalData());

			if (pCurrNode->mChildren.find(internalBufferHandle) == pCurrNode->mChildren.cend())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
			}

			pCurrNode = pCurrNode->mChildren.at(internalBufferHandle).get();
		}

		return Wrench::TOkValue<GLuint>(pCurrNode->mVAOHandle);
	}


	TResult<GLuint> COGLVertexDeclaration::GetVertexArrayObject(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray)
	{
		if (pVertexBuffersArray.IsEmpty())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		GLuint vaoHandler = 0x0;

		if (pVertexBuffersArray.GetSize() == 1 && TBufferHandleId::Invalid == pVertexBuffersArray[0])
		{
			GL_SAFE_VOID_CALL(glGenVertexArrays(1, &vaoHandler));
			GL_SAFE_VOID_CALL(glBindVertexArray(vaoHandler));

			return Wrench::TOkValue<GLuint>(vaoHandler);
		}

		auto doesExistResult = DoesHandleExist(pGraphicsContext, mRootNode, pVertexBuffersArray);
		if (doesExistResult.IsOk())
		{
			return doesExistResult;
		}

		GL_SAFE_VOID_CALL(glGenVertexArrays(1, &vaoHandler));
		GL_SAFE_VOID_CALL(glBindVertexArray(vaoHandler));
		
		if (glGetError() != GL_NO_ERROR)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		/// generate vertex attribute pointers
		GLuint currIndex = 0;

		E_FORMAT_TYPE currFormat = FT_UNKNOWN;
				
		U32 currInstanceDivisorIndex = (std::numeric_limits<U32>::max)();
		U32 nextInstanceDivisorIndex = currInstanceDivisorIndex;
		U32 currInstancesPerData     = 0;
		U32 nextInstancesPerData     = 0;
		USIZE currOffset             = 0;
		U32 vertexStrideSize         = GetStrideSize(0);

		TInstancingInfoArray::const_iterator instancingInfoIter = mInstancingInfo.cbegin();

		/// extract the information about next position where the new instancing block begins at
		if (!mInstancingInfo.empty())
		{
			std::tie(nextInstanceDivisorIndex, nextInstancesPerData) = *instancingInfoIter;
		}

		auto pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		GLuint currBufferHandle = *reinterpret_cast<GLuint*>(pGraphicsObjectManager->GetBufferPtr(pVertexBuffersArray[0])->GetInternalData());

		GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, currBufferHandle)); /// bind the first VBO by default as a main vertex buffer

		TVAORegistryNode* pCurrNode = _insertNewNode(&mRootNode, currBufferHandle);
		
		for (auto iter = mElements.cbegin(); iter != mElements.cend(); ++iter, ++currIndex)
		{
			currFormat = (*iter).mFormatType;

			/// provide information about a per instance rate
			if (currIndex >= currInstanceDivisorIndex && currIndex <= nextInstanceDivisorIndex)
			{
				GL_SAFE_VOID_CALL(glVertexAttribDivisor(currIndex, currInstancesPerData));
			}
			else if (currIndex == nextInstanceDivisorIndex) /// reach the next instancing group
			{
				currOffset = 0; /// reset the current offset because of a new block begins

				/// bind the buffer that is related with the group
				currBufferHandle = *reinterpret_cast<GLuint*>(pGraphicsObjectManager->GetBufferPtr(pVertexBuffersArray[(*iter).mSource])->GetInternalData());
				GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, currBufferHandle));

				pCurrNode = _insertNewNode(pCurrNode, currBufferHandle);

				currInstanceDivisorIndex = nextInstanceDivisorIndex;
				currInstancesPerData     = nextInstancesPerData;

				GL_SAFE_VOID_CALL(glVertexAttribDivisor(currIndex, currInstancesPerData));

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
						
			GL_SAFE_VOID_CALL(glVertexAttribPointer(currIndex, COGLMappings::GetNumOfChannelsOfFormat(currFormat),
													COGLMappings::GetBaseTypeOfFormat(currFormat),
													COGLMappings::IsFormatNormalized(currFormat),
													vertexStrideSize, reinterpret_cast<void*>(currOffset)));

			currOffset += COGLMappings::GetFormatSize(currFormat);
			
			GL_SAFE_VOID_CALL(glEnableVertexAttribArray(currIndex));
		}

		pCurrNode->mVAOHandle = vaoHandler;

		GL_SAFE_VOID_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_SAFE_VOID_CALL(glBindVertexArray(0));

		return Wrench::TOkValue<GLuint>(vaoHandler);
	}

	void COGLVertexDeclaration::Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader)
	{
		GL_SAFE_VOID_CALL(glBindVertexArray(GetVertexArrayObject(pGraphicsContext, pVertexBuffersArray).Get()));
	}

	COGLVertexDeclaration::TVAORegistryNode* COGLVertexDeclaration::_insertNewNode(TVAORegistryNode* pCurrNode, U32 handle)
	{
		if (pCurrNode->mChildren.find(handle) != pCurrNode->mChildren.cend())
		{
			return pCurrNode->mChildren[handle].get();
		}

		pCurrNode->mChildren[handle] = std::make_unique<TVAORegistryNode>();

		return pCurrNode->mChildren[handle].get();
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