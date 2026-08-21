/*!
	\brief CFramePacketsStorage.h
	\date 29.09.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/CBaseObject.h"
#include "../graphics/InternalShaderData.h"
#include "../graphics/IRenderer.h"
#include "../graphics/effects/ParticleEmitters.h"
#include <array>
#include <mutex>
#include <condition_variable>


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
		typedef std::vector<TEmitterUniformsData>                      TGPUParticleEmittersArray;

		U32                       mFrameIndex = 0;
		F32                       mDeltaTime = 0.0f; ///< Also it could be retrieved from mPerFrameData.mTime.y
		
		TPerFrameShaderData       mPerFrameData;
		
		/// \note Per object data
		TRenderQueuesArray        mpRenderQueues { nullptr }; /// PerRenderQueueMemoryBlockSize per render queue

		TGPUParticleEmittersArray mGpuParticleEmitters;

		TLightsDataArray          mActiveLightSources;
	} TFramePacket, *TFramePacketPtr;


	TDE2_API CFramePacketsStorage* CreateFramePacketsStorage(TAllocatorFactoryFunctor allocatorFactoryFunctor, E_RESULT_CODE& result);


	class CFramePacketsStorage final: public CBaseObject
	{
		public:
			friend TDE2_API CFramePacketsStorage* CreateFramePacketsStorage(TAllocatorFactoryFunctor, E_RESULT_CODE&);
		public:
			TDE2_STATIC_CONSTEXPR U32 MAX_FRAME_PACKETS_COUNT = 1;
			static_assert((MAX_FRAME_PACKETS_COUNT & (MAX_FRAME_PACKETS_COUNT - 1)) == 0, "MAX_FRAME_PACKETS_COUNT should be power of 2");

			enum class E_PACKET_STATE : U8
			{
				EMPTY,
				WRITING,
				READY
			};

			typedef std::array<TFramePacket, MAX_FRAME_PACKETS_COUNT>                TFramePacketsArray;
			typedef std::array<std::atomic<E_PACKET_STATE>, MAX_FRAME_PACKETS_COUNT> TFramePacketsStatesArray;
		public:
			TDE2_API E_RESULT_CODE Init(TAllocatorFactoryFunctor allocatorFactoryFunctor);

			TDE2_API TFramePacket& AcquireGameLogicFramePacket();
			TDE2_API E_RESULT_CODE SubmitGameLogicFramePacket();

			TDE2_API TFramePacket& AcquireRenderLogicFramePacket();
			TDE2_API E_RESULT_CODE SubmitRenderLogicFramePacket();

			TDE2_API void IncrementGameLogicFrameCounter();
			TDE2_API void IncrementRenderFrameCounter();

			TDE2_API TFramePacket& GetCurrentFrameForGameLogic();
			TDE2_API TFramePacket& GetCurrentFrameForRender();

			TDE2_API U64 GetGameLogicFrameIndex() const;
			TDE2_API U64 GetRenderFrameIndex() const;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFramePacketsStorage)
		private:
			TFramePacketsArray       mFramePackets;
			TFramePacketsStatesArray mFramePacketsState;

			std::atomic<U64>         mCurrGameLogicFrameIndex { 0 };
			std::atomic<U64>         mCurrRenderFrameIndex{ 0 };

			mutable std::mutex       mMutex;
			std::condition_variable  mSignal;
	};
}