#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <unordered_set>


using namespace TDEngine2;


namespace
{
	class CStubEventManager : public IEventManager, public CBaseObject
	{
		public:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStubEventManager)

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

			E_RESULT_CODE Notify(const TBaseEvent* pEvent) override
			{
				for (auto&& currListener : mListeners)
				{
					if (currListener.first == pEvent->GetEventType())
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
		private:
			std::unordered_map<TypeId, std::function<void()>> mListeners;
	};

	CStubEventManager::CStubEventManager() :
		CBaseObject()
	{
	}


	class CStubJobManager : public IJobManager, public CBaseObject
	{
	public:
		DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStubJobManager)

		E_RESULT_CODE Init(const TJobManagerInitParams& desc) override
		{
			mIsInitialized = true;
			return RC_OK;
		}

		E_RESULT_CODE SubmitJob(TJobCounter* pCounter, const TJobCallback& job, const C8* jobName = "TDE2Job") override { return RC_OK; }

		E_RESULT_CODE SubmitMultipleJobs(TJobCounter* pCounter, U32 jobsCount, U32 groupSize, const TJobCallback& job) override { return RC_OK; }

		void WaitForJobCounter(TJobCounter& counter, U32 counterThreshold = 0, TJobDecl* pAwaitingJob = nullptr) override { }

		E_RESULT_CODE ExecuteInMainThread(const std::function<void()>& action = nullptr) override { return RC_OK; }
				
		void ProcessMainThreadQueue() override { }

		E_ENGINE_SUBSYSTEM_TYPE GetType() const override { return E_ENGINE_SUBSYSTEM_TYPE::EST_JOB_MANAGER; }
	};

	CStubJobManager::CStubJobManager() :
		CBaseObject()
	{
	}


	TDE2_DECLARE_FLAG_COMPONENT(TestComponent);
	TDE2_DEFINE_FLAG_COMPONENT(TestComponent);
}


TEST_CASE("CWorld Tests")
{
	E_RESULT_CODE result = RC_OK;

	TPtr<IEventManager> pEventManager = TPtr<IEventManager>(new CStubEventManager());
	TPtr<IJobManager> pJobManager = TPtr<IJobManager>(new CStubJobManager());
	REQUIRE((pEventManager && pJobManager));

	TPtr<IWorld> pWorld = TPtr<IWorld>(CreateWorld(pEventManager, pJobManager, result));
	REQUIRE(pWorld);

	REQUIRE((RC_OK == pWorld->RegisterComponentFactory(TPtr<IComponentFactory>(CreateTestComponentFactory(result)))));

	auto pStubEventManager = DynamicPtrCast<CStubEventManager>(pEventManager);

	SECTION("TestSetEntityActive_PassSingleEntityAndDeactivateIt_DeactivatesThatReturnsRC_OK")
	{
		auto pEntity = pWorld->CreateEntity();
		REQUIRE((!pEntity->HasComponent<CDeactivatedComponent>() && !pEntity->HasComponent<CDeactivatedGroupComponent>()));

		REQUIRE(RC_OK == SetEntityActive(pWorld.Get(), pEntity->GetId(), false));
		REQUIRE(pEntity->HasComponent<CDeactivatedComponent>());
	}

	SECTION("TestSetEntityActive_DeactivateParentOfSimpleEntitiesHierarchy_DeactivatesAllEntitiesAndChildHasDeactivatedGroupComponent")
	{
		/*
			Parent1 (deactivate this)
				Child1
		*/

		auto pParentEntity = pWorld->CreateEntity("Parent");
		auto pChildEntity = pWorld->CreateEntity("Child");

		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParentEntity->GetId(), pChildEntity->GetId()));

		auto entities =
		{
			pParentEntity, pChildEntity
		};

		for (auto pCurrEntity : entities) /// At the beginning all entities are active
		{
			REQUIRE((!pCurrEntity->HasComponent<CDeactivatedComponent>() && !pCurrEntity->HasComponent<CDeactivatedGroupComponent>()));
		}

		REQUIRE(RC_OK == SetEntityActive(pWorld.Get(), pParentEntity->GetId(), false));

		/// The parent should have Deactivated component, child has only DeactivatedGroup one
		REQUIRE(pParentEntity->HasComponent<CDeactivatedComponent>());
		REQUIRE(!pParentEntity->HasComponent<CDeactivatedGroupComponent>());

		REQUIRE(!pChildEntity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChildEntity->HasComponent<CDeactivatedGroupComponent>());
	}

	SECTION("TestSetEntityActive_DeactivateParentWithChildAlreadyDeactivated_BothWillHaveDeactivatedAndDeactivatedGroupComponents")
	{
		/*
			 Parent1 (deactivate this)
			x	Child1  
		*/

		auto pParentEntity = pWorld->CreateEntity("Parent");
		auto pChildEntity = pWorld->CreateEntity("Child");
		pChildEntity->AddComponent<CDeactivatedComponent>();

		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParentEntity->GetId(), pChildEntity->GetId()));

		REQUIRE((!pParentEntity->HasComponent<CDeactivatedComponent>() && !pParentEntity->HasComponent<CDeactivatedGroupComponent>()));

		REQUIRE(RC_OK == SetEntityActive(pWorld.Get(), pParentEntity->GetId(), false));

		/// The parent should have Deactivated component, child has only DeactivatedGroup one
		REQUIRE(pParentEntity->HasComponent<CDeactivatedComponent>());
		REQUIRE(!pParentEntity->HasComponent<CDeactivatedGroupComponent>());

		REQUIRE(pChildEntity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChildEntity->HasComponent<CDeactivatedGroupComponent>());
	}

	SECTION("TestSetEntityActive_HierarchyWithAllDeactivatedEntitiesChildIsActivated_DeactivatedGroupStayOnAllEntities")
	{
		/*
			x Parent1
			x	Child1  (activate this)
			x		Child2
		*/

		auto pParentEntity = pWorld->CreateEntity("Parent");
		auto pChild1Entity = pWorld->CreateEntity("Child1");
		auto pChild2Entity = pWorld->CreateEntity("Child2");
		
		auto entities =
		{
			pParentEntity, pChild1Entity, pChild2Entity
		};

		for (auto pCurrEntity : entities)
		{
			pCurrEntity->AddComponent<CDeactivatedComponent>();
			pCurrEntity->AddComponent<CDeactivatedGroupComponent>();
		}

		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParentEntity->GetId(), pChild1Entity->GetId()));
		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pChild1Entity->GetId(), pChild2Entity->GetId()));

		REQUIRE(RC_OK == SetEntityActive(pWorld.Get(), pChild1Entity->GetId(), true));

		REQUIRE(pParentEntity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pParentEntity->HasComponent<CDeactivatedGroupComponent>());

		REQUIRE(!pChild1Entity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChild1Entity->HasComponent<CDeactivatedGroupComponent>());

		REQUIRE(pChild2Entity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChild2Entity->HasComponent<CDeactivatedGroupComponent>());
	}

	SECTION("TestSetEntityActive_HierarchyWithAllDeactivatedEntitiesParentIsActivated_DeactivatedGroupRemovedFromEntities")
	{
		/*
			x Parent1 (activate this)
			x	Child1
			x		Child2
		*/

		auto pParentEntity = pWorld->CreateEntity("Parent");
		auto pChild1Entity = pWorld->CreateEntity("Child1");
		auto pChild2Entity = pWorld->CreateEntity("Child2");

		auto entities =
		{
			pParentEntity, pChild1Entity, pChild2Entity
		};

		for (auto pCurrEntity : entities)
		{
			pCurrEntity->AddComponent<CDeactivatedComponent>();
			pCurrEntity->AddComponent<CDeactivatedGroupComponent>();
		}

		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParentEntity->GetId(), pChild1Entity->GetId()));
		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pChild1Entity->GetId(), pChild2Entity->GetId()));

		REQUIRE(RC_OK == SetEntityActive(pWorld.Get(), pParentEntity->GetId(), true));

		REQUIRE(!pParentEntity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChild1Entity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChild2Entity->HasComponent<CDeactivatedComponent>());

		for (auto pCurrEntity : entities)
		{
			REQUIRE(!pCurrEntity->HasComponent<CDeactivatedGroupComponent>());
		}
	}

	SECTION("TestSetEntityActive_HierarchyWithAllDeactivatedEntitiesParent2IsActivated_DeactivatedComponentRemovedOnlyFromParent2")
	{
		/*
			x Parent1
			x	Parent2 (activate this)
			x		Child1
			x			Child2
		*/

		auto pParent1Entity = pWorld->CreateEntity("Parent1");
		auto pParent2Entity = pWorld->CreateEntity("Parent2");
		auto pChild1Entity = pWorld->CreateEntity("Child1");
		auto pChild2Entity = pWorld->CreateEntity("Child2");

		auto entities =
		{
			pParent1Entity, pParent2Entity, pChild1Entity, pChild2Entity
		};

		for (auto pCurrEntity : entities)
		{
			pCurrEntity->AddComponent<CDeactivatedComponent>();
			pCurrEntity->AddComponent<CDeactivatedGroupComponent>();
		}

		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParent1Entity->GetId(), pParent2Entity->GetId()));
		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParent2Entity->GetId(), pChild1Entity->GetId()));
		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pChild1Entity->GetId(), pChild2Entity->GetId()));

		REQUIRE(RC_OK == SetEntityActive(pWorld.Get(), pParent2Entity->GetId(), true));

		REQUIRE(pParent1Entity->HasComponent<CDeactivatedComponent>());
		REQUIRE(!pParent2Entity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChild1Entity->HasComponent<CDeactivatedComponent>());
		REQUIRE(pChild2Entity->HasComponent<CDeactivatedComponent>());

		for (auto pCurrEntity : entities)
		{
			REQUIRE(pCurrEntity->HasComponent<CDeactivatedGroupComponent>());
		}
	}

	SECTION("TestFindEntitiesWithComponents_DeactivateOneOfTheEntities_MehtodReturnsAllEntitiesExceptDeactivated")
	{
		const U32 entitiesCount = 10;
		const U32 deactivatedEntitiesCount = 3;

		for (U32 i = 0; i < entitiesCount; i++)
		{
			pWorld->CreateEntity();
		}

		std::unordered_set<TEntityId> deactivatedEntities;

		for (U32 i = 0; i < deactivatedEntitiesCount; i++)
		{
			CEntity* pEntity = pWorld->FindEntity(TEntityId(std::rand() % (entitiesCount - 1) + 1));
			if (!pEntity)
			{
				continue;
			}

			REQUIRE(RC_OK == SetEntityActive(pWorld.Get(), pEntity->GetId(), false));
			deactivatedEntities.emplace(pEntity->GetId());
		}

		auto&& activeEntities = pWorld->FindEntitiesWithComponents<CTransform>();
		REQUIRE(activeEntities.size() == static_cast<USIZE>(entitiesCount - deactivatedEntities.size()));

		for (const TEntityId currEntityId : activeEntities)
		{
			REQUIRE((deactivatedEntities.find(currEntityId) == deactivatedEntities.cend()));
		}
	}

	SECTION("TestCreateLocalComponentsSlice_TryToGetSliceOfComponentsForTwoEntitiesHierarchies_TheSliceContainsParentsEvenIfTheyDontHaveSpecifiedComponents")
	{
		CEntity* pParent1Entity = pWorld->CreateEntity("Parent1");
		CEntity* pParent2Entity = pWorld->CreateEntity("Parent2");
		CEntity* pChild1Entity = pWorld->CreateEntity("Child1");
		CEntity* pChild2Entity = pWorld->CreateEntity("Child2");

		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParent1Entity->GetId(), pChild1Entity->GetId()));
		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParent2Entity->GetId(), pChild2Entity->GetId()));

		pChild1Entity->AddComponent<CTestComponent>();
		pChild2Entity->AddComponent<CTestComponent>();

		auto slice = pWorld->CreateLocalComponentsSlice<CTestComponent, CTransform>();
		REQUIRE(slice.mComponentsCount == 2);

		auto& componentsSlice = std::get<std::vector<CTestComponent*>>(slice.mComponentsSlice);
		REQUIRE(componentsSlice.size() == slice.mComponentsCount);

		auto& transformsMappings = slice.mParentsToChildMapping;
		REQUIRE(transformsMappings.size() == slice.mComponentsCount + 2);
	}

	SECTION("TestCreateLocalComponentsSlice_TryToGetSliceOfComponentsForEntitiesHierarchy_TheSliceContainsParentsEvenIfTheyDontHaveSpecifiedComponents")
	{
		CEntity* pParent1Entity = pWorld->CreateEntity("Parent1");
		CEntity* pChild1Entity = pWorld->CreateEntity("Child1");
		CEntity* pSeparateEntity = pWorld->CreateEntity("Entity3");

		REQUIRE(RC_OK == GroupEntities(pWorld.Get(), pParent1Entity->GetId(), pChild1Entity->GetId()));

		pChild1Entity->AddComponent<CTestComponent>();
		pSeparateEntity->AddComponent<CTestComponent>();

		auto slice = pWorld->CreateLocalComponentsSlice<CTestComponent, CTransform>();
		REQUIRE(slice.mComponentsCount == 2);

		auto& componentsSlice = std::get<std::vector<CTestComponent*>>(slice.mComponentsSlice);
		REQUIRE(componentsSlice.size() == slice.mComponentsCount);

		auto& transformsMappings = slice.mParentsToChildMapping;
		REQUIRE(transformsMappings.size() == slice.mComponentsCount + 1);
	}
}