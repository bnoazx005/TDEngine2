#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


namespace
{
	TDE2_DECLARE_FLAG_COMPONENT(TestComponent);
	TDE2_DEFINE_FLAG_COMPONENT(TestComponent);

	TDE2_DECLARE_FLAG_COMPONENT(UniqueComponent);
	TDE2_DEFINE_FLAG_COMPONENT(UniqueComponent);
	TDE2_REGISTER_UNIQUE_COMPONENT(CUniqueComponent);
}


TEST_CASE("CComponentManager Tests")
{
	E_RESULT_CODE result = RC_OK;

	TPtr<IComponentManager> pComponentManager = TPtr<IComponentManager>(CreateComponentManager(result));
	REQUIRE(pComponentManager);

	SECTION("TestRegisterFactory_PassInvalidParameter_ReturnsRC_INVALID_ARGS")
	{
		REQUIRE(RC_INVALID_ARGS == pComponentManager->RegisterFactory(nullptr));
	}

	SECTION("TestRegisterFactory_TryToRegisterSameFactoryTwice_ReturnsRC_OKAndDoNothingForDuplicate")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		
		REQUIRE(pFactory);
		REQUIRE(RC_OK == pComponentManager->RegisterFactory(pFactory));
		REQUIRE(RC_OK == pComponentManager->RegisterFactory(pFactory));
	}

	SECTION("TestUnregisterFactory_TryToUnregisterInexistingFactory_ReturnsRC_FAIL")
	{
		REQUIRE(RC_FAIL == pComponentManager->UnregisterFactory<CTestComponent>());
	}

	SECTION("TestUnregisterFactory_TryToUnregisterAlreadyDetachedFactory_ReturnsRC_FAIL")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		REQUIRE(RC_OK == pComponentManager->UnregisterFactory<CTestComponent>());
		REQUIRE(RC_FAIL == pComponentManager->UnregisterFactory<CTestComponent>());
	}

	SECTION("TestCreateComponent_TryToCreateComponentForInvalidEntity_ReturnsNullptr")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		REQUIRE(!pComponentManager->CreateComponent<CTestComponent>(TEntityId::Invalid));
	}

	SECTION("TestCreateComponent_TryToCreateInexistingComponent_ReturnsNullptr")
	{
		REQUIRE(!pComponentManager->CreateComponent<CTestComponent>(TEntityId(1)));
	}

	SECTION("TestCreateComponent_TryToCreateComponentForEntityThatAlreadyHasThat_ReturnsPointerToExistingComponent")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		auto pNewComponent = pComponentManager->CreateComponent<CTestComponent>(TEntityId(1));
		REQUIRE(pNewComponent);

		REQUIRE(pNewComponent == pComponentManager->CreateComponent<CTestComponent>(TEntityId(1)));
	}

	SECTION("TestRemoveComponent_TryToRemoveInexistingComponentForExistingEntity_ReturnsRC_FAIL")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		pFactory = TPtr<IComponentFactory>(CreateUniqueComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		const TEntityId entityId = TEntityId(1);

		REQUIRE(pComponentManager->CreateComponent<CUniqueComponent>(entityId));
		REQUIRE(RC_FAIL == pComponentManager->RemoveComponent<CTestComponent>(entityId));
	}

	SECTION("TestRemoveComponent_TryToRemoveComponentForInexistingEntity_ReturnsRC_FAIL")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		REQUIRE(RC_FAIL == pComponentManager->RemoveComponent<CTestComponent>(TEntityId(1)));
	}

	SECTION("TestRemoveComponent_TryToRemoveExistingComponentForExistingEntity_ReturnsRC_OK")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		pFactory = TPtr<IComponentFactory>(CreateUniqueComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		const TEntityId entityId = TEntityId(1);

		REQUIRE(pComponentManager->CreateComponent<CUniqueComponent>(entityId));
		REQUIRE(RC_OK == pComponentManager->RemoveComponent<CUniqueComponent>(entityId));
	}

	SECTION("TestRemoveComponent_TryToRemoveSameComponentTwiceForSameEntity_ReturnsRC_FAILForSecondInvokation")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		const TEntityId entityId = TEntityId(1);

		REQUIRE(pComponentManager->CreateComponent<CTestComponent>(entityId));
		
		REQUIRE(RC_OK == pComponentManager->RemoveComponent<CTestComponent>(entityId));
		REQUIRE(RC_FAIL == pComponentManager->RemoveComponent<CTestComponent>(entityId));
	}

	SECTION("TestRemoveComponents_TryToInvokeForInexistingEntity_ReturnsRC_FAIL")
	{
		REQUIRE(RC_FAIL == pComponentManager->RemoveComponents(TEntityId::Invalid));
		REQUIRE(RC_FAIL == pComponentManager->RemoveComponents(TEntityId(1)));
	}

	SECTION("TestRemoveComponents_TryToRemoveComponentsForExistingEntity_ReturnsRC_OK")
	{
		auto pFactory = TPtr<IComponentFactory>(CreateTestComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		pFactory = TPtr<IComponentFactory>(CreateUniqueComponentFactory(result));
		REQUIRE((pFactory && RC_OK == pComponentManager->RegisterFactory(pFactory)));

		const TEntityId entityId = TEntityId(1);

		REQUIRE(pComponentManager->CreateComponent<CUniqueComponent>(entityId));
		REQUIRE(pComponentManager->CreateComponent<CTestComponent>(entityId));

		REQUIRE(RC_OK == pComponentManager->RemoveComponents(entityId));
	}

	/// GetComponents
	/// FindComponentsOfType
	/// FindEntitiesWithAll
	/// FindEntitiesWithAny
	/// FindEntityWithUniqueComponent
	/// HasComponent
	/// GetComponent
}