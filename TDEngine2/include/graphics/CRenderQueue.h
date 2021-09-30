/*!
	\file IRenderQueue.h
	\date 08.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/CBaseObject.h"
#include "../core/IGraphicsContext.h"
#include "../core/memory/IAllocator.h"
#include "../graphics/IMaterial.h"
#include "InternalShaderData.h"
#include <vector>
#include <tuple>
#include <string>
#include <type_traits>


namespace TDEngine2
{
	class IVertexBuffer;
	class IAllocator;
	class CRenderQueue;
	class IRenderer;
	class IVertexDeclaration;
	class IResourceManager;
	class IGlobalShaderProperties;
	class IResourceHandler;


	typedef struct TRenderCommand
	{
		TDE2_API virtual ~TRenderCommand() = default;

		/*!
			\brief The method submits a command to a rendering pipeline

			\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

			\param[in, out] pResourceManager A pointer to IResourceManager implementation

			\param[in, out] pGlobalShaderProperties A pointer to IGlobalShaderProperties implementation

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API virtual E_RESULT_CODE Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties) = 0;

		E_PRIMITIVE_TOPOLOGY_TYPE mPrimitiveType;

		IVertexBuffer*            mpVertexBuffer;

		IVertexDeclaration*       mpVertexDeclaration;

		TResourceId               mMaterialHandle;

		TMaterialInstanceId       mMaterialInstanceId = DefaultMaterialInstanceId;

		TPerObjectShaderData      mObjectData;
	} TRenderCommand, *TRenderCommandPtr;


	typedef struct TDrawCommand: TRenderCommand
	{
		/*!
			\brief The method submits a command to a rendering pipeline

			\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

			\param[in, out] pResourceManager A pointer to IResourceManager implementation

			\param[in, out] pGlobalShaderProperties A pointer to IGlobalShaderProperties implementation

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties) override;

		U32 mNumOfVertices;

		U32 mStartVertex;
	} TDrawCommand, *TDrawCommandPtr;


	typedef struct TDrawIndexedCommand: TRenderCommand
	{
		/*!
			\brief The method submits a command to a rendering pipeline

			\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

			\param[in, out] pResourceManager A pointer to IResourceManager implementation

			\param[in, out] pGlobalShaderProperties A pointer to IGlobalShaderProperties implementation

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties) override;

		U32           mNumOfIndices;

		U32           mStartIndex;

		U32           mStartVertex;

		IIndexBuffer* mpIndexBuffer;
	} TDrawIndexedCommand, *TDrawIndexedCommandPtr;


	typedef struct TDrawInstancedCommand: TRenderCommand
	{
		/*!
			\brief The method submits a command to a rendering pipeline

			\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

			\param[in, out] pResourceManager A pointer to IResourceManager implementation

			\param[in, out] pGlobalShaderProperties A pointer to IGlobalShaderProperties implementation

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties) override;

		U32            mStartVertex;

		U32            mVerticesPerInstance;

		U32            mStartInstance;

		U32            mNumOfInstances;

		IVertexBuffer* mpInstancingBuffer;		
	} TDrawInstancedCommand, *TDrawInstancedCommandPtr;


	typedef struct TDrawIndexedInstancedCommand: TRenderCommand
	{
		/*!
			\brief The method submits a command to a rendering pipeline

			\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

			\param[in, out] pResourceManager A pointer to IResourceManager implementation

			\param[in, out] pGlobalShaderProperties A pointer to IGlobalShaderProperties implementation

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Submit(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IGlobalShaderProperties* pGlobalShaderProperties) override;

		U32            mBaseVertexIndex;

		U32            mStartIndex;

		U32            mStartInstance;

		U32            mIndicesPerInstance;

		U32            mNumOfInstances;

		IIndexBuffer*  mpIndexBuffer;

		IVertexBuffer* mpInstancingBuffer;
	} TDrawIndexedInstancedCommand, *TDrawIndexedInstancedCommandPtr;


	/*!
		\brief A factory function for creation objects of CRenderQueue's type

		\param[in, out] pTempAllocator  A pointer to IAllocator object which will be used for temporary allocations

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CRenderQueue's implementation
	*/

	TDE2_API CRenderQueue* CreateRenderQueue(IAllocator* pTempAllocator, E_RESULT_CODE& result);


	/*!
		interface CRenderQueue

		\brief The interface describes a functionality of
		a rendering queue, which accumulates commands and
		later sends it to a renderer
	*/

	class CRenderQueue: public CBaseObject
	{
		public:
			friend TDE2_API CRenderQueue* CreateRenderQueue(IAllocator* pTempAllocator, E_RESULT_CODE& result);
		protected:
			typedef std::vector<std::tuple<U32, TRenderCommand*>> TCommandsArray;
		public:
			/*!
				class CRenderQueueIterator

				\brief The class implements a one-way iterator over draw commands that are stored
				within a render queue
			*/

			class CRenderQueueIterator
			{
				public:
					TDE2_API CRenderQueueIterator(TCommandsArray& commandsBuffer, U32 initialIndex = 0);
					TDE2_API CRenderQueueIterator(const CRenderQueueIterator& iter);
					TDE2_API CRenderQueueIterator(CRenderQueueIterator&& iter);
					TDE2_API ~CRenderQueueIterator() = default;

					TDE2_API TRenderCommand* GetNext();

					TDE2_API bool HasNext() const;

					TDE2_API void Reset();

					TDE2_API TRenderCommand* Get() const;
					TDE2_API U32 GetIndex() const;

					TDE2_API CRenderQueueIterator& operator++();

					TDE2_API CRenderQueueIterator operator++(int);
					
					TDE2_API TRenderCommand* operator*() const;
				protected:
					TDE2_API CRenderQueueIterator() = default;
				protected:
					TCommandsArray* mpTargetCollection;

					U32             mCurrCommandIndex;
			};
		public:
			/*!
				\brief The method initializes an internal state of a queue

				\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
				for temporary allocations

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			
			TDE2_API E_RESULT_CODE Init(IAllocator* pTempAllocator);

			/*!
				\brief The method creates and pushes a new command to the queue

				\param groupKey A key of a created command

				\return A pointer to TRenderCommand object
			*/

			template <typename T>
			TDE2_API T* SubmitDrawCommand(U32 groupKey)
			{
				static_assert(std::is_base_of<TRenderCommand, T>::value, "Invalid template argument's type. \"T\" should derive TRenderCommand type");

				if (!mpTempAllocator)
				{
					return nullptr;
				}

				void* pMemoryBlock = mpTempAllocator->Allocate(sizeof(T), __alignof(T));
				TDE2_ASSERT(pMemoryBlock);

				T* pRenderCommand = new (pMemoryBlock) T(); /// \todo Replace the allocation with a helper function's invokation

				mCommandsBuffer.emplace_back(groupKey, pRenderCommand);

				return pRenderCommand;
			}

			/*!
				\brief The method clears up the existing list of commands

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Clear();

			/*!
				\brief The method sorts existing commands in buffer based on their group keys
			*/

			TDE2_API void Sort();

			/*!
				\brief The method returns true if a command buffer does not contain any draw command

				\return The method returns true if a command buffer does not contain any draw command
			*/

			TDE2_API bool IsEmpty() const;

			/*!
				\brief The method creates a new iterator that points to the beginning of the commands buffer and returns it

				\return The method creates a new iterator that points to the beginning of the commands buffer and returns it
			*/

			TDE2_API CRenderQueueIterator GetIterator();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CRenderQueue)
		protected:
			TCommandsArray mCommandsBuffer;

			IAllocator*    mpTempAllocator;
	};
}