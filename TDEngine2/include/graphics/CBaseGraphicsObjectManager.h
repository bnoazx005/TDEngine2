/*!
\file CBaseGraphicsObjectManager.h
\date 30.11.2018
\authors Kasimov Ildar
*/

#pragma once


#include "IGraphicsObjectManager.h"
#include "./../core/CBaseObject.h"
#include "./../utils/CResourceContainer.h"
#include <vector>
#include <list>
#include <unordered_map>


namespace TDEngine2
{
	
	/*!
		class CBaseGraphicsObjectManager

		\brief The class is a common implementation for all graphics contexts
	*/

	class CBaseGraphicsObjectManager : public IGraphicsObjectManager, public CBaseObject
	{
		protected:
			typedef std::vector<IBuffer*>               TBuffersArray;

			typedef std::list<U32>                      TFreeEntitiesRegistry;

			typedef std::vector<IVertexDeclaration*>    TVertexDeclarationsArray;

			typedef std::unordered_map<U32, U32>        TStatesHashTable;
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

				\param[in, out] pResourceManager A pointer to implementation of IResourceManager interface
				\param[in, out] pRenderer A pointer to implementation of IRenderer interface

				\return The result object contains either a pointer to IDebugUtility or an error code
			*/

			TDE2_API TResult<IDebugUtility*> CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer) override;

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

			TDE2_API virtual E_RESULT_CODE _freeBlendStates() = 0;

			TDE2_API virtual E_RESULT_CODE _freeDepthStencilStates() = 0;
		protected:
			IGraphicsContext*        mpGraphicsContext;

			TBuffersArray            mBuffersArray;

			TFreeEntitiesRegistry    mFreeBuffersSlots;

			TVertexDeclarationsArray mVertexDeclarationsArray;

			TFreeEntitiesRegistry    mFreeVertDeclsSlots;

			TStatesHashTable         mTextureSamplesHashTable;
			TStatesHashTable         mBlendStatesHashTable;

			IDebugUtility*           mpDebugUtility;
	};
}