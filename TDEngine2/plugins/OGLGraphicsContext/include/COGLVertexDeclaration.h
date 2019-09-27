/*!
	\file COGLVertexDeclaration.h
	\date 27.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CVertexDeclaration.h>
#include <vector>
#include <GL/glew.h>
#include <utils/CResult.h>
#include <unordered_map>
#include <memory>


namespace TDEngine2
{
	class IVertexBuffer;


	/*!
		\brief A factory function for creation objects of COGLVertexDeclaration's type

		\return A pointer to COGLVertexDeclaration's implementation
	*/

	TDE2_API IVertexDeclaration* CreateOGLVertexDeclaration(E_RESULT_CODE& result);


	/*!
		class COGLVertexDeclaration

		\brief The class implements a vertex declaration for OGL
	*/

	class COGLVertexDeclaration : public CVertexDeclaration
	{
		public:
			friend TDE2_API IVertexDeclaration* CreateOGLVertexDeclaration(E_RESULT_CODE& result);
		public:
			typedef struct TVAORegistryNode
			{
				GLuint                                                     mVAOHandle;

				std::unordered_map<U32, std::unique_ptr<TVAORegistryNode>> mChildren;
			} TVAORegistryNode, *TVAORegistryNodePtr;
		public:
			/*!
				\brief The method creates a VAO for specified vertex declaration and binds it
				to a given vertex buffer

				\param[in] pVertexBuffersArray An array of  IVertexBuffer implementations

				\return An object that contains either a handler of created VAO or an error code
			*/

			TDE2_API TResult<GLuint> GetVertexArrayObject(const CStaticArray<IVertexBuffer*>& pVertexBuffersArray);

			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffersArray An array of  IVertexBuffer implementations

				\param[in, out] pShader A pointer to IShader implementation
			*/

			TDE2_API void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<IVertexBuffer*>& pVertexBuffersArray, IShader* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLVertexDeclaration)

			TDE2_API TResult<GLuint> _doesHandleExist(const TVAORegistryNode& registry, const CStaticArray<IVertexBuffer*>& pVertexBuffersArray) const;

			TDE2_API TVAORegistryNode* _insertNewNode(TVAORegistryNode* pCurrNode, U32 handle);
		protected:
			TVAORegistryNode mRootNode;
	};
}