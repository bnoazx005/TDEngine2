#pragma once


#include <TDEngine2.h>


namespace TDEngine2
{
	class CStubEventManager : public IEventManager, public CBaseObject
	{
		public:
			CStubEventManager() :
				CBaseObject()
			{
			}

			~CStubEventManager() = default;

			E_RESULT_CODE Init() override
			{
				mIsInitialized = true;
				return RC_OK;
			}

			E_RESULT_CODE Subscribe(TypeId eventType, IEventHandler* pEventListener) override
			{
				return RC_OK;
			}

			E_RESULT_CODE Unsubscribe(TypeId eventType, IEventHandler* pEventListener) override
			{
				return RC_OK;
			}

			E_RESULT_CODE NotifyImmediate(const TBaseEvent& event) override
			{
				for (auto&& currListener : mListeners)
				{
					if (currListener.first == event.GetEventType())
					{
						if (!currListener.second)
						{
							continue;
						}

						(currListener.second)();
					}
				}

				return RC_OK;
			}


			void AddSimpleListener(TypeId eventType, const std::function<void()>& callback)
			{
				mListeners.emplace(eventType, callback);
			}

			E_ENGINE_SUBSYSTEM_TYPE GetType() const { return GetTypeID(); }
			static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_EVENT_MANAGER; }
		protected:
			E_RESULT_CODE _registerEventQueue(TypeId eventTypeId, std::unique_ptr<IEventQueue> pEventQueue) override { return RC_OK; }
			IEventQueue* _getEventQueueByType(TypeId eventTypeId) override { return nullptr; }

			E_RESULT_CODE _enableBufferingForEventTypeImpl(TypeId eventTypeId) override { return RC_OK; }
			bool _isEventTypeSupportBuffering(TypeId eventTypeId) const { return false; }
		private:
			std::unordered_map<TypeId, std::function<void()>> mListeners;
	};
}