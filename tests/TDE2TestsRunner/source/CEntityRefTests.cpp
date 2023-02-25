#include <TDEngine2.h>
#include "deferOperation.hpp"


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED

TDE2_TEST_FIXTURE("EntityRefTests")
{
	TDE2_TEST_CASE("TestSave_LoadSceneWithNestedPrefabsThenCreateReferenceAndTryToSerializeIt_TheReferenceShouldBeCorrectlySaved")
	{
		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestEntityRefs.scene", nullptr);
		});

		pTestCase->Wait(1.5f);

		pTestCase->ExecuteAction([&]
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
	}
}

#endif