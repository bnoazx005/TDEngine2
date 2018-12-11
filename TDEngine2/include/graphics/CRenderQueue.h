/*!
	\file IRenderQueue.h
	\date 08.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/CBaseObject.h"
#include "./../core/IGraphicsContext.h"
#include "./../core/memory/IAllocator.h"
#include <vector>
#include <tuple>


namespace TDEngine2
{
	class IVertexBuffer;
	class IAllocator;
	class CRenderQueue;


	typedef struct TRenderCommand
	{
		virtual ~TRenderCommand() = default;
	} TRenderCommand, *TRenderCommandPtr;


	typedef struct TDrawCommand: TRenderCommand
	{
		U32                       mNumOfVertices;

		U32                       mStartVertex;

		E_PRIMITIVE_TOPOLOGY_TYPE mPrimitiveType;

		IVertexBuffer*            mpVertexBuffer;
	} TDrawCommand, *TDrawCommandPtr;


	typedef struct TDrawIndexedCommand: TRenderCommand
	{
		U32                       mNumOfIndices;

		U32                       mStartIndex;

		U32                       mStartVertex;

		E_PRIMITIVE_TOPOLOGY_TYPE mPrimitiveType;

		IVertexBuffer*            mpVertexBuffer;

		IIndexBuffer*             mpIndexBuffer;
	} TDrawIndexedCommand, *TDrawIndexedCommandPtr;


	typedef struct TDrawInstancedCommand: TRenderCommand
	{

	} TDrawInstancedCommand, *TDrawInstancedCommandPtr;


	typedef struct TDrawIndexedInstancedCommand: TRenderCommand
	{

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
		public:
			/*!
				\brief The method initializes an internal state of a queue

				\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
				for temporary allocations

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			
			TDE2_API E_RESULT_CODE Init(IAllocator* pTempAllocator);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates and pushes a new command to the queue

				\param groupKey A key of a created command

				\return A pointer to TRenderCommand object
			*/

			template <typename T>
			TDE2_API T* SubmitDrawCommand(U32 groupKey)
			{
				if (!mpTempAllocator)
				{
					return nullptr;
				}

				TRenderCommand* pRenderCommand = new (mpTempAllocator->Allocate(sizeof(T), __alignof(T))) T(); /// \todo Replace the allocation with a helper function's invokation

				mCommandsBuffer.emplace_back(groupKey, pRenderCommand);

				return dynamic_cast<T*>(pRenderCommand);
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CRenderQueue)
		protected:
			std::vector<std::tuple<U32, TRenderCommand*>> mCommandsBuffer;

			IAllocator*                  mpTempAllocator;
	};
}