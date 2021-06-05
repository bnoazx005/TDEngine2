#include "../include/CUtilityListener.h"
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

	/// \todo Load font data
	/// \todo Create a texture atlas for a font
	/// \todo Update characters

	auto pFileSystem = mpEngineCoreInstance->GetSubsystem<TDEngine2::IFileSystem>();

	// read ttf file
	auto pFontFile = pFileSystem->Get<IBinaryFileReader>(pFileSystem->Open<IBinaryFileReader>("arial.ttf").Get());
	
	std::unique_ptr<U8[]> pFontBuffer(new U8[static_cast<U32>(pFontFile->GetFileLength())]);

	if (pFontFile->Read(pFontBuffer.get(), static_cast<U32>(pFontFile->GetFileLength())) != RC_OK)
	{
		return RC_FAIL;
	}

	/// \note All the code below is just test environment which should be later replaced with proper solution
	stbtt_fontinfo font;

	stbtt_InitFont(&font, pFontBuffer.get(), stbtt_GetFontOffsetForIndex(pFontBuffer.get(), 0));

	const std::string text{ "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.,!@#$%^&*()_=+" };

	I32 width, height, xoff, yoff;

	auto fontAtlasHandler = mpResourceManager->Create<TDEngine2::ITextureAtlas>("TexAtlas2", TDEngine2::TTexture2DParameters(512, 512, TDEngine2::FT_NORM_UBYTE1));
	auto pTexAtlas = dynamic_cast<ITextureAtlas*>(mpResourceManager->GetResource(fontAtlasHandler));

	I32 advance, leftBearing;

	auto pFontResource = mpResourceManager->GetResource<TDEngine2::IFont>(mpResourceManager->Load<TDEngine2::IFont>("Arial"));

	F32 scale = stbtt_ScaleForPixelHeight(&font, 50.0f);
	
	int ascent, descent;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, 0);

	ascent = static_cast<I32>(ascent * scale);
	descent = static_cast<I32>(descent * scale);
	
	I32 x0, y0, x1, y1;
	stbtt_GetFontBoundingBox(&font, &x0, &y0, &x1, &y1);

	auto w = scale*(x1 - x0);
	auto h = scale*(y1 - y0);

	for (auto ch : text)
	{
		//stbtt_GetCodepointBitmap(&font, stbtt_ScaleForPixelHeight(&font, 24.0f), stbtt_ScaleForPixelHeight(&font, 24.0f), ch, &width, &height, &xoff, &yoff);
		U8* pBitmap =  stbtt_GetCodepointSDF(&font, scale, ch, 10, 255, 20.0f, &width, &height, &xoff, &yoff);
		std::string name = "";
		name.push_back(ch);

		//stbi_write_png(name.c_str(), width, height, 1, pBitmap, width);

		assert(pTexAtlas->AddRawTexture(name, width, height, FT_NORM_UBYTE1, pBitmap) == RC_OK);

		stbtt_GetCodepointHMetrics(&font, ch, &advance, &leftBearing);
		
		pFontResource->AddGlyphInfo(ch, { static_cast<U16>(width), static_cast<U16>(height), static_cast<I16>(xoff), static_cast<I16>(yoff), scale * advance });
	}
	
	pTexAtlas->Bake();
	//pTexAtlas->Serialize(pFileSystem, "atlas2.info");

	//pFontResource->Serialize(pFileSystem, "Arial2.font");

	auto pTexture = pTexAtlas->GetTexture();




	mpPreviewEditorWindow = dynamic_cast<CFontPreviewWindow*>(TDEngine2::CreateFontPreviewWindow(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IInputContext>(), mpWindowSystem, result));
	mpPreviewEditorWindow->SetTextureAtlasResourceHandle(fontAtlasHandler);

	mpConfigEditorWindow = CreateConfigWindow({ mpResourceManager, mpEngineCoreInstance->GetSubsystem<IInputContext>(), mpWindowSystem, pFileSystem }, result);

	/// \note For this tool this entity isn't used but create it to suppress assertions
	if (IWorld* pWorld = mpEngineCoreInstance->GetWorldInstance())
	{
		if (CEntity* pCameraEntity = pWorld->CreateEntity("Camera"))
		{
			pCameraEntity->AddComponent<TDEngine2::COrthoCamera>();
		}
	}

	if (result != TDEngine2::RC_OK)
	{
		return result;
	}

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpPreviewEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);
	mpConfigEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);

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

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<TDEngine2::IGraphicsContext>();

	mpWindowSystem = mpEngineCoreInstance->GetSubsystem<TDEngine2::IWindowSystem>();

	mpResourceManager = mpEngineCoreInstance->GetSubsystem<TDEngine2::IResourceManager>();
}
