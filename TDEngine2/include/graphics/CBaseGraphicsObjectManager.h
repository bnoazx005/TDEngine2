/*!
\file CBaseGraphicsObjectManager.h
\date 30.11.2018
\authors Kasimov Ildar
*/

#pragma once


#include "IGraphicsObjectManager.h"
#include "./../core/CBaseObject.h"
#include <vector>
#include <list>


namespace TDEngine2
{
	
	/*!
		class CBaseGraphicsObjectManager

		\brief The class is a common implementation for all graphics contexts
	*/

	class CBaseGraphicsObjectManager : public IGraphicsObjectManager, public CBaseObject
	{
		protected:
			typedef std::vector<IBuffer*>            TBuffersArray;

			typedef std::list<U32>                   TFreeEntitiesRegistry;

			typedef std::vector<IVertexDeclaration*> TVertexDeclarationsArray;
		public:
			/*!
				\brief The method initializes an initial state of a buffer

				\param[in] pGraphicsContext A pointer to implementation of IGraphicsContext interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method is a factory for creation of a debug helper object

				\return The result object contains either a pointer to IDebugUtility or an error code
			*/

			TDE2_API TResult<IDebugUtility*> CreateDebugUtility() override;

			/*!
				\brief The method returns a pointer to IGraphicsContext
				\return The method returns a pointer to IGraphicsContext
			*/

			TDE2_API IGraphicsContext* GetGraphicsContext() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseGraphicsObjectManager)

			TDE2_API void _insertBuffer(IBuffer* pBuffer);

			TDE2_API void _insertVertexDeclaration(IVertexDeclaration* pVertDecl);

			TDE2_API E_RESULT_CODE _freeBuffers();

			TDE2_API E_RESULT_CODE _freeVertexDeclarations();

			TDE2_API virtual E_RESULT_CODE _freeTextureSamplers() = 0;
		protected:
			IGraphicsContext*        mpGraphicsContext;

			TBuffersArray            mBuffersArray;

			TFreeEntitiesRegistry    mFreeBuffersSlots;

			TVertexDeclarationsArray mVertexDeclarationsArray;

			TFreeEntitiesRegistry    mFreeVertDeclsSlots;

			IDebugUtility*           mpDebugUtility;
	};
}