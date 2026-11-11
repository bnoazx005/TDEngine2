#include <TDEngine2.h>


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED


TDE2_TEST_FIXTURE("Resources Loaders Tests")
{
	TDE2_TEST_CASE("TestLoad_PassCorrect3DTexturePath_ResourceManagerShouldLoadAndReturnHandle")
	{
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			TPtr<IResourceManager> pResourceManager = pEngineCore->GetSubsystem<IResourceManager>();

			TDE2_TEST_IS_TRUE(pResourceManager && pResourceManager->Load<ITexture3D>("DefaultResources/Textures/LUTs/ColorLUT.texture3d.info") != TResourceId::Invalid);
		});
	}
}

#endif