#include "./../include/CUtilityListener.h"
#define TDE2_YAML_PLUGIN_IMPLEMENTATION
#include "./../plugins/YAMLFormatSupport/include/CYAMLSupportPlugin.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable:4996)
#include "./../deps/stb/stb_image_write.h"
#pragma warning(pop)
#include <memory>


using namespace TDEngine2;


TDEngine2::E_RESULT_CODE CUtilityListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = RC_OK;

	auto pFileSystem = dynamic_cast<TDEngine2::IFileSystem*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_FILE_SYSTEM));

	auto pYamlFile = pFileSystem->Get<CYAMLFileReader>(pFileSystem->Open<CYAMLFileReader>("test.info").Get());

	Yaml::Node root;

	if (pYamlFile->Deserialize(root) != RC_OK)
	{
		return RC_FAIL;
	}

	// read ttf file
	auto pFontFile = pFileSystem->Get<CBinaryFileReader>(pFileSystem->Open<CBinaryFileReader>("arial.ttf").Get());
	
	std::unique_ptr<U8[]> pFontBuffer(new U8[pFontFile->GetFileLength()]);

	if (pFontFile->Read(pFontBuffer.get(), pFontFile->GetFileLength()) != RC_OK)
	{
		return RC_FAIL;
	}

	/// \note All the code below is just test environment which should be later replaced with proper solution
	stbtt_fontinfo font;

	stbtt_InitFont(&font, pFontBuffer.get(), stbtt_GetFontOffsetForIndex(pFontBuffer.get(), 0));

	const std::string text{ "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.,!@#$%^&*()_=+" };

	I32 width, height, xoff, yoff;

	auto pTexAtlasHandler = mpResourceManager->Create<TDEngine2::CTextureAtlas>("TexAtlas", TDEngine2::TTexture2DParameters(512, 512, TDEngine2::FT_NORM_UBYTE1));
	auto pTexAtlas = dynamic_cast<TDEngine2::ITextureAtlas*>(pTexAtlasHandler->Get(TDEngine2::RAT_BLOCKING));

	for (auto ch : text)
	{
		//stbtt_GetCodepointBitmap(&font, stbtt_ScaleForPixelHeight(&font, 24.0f), stbtt_ScaleForPixelHeight(&font, 24.0f), ch, &width, &height, &xoff, &yoff);
		U8* pBitmap =  stbtt_GetCodepointSDF(&font, stbtt_ScaleForPixelHeight(&font, 24.0f), ch, 10, 200, 20.0f, &width, &height, &xoff, &yoff);
		std::string name = "";
		name.push_back(ch);
		name.append(".png");
		//stbi_write_png(name.c_str(), width, height, 1, pBitmap, width);

		assert(pTexAtlas->AddRawTexture(name, width, height, FT_NORM_UBYTE1, pBitmap) == RC_OK);
	}
	
	pTexAtlas->Bake();
	pTexAtlas->Serialize(pFileSystem, "atlas.info");

	auto pTexture = pTexAtlas->GetTexture();

	/// TEST: load texture atlas from a file
	//auto pAtlasHandler = mpResourceManager->Load<CTextureAtlas>("atlas");
	/// END TEST

	// NOTE: delete this code later
	IWorld* pWorld = mpEngineCoreInstance->GetWorldInstance();

	TDEngine2::IMaterial* pMaterial = dynamic_cast<TDEngine2::IMaterial*>(
		mpResourceManager->Create<TDEngine2::CBaseMaterial>("NewMaterial.material",
			TDEngine2::TMaterialParameters{ "testGLShader.shader" })->Get(TDEngine2::RAT_BLOCKING));

	pMaterial->SetTextureResource("TextureAtlas", pTexture);

	//for (TDEngine2::I32 i = 0; i < 10; ++i)
	{
		auto pEntity = pWorld->CreateEntity();

		auto pTransform = pEntity->GetComponent<TDEngine2::CTransform>();

		pTransform->SetScale(TVector3(4.0f));

		auto pSprite = pEntity->AddComponent<TDEngine2::CQuadSprite>();
		pSprite->SetColor(TColor32F(1.0f, 1.0f, 1.0f, 1.0f));

		pSprite->SetMaterialName("NewMaterial.material");
	}

	CEntity* pCameraEntity = pWorld->CreateEntity("Camera");

	pCameraEntity->AddComponent<TDEngine2::COrthoCamera>();

	if (result != TDEngine2::RC_OK)
	{
		return result;
	}

	TDEngine2::TTextureSamplerDesc textureSamplerDesc;

	textureSamplerDesc.mUAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_CLAMP;
	textureSamplerDesc.mVAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_CLAMP;
	textureSamplerDesc.mWAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_CLAMP;

	auto textureSampler = mpGraphicsContext->GetGraphicsObjectManager()->CreateTextureSampler(textureSamplerDesc).Get();

	mpGraphicsContext->BindTextureSampler(0, textureSampler);
	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpWindowSystem->SetTitle(std::to_string(dt));

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnFree()
{
	return RC_OK;
}

void CUtilityListener::SetEngineInstance(TDEngine2::IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = dynamic_cast<TDEngine2::IGraphicsContext*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_GRAPHICS_CONTEXT));

	mpWindowSystem = dynamic_cast<TDEngine2::IWindowSystem*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_WINDOW));

	mpResourceManager = dynamic_cast<TDEngine2::IResourceManager*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_RESOURCE_MANAGER));
}
