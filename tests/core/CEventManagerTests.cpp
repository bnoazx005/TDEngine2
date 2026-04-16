#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


struct TTestEvent : TBaseEvent
{
	TDE2_REGISTER_TYPE(TTestEvent)
	REGISTER_EVENT_TYPE(TTestEvent)
};


TEST_CASE("CEventManager Tests")
{
	E_RESULT_CODE result = RC_OK;
	
	TPtr<IEventManager> pEventManager = TPtr<IEventManager>(CreateEventManager(result));
	REQUIRE((pEventManager && RC_OK == result));

	SECTION("TestEnableBufferingForEventType_PassCorrectEventType_ReturnsRC_OK")
	{
		REQUIRE(RC_OK == pEventManager->EnableBufferingForEventType<TTestEvent>());
	}

	SECTION("TestNotifyImmediate_SubscribeToEventAndTryToInvokeOne_ListenerReceivesEvent")
	{
		class CTestEventHandler : public IEventHandler
		{
			public:
				TDE2_REGISTER_TYPE(CTestEventHandler)

				E_RESULT_CODE OnEvent(const TBaseEvent& event) override
				{
					mIsHandled = true;
					return RC_OK;
				}

				TEventListenerId GetListenerId() const override 
				{ 
					return static_cast<TEventListenerId>(CTestEventHandler::GetTypeId());
				}
			private:
				bool mIsHandled = false;
		};

		std::unique_ptr<CTestEventHandler> pEventListener = std::make_unique<CTestEventHandler>();

		REQUIRE(RC_OK == pEventManager->Subscribe(TTestEvent::GetTypeId(), pEventListener.get()));



		REQUIRE(RC_OK == pEventManager->Unsubscribe(TTestEvent::GetTypeId(), pEventListener.get()));
	}
}