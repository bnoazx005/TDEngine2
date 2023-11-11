/*!
	\file CD3D11VertexDeclaration.h
	\date 11.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CVertexDeclaration.h>
#include <vector>


#if defined (TDE2_USE_WINPLATFORM)


#include <d3d11.h>


namespace TDEngine2
{
	class IGraphicsContext;
	class IShader;


	/*!
		\brief A factory function for creation objects of CD3D11VertexDeclaration's type

		\return A pointer to CD3D11VertexDeclaration's implementation
	*/

	TDE2_API IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result);


	/*!
		class CD3D11VertexDeclaration

		\brief The class implements a vertex declaration for D3D11
	*/

	class CD3D11VertexDeclaration : public CVertexDeclaration
	{
		public:
			friend TDE2_API IVertexDeclaration* CreateD3D11VertexDeclaration(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method creates a new input layout object  based on a given shader's description and returns it

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in] pShader A pointer to IShader implementation

				\return The method creates a new input layout object  based on a given shader's description and returns it
			*/

			TDE2_API TResult<ID3D11InputLayout*> GetInputLayoutByShader(IGraphicsContext* pGraphicsContext, const IShader* pShader);

			/*!
				\brief The method creates an internal handlers for a vertex declaration and binds it
				to a rendering pipeline

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pVertexBuffersArray An array of  IVertexBuffer implementations

				\param[in, out] pShader A pointer to IShader implementation
			*/

			TDE2_API void Bind(IGraphicsContext* pGraphicsContext, const CStaticArray<TBufferHandleId>& pVertexBuffersArray, IShader* pShader) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11VertexDeclaration)

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			ID3D11InputLayout* mpInputLayout;
	};
}

#endif