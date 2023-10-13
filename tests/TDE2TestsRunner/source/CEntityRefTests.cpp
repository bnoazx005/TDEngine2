#include <TDEngine2.h>
#include "deferOperation.hpp"


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED


TDE2_API TDEngine2::IComponent* CreateEntityRefHolder(TDEngine2::E_RESULT_CODE& result);


class CEntityRefHolder : public TDEngine2::CBaseComponent, public TDEngine2::CPoolMemoryAllocPolicy<CEntityRefHolder, 1 << 20>
{
	public:
		friend TDE2_API TDEngine2::IComponent* CreateEntityRefHolder(TDEngine2::E_RESULT_CODE&);
	public:
		TDE2_REGISTER_COMPONENT_TYPE(CEntityRefHolder)

		TDE2_API TDEngine2::E_RESULT_CODE Load(TDEngine2::IArchiveReader* pReader) override
		{
			if (!pReader)
			{
				return RC_FAIL;
			}

			mRef = LoadEntityRef(pReader, "ref");

			return RC_OK;
		}

		TDE2_API TDEngine2::E_RESULT_CODE Save(TDEngine2::IArchiveWriter* pWriter) override
		{
			if (!pWriter)
			{
				return RC_FAIL;
			}

			pWriter->BeginGroup("component");
			{
				pWriter->SetUInt32("type_id", static_cast<U32>(CEntityRefHolder::GetTypeId()));
				SaveEntityRef(pWriter, "ref", mRef);
			}
			pWriter->EndGroup();

			return RC_OK;
		}

		TDE2_API TDEngine2::E_RESULT_CODE PostLoad(TDEngine2::CEntityManager* pEntityManager, const TDEngine2::TEntitiesMapper& entitiesIdentifiersRemapper) override
		{
			mRef.PostLoad(pEntityManager, entitiesIdentifiersRemapper);

			return CBaseComponent::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
		}

		TDE2_API TDEngine2::E_RESULT_CODE Clone(TDEngine2::IComponent*& pDestObject) const override
		{
			if (CEntityRefHolder* pObject = dynamic_cast<CEntityRefHolder*>(pDestObject))
			{
				pObject->mRef = mRef;

				return RC_OK;
			}

			return RC_FAIL;
		}
	protected:
		DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CEntityRefHolder)
	public:
		TDEngine2::CEntityRef mRef;
};


typedef struct TEntityRefHolderParameters : public TDEngine2::TBaseComponentParameters
{
} TEntityRefHolderParameters;


TDE2_DECLARE_COMPONENT_FACTORY(EntityRefHolder, TEntityRefHolderParameters);



CEntityRefHolder::CEntityRefHolder():
	CBaseComponent()
{
}


IComponent* CreateEntityRefHolder(E_RESULT_CODE& result)
{
	return CREATE_IMPL(IComponent, CEntityRefHolder, result);
}


CEntityRefHolderFactory::CEntityRefHolderFactory() :
	CBaseComponentFactory()
{
}

IComponent* CEntityRefHolderFactory::CreateDefault() const
{
	E_RESULT_CODE result = RC_OK;
	return CreateEntityRefHolder(result);
}

E_RESULT_CODE CEntityRefHolderFactory::SetupComponent(CEntityRefHolder* pComponent, const TEntityRefHolderParameters& params) const
{
	if (!pComponent)
	{
		return RC_INVALID_ARGS;
	}

	return RC_OK;
}


IComponentFactory* CreateEntityRefHolderFactory(E_RESULT_CODE& result)
{
	return CREATE_IMPL(IComponentFactory, CEntityRefHolderFactory, result);
}



TDE2_TEST_FIXTURE("EntityRefTests")
{
	TDE2_TEST_CASE("TestSave_LoadSceneWithNestedPrefabsThenCreateReferenceAndTryToSerializeIt_TheReferenceShouldBeCorrectlySaved")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		pTestCase->ExecuteAction([&] // Serialize a path to child of a prefab
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			auto&& prefabRootEntity = pWorld->FindEntity(pTestScene->GetEntities()[1]);
			TDE2_TEST_IS_TRUE(prefabRootEntity);

			auto pTransform = prefabRootEntity->GetComponent<CTransform>();

			const TEntityId childEntityId = pTransform->GetChildren().front();

			/// \note Create a ref and try to serialize that
			CEntityRef ref(pWorld->GetEntityManager(), childEntityId);
			TDE2_TEST_IS_TRUE(ref.IsResolved());

			{
				E_RESULT_CODE result = RC_OK;
				
				auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
				if (!pMemoryMappedStream)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
				if (!pFileWriter)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				result = result | ref.Save(pFileWriter);
				result = result | pFileWriter->Close();

				auto&& pathParts = ref.GetPath();

				TDE2_TEST_IS_TRUE(pathParts.size() == 2);
				TDE2_TEST_IS_TRUE(static_cast<TEntityId>(pathParts.front()) == prefabRootEntity->GetId());
				TDE2_TEST_IS_TRUE(pathParts.back() == pWorld->FindEntity(childEntityId)->GetComponent<CObjIdComponent>()->mId);

				TDE2_TEST_IS_TRUE(RC_OK == result);
			}
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}

	TDE2_TEST_CASE("TestSave_LoadSceneWithNestedPrefabsThenCreateReferenceToRootAndToSerializeIt_TheReferenceShouldContainSingleIdToPrefabRoot")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		pTestCase->ExecuteAction([&] /// Try to serialize a root of a prefab which is attached to some scene's entity
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			CEntity* pNestedPrefabRoot = pTestScene->Spawn("TestNestedPrefab", pWorld->FindEntity(pTestScene->GetEntities()[1]));
			TDE2_TEST_IS_TRUE(pNestedPrefabRoot);

			CEntityRef ref(pWorld->GetEntityManager(), pNestedPrefabRoot->GetId());
			TDE2_TEST_IS_TRUE(ref.IsResolved());

			{
				E_RESULT_CODE result = RC_OK;

				auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
				if (!pMemoryMappedStream)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
				if (!pFileWriter)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				result = result | ref.Save(pFileWriter);
				result = result | pFileWriter->Close();

				auto&& pathParts = ref.GetPath();

				TDE2_TEST_IS_TRUE(pathParts.size() == 1);
				TDE2_TEST_IS_TRUE(static_cast<TEntityId>(pathParts.front()) == pNestedPrefabRoot->GetId());

				TDE2_TEST_IS_TRUE(RC_OK == result);
			}
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}

	TDE2_TEST_CASE("TestSave_LoadSceneThenAddEntityHolderComponentWithReferenceToSomePrefabsChildAndSerializeIt_TheRefShouldContainSingleIdToThatChild")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;

		static const std::string TestSceneId = "TestLoadEntityRefs.scene";

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			E_RESULT_CODE result = RC_OK;
			pWorld->RegisterComponentFactory(TPtr<IComponentFactory>(CreateEntityRefHolderFactory(result)));

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestLoadEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId(TestSceneId)).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			CEntity* pPrefab = pWorld->FindEntity(TEntityId(10302));
			TDE2_TEST_IS_TRUE(pPrefab);

			CEntityRefHolder* pHolder = pPrefab->AddComponent<CEntityRefHolder>();
			pHolder->mRef = CEntityRef(pWorld->GetEntityManager(), pPrefab->GetComponent<CTransform>()->GetChildren()[1]);

			TDE2_TEST_IS_TRUE(pHolder->mRef.IsResolved());

			{
				E_RESULT_CODE result = RC_OK;

				auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
				if (!pMemoryMappedStream)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
				if (!pFileWriter)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				result = result | pPrefab->Save(pFileWriter);
				result = result | pFileWriter->Close();

				auto&& pathParts = pHolder->mRef.GetPath();

				TDE2_TEST_IS_TRUE(pathParts.size() == 1);
				TDE2_TEST_IS_TRUE(RC_OK == result);
			}
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId(TestSceneId)).Get();
			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}

	TDE2_TEST_CASE("TestSave_LoadSceneWithNestedPrefabsThenCreateReferenceToSimpleEntityAndSerializeIt_TheReferenceIsSingleEntityId")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		pTestCase->ExecuteAction([&] /// Try to serialize a reference to simple entity of the scene
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			CEntity* pRootEntity = pTestScene->FindEntityByPath("Root", nullptr);
			TDE2_TEST_IS_TRUE(pRootEntity);

			CEntityRef ref(pWorld->GetEntityManager(), pRootEntity->GetId());
			TDE2_TEST_IS_TRUE(ref.IsResolved());

			{
				E_RESULT_CODE result = RC_OK;

				auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
				if (!pMemoryMappedStream)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
				if (!pFileWriter)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				result = result | ref.Save(pFileWriter);
				result = result | pFileWriter->Close();

				auto&& pathParts = ref.GetPath();

				TDE2_TEST_IS_TRUE(pathParts.size() == 1);
				TDE2_TEST_IS_TRUE(static_cast<TEntityId>(pathParts.front()) == pRootEntity->GetId());

				TDE2_TEST_IS_TRUE(RC_OK == result);
			}
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}

	TDE2_TEST_CASE("TestSave_LoadSceneThenSpawnAPrefabAndCreateReferenceToItsChildEntityAndTryToSerializeIt_TheReferenceShouldBeCorrectlySaved")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;
		static CEntity* pNestedPrefabRoot = nullptr;

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			pNestedPrefabRoot = pTestScene->Spawn("TestNestedPrefab");
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();

			auto pTargetEntity = pTestScene->FindEntityByPath("Entity2/Entity0/Entity1", pNestedPrefabRoot);
			TDE2_TEST_IS_TRUE(pTargetEntity);

			/// \note Create a ref and try to serialize that
			CEntityRef ref(pWorld->GetEntityManager(), pTargetEntity->GetId());
			TDE2_TEST_IS_TRUE(ref.IsResolved());

			{
				E_RESULT_CODE result = RC_OK;

				auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
				if (!pMemoryMappedStream)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
				if (!pFileWriter)
				{
					TDE2_TEST_IS_TRUE(false);
					return;
				}

				///  1344.1867944444
				result = result | ref.Save(pFileWriter);
				result = result | pFileWriter->Close();

				auto&& pathParts = ref.GetPath();

				TDE2_TEST_IS_TRUE(pathParts.size() == 2);
				TDE2_TEST_IS_TRUE(static_cast<TEntityId>(pathParts.front()) == pTestScene->FindEntityByPath("Entity2/Entity0", pNestedPrefabRoot)->GetId());
				TDE2_TEST_IS_TRUE(pathParts.back() == pTargetEntity->GetComponent<CObjIdComponent>()->mId);

				TDE2_TEST_IS_TRUE(RC_OK == result);

				TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
			}
		});
	}

	TDE2_TEST_CASE("TestLoad_LoadSceneWithEntityRefToRootEntity_TheReferenceShouldBeCorrectlyResolvedAfterLoading")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;
		static const std::string TestSceneId = "TestLoadEntityRefs.scene";

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			E_RESULT_CODE result = RC_OK;
			pWorld->RegisterComponentFactory(TPtr<IComponentFactory>(CreateEntityRefHolderFactory(result)));

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestLoadEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		pTestCase->ExecuteAction([&] 
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId(TestSceneId)).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			auto&& entities = pWorld->FindEntitiesWithComponents<CEntityRefHolder>();
			TDE2_TEST_IS_TRUE(!entities.empty());

			auto pEntityRefHolder = pWorld->FindEntity(entities.front())->GetComponent<CEntityRefHolder>();
			TDE2_TEST_IS_TRUE(pEntityRefHolder);
			
			const TEntityId entityId = pEntityRefHolder->mRef.Get();
			TDE2_TEST_IS_TRUE(entityId != TEntityId::Invalid);

			TDE2_TEST_IS_TRUE(pWorld->FindEntity(entityId) == pTestScene->FindEntityByPath("Root", nullptr));
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pWorld->UnregisterComponentFactory(CEntityRefHolder::GetTypeId());

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId(TestSceneId)).Get();
			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}

	TDE2_TEST_CASE("TestGet_LoadASceneAndResolveReferenceToRootOfAPrefab_EntityRefShouldBeCorrectlyResolved")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;
		static CEntity* pNestedPrefabRoot = nullptr;

		/// \note Load a scene
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			constexpr TEntityId expectedEntityId = TEntityId(10302);

			CEntityRef ref(pWorld->GetEntityManager(), Wrench::StringUtils::Format("{0}", static_cast<U32>(expectedEntityId))); // 10302 is a root of a TestPrefab
			const TEntityId resolvedEntityId = ref.Get();

			TDE2_TEST_IS_TRUE(resolvedEntityId == expectedEntityId);
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(pSceneManager->GetSceneId("TestEntityRefs.scene")).Get();
			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}
}

#endif