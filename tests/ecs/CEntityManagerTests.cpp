#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


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


	class CStubComponentManager : public IComponentManager, public CBaseObject
	{
		private:
			static std::vector<IComponent*> mpComponents;
			static CComponentIterator mInvalidIterator;
		public:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStubComponentManager)

			E_RESULT_CODE Init() override
			{
				mIsInitialized = true;
				return RC_OK;
			}

			E_RESULT_CODE RegisterFactory(TPtr<IComponentFactory> pFactory) override
			{
				return RC_OK;
			}

			E_RESULT_CODE UnregisterFactory(TypeId typeId) override
			{
				return RC_OK;
			}

			IComponent* CreateComponent(TEntityId entityId, TypeId componentTypeId) override
			{
				return nullptr;
			}

			E_RESULT_CODE RemoveComponent(TypeId componentTypeId, TEntityId entityId) override
			{
				return RC_OK;
			}

			E_RESULT_CODE RemoveComponents(TEntityId id, const std::function<void(std::vector<TypeId>)>& onComponentsRemoved = nullptr) override { return RC_OK; }

			std::vector<IComponent*> GetComponents(TEntityId id) const override { return {}; }

			bool HasComponent(TEntityId id, TypeId componentTypeId) override { return false; }

			CComponentIterator FindComponentsOfType(TypeId typeId) override	{ return mInvalidIterator; }

			void ForEach(TypeId componentTypeId, const std::function<void(TEntityId entityId, IComponent* pComponent)>& action) override {}

			void ForEachFactory(const std::function<void(TPtr<IComponentFactory>)>& action = nullptr) override { }

			std::vector<TEntityId> FindEntitiesWithAll(const std::vector<TypeId>& types) override { return {}; }

			std::vector<TEntityId> FindEntitiesWithAny(const std::vector<TypeId>& types) override { return {}; }

			TEntityId FindEntityWithUniqueComponent(TypeId typeId) override { return TEntityId::Invalid; }

#if TDE2_EDITORS_ENABLED
			const std::vector<TComponentTypeInfo>& GetRegisteredComponentsIdentifiers() const override
			{
				static std::vector<TComponentTypeInfo> infos;
				return infos;
			}
#endif
	protected:
		IComponent* _createComponent(TypeId componentTypeId, TEntityId entityId) override 
		{ 
			return nullptr;
		}

		IComponent* _getComponent(TypeId componentTypeId, TEntityId entityId) override { return nullptr; }

		E_RESULT_CODE _removeComponentWithAction(TypeId componentTypeId, TEntityId entityId,
			const std::function<E_RESULT_CODE(IComponent*&)>& action)
		{
			return RC_OK;
		}

		bool _hasComponent(TypeId componentTypeId, TEntityId entityId) override { return false; }
	};

	std::vector<IComponent*> CStubComponentManager::mpComponents;
	CComponentIterator CStubComponentManager::mInvalidIterator(CStubComponentManager::mpComponents, 0);

	CStubComponentManager::CStubComponentManager() :
		CBaseObject()
	{
	}
}


struct TFinalizer
{
	TPtr<CMemoryProfiler> mpMemProfiler = nullptr;

	TFinalizer()
	{
		mpMemProfiler = CMemoryProfiler::Get();
	}

	~TFinalizer()
	{
		CPoolAllocatorsRegistry::ClearAllAllocators(); /// As we work with components we should clear up their allocators
		mpMemProfiler = nullptr;
	}
} Finalizer;


TEST_CASE("CEntityManager Tests")
{
	E_RESULT_CODE result = RC_OK;

	TPtr<IEventManager> pEventManager = TPtr<IEventManager>(new CStubEventManager());
	TPtr<IComponentManager> pComponentManager = TPtr<IComponentManager>(new CStubComponentManager());

	REQUIRE((pEventManager && pComponentManager));

	TPtr<CEntityManager> pEntityManager = TPtr<CEntityManager>(CreateEntityManager(pEventManager.Get(), pComponentManager.Get(), false, result));
	REQUIRE(pEntityManager);

	auto pStubEventManager = DynamicPtrCast<CStubEventManager>(pEventManager);

	SECTION("TestCreate_PassNothing_ReturnsNewEntityWithDefaultName")
	{
		const U32 expectedEntitiesCount = 10;
		U32 actualEntitiesCount = 0;

		pStubEventManager->AddSimpleListener(TOnEntityCreatedEvent::GetTypeId(), [&actualEntitiesCount]
		{
			actualEntitiesCount++;
		});

		for (U32 i = 0; i < expectedEntitiesCount; i++)
		{
			auto pEntity = pEntityManager->Create();
			REQUIRE(pEntity);

			REQUIRE(pEntity->GetId() != TEntityId::Invalid);
			REQUIRE(pEntity->GetName() == Wrench::StringUtils::Format("Entity{0}", i));
		}

		REQUIRE(expectedEntitiesCount == actualEntitiesCount);
	}

	SECTION("TestCreate_PassUniqueName_ReturnsNewEntityWithTheGivenIdentifier")
	{
		const std::string identifiers[] =
		{
			"FirstEntity", "SecondEntity", "ThirdEntity"
		};

		const U32 expectedEntitiesCount = sizeof(identifiers) / sizeof(identifiers[0]);
		U32 actualEntitiesCount = 0;

		pStubEventManager->AddSimpleListener(TOnEntityCreatedEvent::GetTypeId(), [&actualEntitiesCount]
		{
			actualEntitiesCount++;
		});

		for (const std::string& name : identifiers)
		{
			auto pEntity = pEntityManager->Create(name);
			REQUIRE(pEntity);

			REQUIRE(pEntity->GetId() != TEntityId::Invalid);
			REQUIRE(pEntity->GetName() == name);
		}

		REQUIRE(expectedEntitiesCount == actualEntitiesCount);
	}

	SECTION("TestDestroy_CreateSingleEntityAndDestroyThat_ItCorrectlyDestroyedAndNotifiesListeners")
	{
		bool hasEntityBeenDestroyed = false;

		pStubEventManager->AddSimpleListener(TOnEntityRemovedEvent::GetTypeId(), [&hasEntityBeenDestroyed]
		{
			hasEntityBeenDestroyed = true;
		});

		auto pEntity = pEntityManager->Create();
		REQUIRE(pEntity);

		E_RESULT_CODE result = pEntityManager->Destroy(pEntity->GetId());
		REQUIRE((RC_OK == result && hasEntityBeenDestroyed));
	}

	SECTION("TestDestroy_CreateFewEntitiesDestroyTheFirstOne_AllExistingEntitiesShouldBeValid")
	{
		const U32 entitiesCount = 15;

		std::unordered_set<TEntityId> createdEntities;

		for (U32 i = 0; i < entitiesCount; i++)
		{
			createdEntities.emplace(pEntityManager->Create()->GetId());
		}

		/// \note Destroy some entity in between
		const TEntityId entityToDestroy = TEntityId(*createdEntities.begin());

		E_RESULT_CODE result = pEntityManager->Destroy(entityToDestroy);
		REQUIRE(RC_OK == result);

		/// \note Now all the entities except the destroyed one should exist and be accessible by their handles
		for (auto it = createdEntities.begin(); it != createdEntities.end(); it++)
		{
			auto pEntity = pEntityManager->GetEntity(TEntityId(*it));

			if (it == createdEntities.begin())
			{
				REQUIRE(!pEntity);
				continue;
			}

			REQUIRE(pEntity);
		}
	}

	SECTION("TestDestroy_CreateFewEntitiesDestroySomeInBetween_AllExistingEntitiesShouldBeValid")
	{
		const U32 entitiesCount = 15;

		std::unordered_set<TEntityId> createdEntities;
		TEntityId entityToDestroy;

		for (U32 i = 0; i < entitiesCount; i++)
		{
			auto id = pEntityManager->Create()->GetId();
			createdEntities.emplace(id);

			if (i == entitiesCount / 2)
			{
				entityToDestroy = id;
			}
		}

		/// \note Destroy some entity in between
		E_RESULT_CODE result = pEntityManager->Destroy(entityToDestroy);
		REQUIRE(RC_OK == result);

		/// \note Now all the entities except the destroyed one should exist and be accessible by their handles
		for (TEntityId currEntityId : createdEntities)
		{
			auto pEntity = pEntityManager->GetEntity(currEntityId);
			
			if (entityToDestroy == currEntityId)
			{
				REQUIRE(!pEntity);
				continue;
			}

			REQUIRE(pEntity);
		}
	}
}