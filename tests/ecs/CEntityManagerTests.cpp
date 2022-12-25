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
				return RC_OK;
			}

			E_ENGINE_SUBSYSTEM_TYPE GetType() const { return GetTypeID(); }
			static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_EVENT_MANAGER; }
		private:
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

			E_RESULT_CODE RemoveComponents(TEntityId id) override { return RC_OK; }

			E_RESULT_CODE RemoveComponentsImmediately(TEntityId id) override { return RC_OK; }

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

		E_RESULT_CODE _removeComponentImmediately(TypeId componentTypeId, TEntityId entityId) override { return RC_OK; }

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

	SECTION("TestCreate_PassNothing_ReturnsNewEntityWithDefaultName")
	{
		for (U32 i = 0; i < 1; i++)
		{
			auto pEntity = pEntityManager->Create();
			REQUIRE(pEntity);

			REQUIRE(pEntity->GetId() != TEntityId::Invalid);
			REQUIRE(pEntity->GetName() == Wrench::StringUtils::Format("Entity{0}", i));
		}
	}
}