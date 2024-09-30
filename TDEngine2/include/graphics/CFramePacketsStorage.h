/*!
	\brief CFramePacketsStorage.h
	\date 29.09.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../graphics/InternalShaderData.h"
#include "../graphics/IRenderer.h"
#include <array>


namespace TDEngine2
{
	class CRenderQueue;
	class IAllocator;
	class CFramePacketsStorage;


	TDE2_DECLARE_SCOPED_PTR(CRenderQueue)


	/*!
		\brief The type contains all the data that's relevant to specific game frame. Later it's used to split game and render logic into two
		separate independent stages like described in https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
	*/

	typedef struct TFramePacket
	{
		typedef std::array<TPtr<CRenderQueue>, NumOfRenderQueuesGroup> TRenderQueuesArray;
		typedef std::array<TPtr<IAllocator>, NumOfRenderQueuesGroup>   TAllocatorsArray;

		U32                   mFrameIndex = 0;
		F32                   mDeltaTime = 0.0f; ///< Also it could be retrieved from mPerFrameData.mTime.y
		
		TPerFrameShaderData   mPerFrameData;
		TRareUpdateShaderData mRareUpdatedData;
		
		/// \note Per object data
		TRenderQueuesArray    mpRenderQueues { nullptr }; /// PerRenderQueueMemoryBlockSize per render queue
		TAllocatorsArray      mpRenderQueuesAllocators { nullptr};
	} TFramePacket, *TFramePacketPtr;


	TDE2_API CFramePacketsStorage* CreateFramePacketsStorage(TAllocatorFactoryFunctor allocatorFactoryFunctor, E_RESULT_CODE& result);


	class CFramePacketsStorage final: public CBaseObject
	{
		public:
			friend TDE2_API CFramePacketsStorage* CreateFramePacketsStorage(TAllocatorFactoryFunctor, E_RESULT_CODE&);
		public:
			TDE2_STATIC_CONSTEXPR U32 MAX_FRAME_PACKETS_COUNT = 16;

			typedef std::array<TFramePacket, 16> TFramePacketsArray;
		public:
			TDE2_API E_RESULT_CODE Init(TAllocatorFactoryFunctor allocatorFactoryFunctor);

			TDE2_API void IncrementGameLogicFrameCounter();
			TDE2_API void IncrementRenderFrameCounter();

			TDE2_API TFramePacket& GetCurrentFrameForGameLogic();
			TDE2_API TFramePacket& GetCurrentFrameForRender();

			TDE2_API U32 GetGameLogicFrameIndex() const;
			TDE2_API U32 GetRenderFrameIndex() const;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFramePacketsStorage)
		private:
			TFramePacketsArray mFramePackets;

			U32                mCurrGameLogicFrameIndex = 1;
			U32                mCurrRenderFrameIndex = 0;
	};
}