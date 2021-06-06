#include "../include/CConfigWindow.h"
#include <tuple>
#include <vector>
#include <string>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <memory>


namespace TDEngine2
{
	typedef std::vector<std::tuple<std::string, std::string>> TFileFiltersArray;

	const TFileFiltersArray FileExtensionsFilter
	{
		{ "Font info", "*.info" }
	};


	static TResult<TResourceId> OpenFromFile(IWindowSystem* pWindowSystem, IFileSystem* pFileSystem, IResourceManager* pResourceManager, const TFileFiltersArray& filters,
		const std::function<TResult<TResourceId>(const std::string&)>& onSuccessAction)
	{
		if (!pWindowSystem || !pFileSystem || !pResourceManager)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		auto openFileResult = pWindowSystem->ShowOpenFileDialog(filters);
		if (openFileResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(openFileResult.GetError());
		}

		return onSuccessAction(openFileResult.Get());
	}


	static E_RESULT_CODE SaveToFile(IFileSystem* pFileSystem, IResourceManager* pResourceManager, TResourceId fontResourceId, TResourceId atlasResourceId, const std::string& destFontFilePath,
									const std::string& destAtlasFilePath)
	{
		if (destFontFilePath.empty() || 
			destAtlasFilePath.empty() || 
			(TResourceId::Invalid == fontResourceId) || 
			(TResourceId::Invalid == atlasResourceId) || 
			!pFileSystem || 
			!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		if (auto openFileResult = pFileSystem->Open<IYAMLFileWriter>(destFontFilePath, true))
		{
			if (auto pFileWriter = pFileSystem->Get<IYAMLFileWriter>(openFileResult.Get()))
			{
				if (auto pFont = pResourceManager->GetResource<IFont>(fontResourceId))
				{
					if (RC_OK != (result = pFont->Save(pFileWriter)))
					{
						return result;
					}
				}

				if (RC_OK != (result = pFileWriter->Close()))
				{
					return result;
				}
			}
		}

		if (auto pAtlasTexture = pResourceManager->GetResource<ITextureAtlas>(atlasResourceId))
		{
			return CTextureAtlas::Serialize(pFileSystem, pAtlasTexture, destAtlasFilePath);
		}

		return RC_FAIL;
	}


	CConfigWindow::CConfigWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CConfigWindow::Init(const TConfigWindowParams& params)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!params.mpResourceManager || !params.mpWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = params.mpResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(params.mpInputContext);
		mpWindowSystem = params.mpWindowSystem;
		mpFileSystem = params.mpFileSystem;

		mFontTextureAtlasId = TResourceId::Invalid;

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CConfigWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	TResourceId CConfigWindow::GetFontAtlasHandle() const
	{
		return mFontTextureAtlasId;
	}

	void CConfigWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(300.0f, 150.0f),
			TVector2(300.0f, 1000.0f),
		};

		if (mpImGUIContext->BeginWindow("Settings", isEnabled, params))
		{
			_fontSelectionToolbar();
			_fontSDFConfiguration();
			_updateFontsAtlas();
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	void CConfigWindow::_fontSelectionToolbar()
	{
		mpImGUIContext->BeginHorizontal();
		{
			const TVector2 buttonSizes{ mpImGUIContext->GetWindowWidth() * 0.45f, 25.0f };

			mpImGUIContext->Button("Open", buttonSizes, [this]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem, mpFileSystem, mpResourceManager, FileExtensionsFilter, [this](auto&& path)
				{
					if (IResource* pAtlas = mpResourceManager->GetResource<IResource>(mFontResourceId))
					{
						pAtlas->Unload();
					}

					return Wrench::TOkValue<TResourceId>(mpResourceManager->Load<ITextureAtlas>(path));
				}))
				{
					mFontResourceId = openFileResult.Get();
					mFontFileName = mpResourceManager->GetResource(mFontResourceId)->GetName();

					mLastSavedPath = Wrench::StringUtils::GetEmptyStr();
				}
			});

			mpImGUIContext->Button("Save", buttonSizes, [this]
			{
				if (!mLastSavedPath.empty())
				{
					//SaveToFile(mpFileSystem, mpResourceManager, mFontResourceId, mLastSavedPath);
					return;
				}

				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					//SaveToFile(mpFileSystem, mpResourceManager, mFontResourceId, mLastSavedPath);
				}
			});
		}		
		mpImGUIContext->EndHorizontal();

		/// \note Font's name
		mpImGUIContext->BeginHorizontal();
		{
			mpImGUIContext->Label("Font Name:");

			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth() * 0.4f, [this]
			{
				mpImGUIContext->TextField("##FontName", mFontFileName);
			});
		}
		mpImGUIContext->EndHorizontal();
	}

	void CConfigWindow::_fontSDFConfiguration()
	{
		/// \note a path to TTF font
		mpImGUIContext->BeginHorizontal();
		{
			mpImGUIContext->Label("Font TTF Path:");

			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth() * 0.4f, [this]
			{
				mpImGUIContext->TextField("##FontPath", mTTFFontFilePath);
			});

			mpImGUIContext->Button("Browse", TVector2(mpImGUIContext->GetWindowWidth() * 0.16f, 25.0f), [this]
			{
				OpenFromFile(mpWindowSystem, mpFileSystem, mpResourceManager, { { "TrueType Fonts", "*.ttf" } }, [this](auto&& path)
				{
					mTTFFontFilePath = path;
					return Wrench::TOkValue<TResourceId>(TResourceId::Invalid); // \note Isn't used
				});
			});
		}
		mpImGUIContext->EndHorizontal();

		/// \note SDF configs

		// \note Glyph's height
		mpImGUIContext->BeginHorizontal();
		{
			mpImGUIContext->Label("Glyph Height:");

			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth() * 0.55f, [this]
			{
				mpImGUIContext->FloatSlider("##GlyphHeight", mGlyphHeight, 0.0f, 100.0f);
			});
		}
		mpImGUIContext->EndHorizontal();

		// \note Texture's parameters
		if (std::get<0>(mpImGUIContext->BeginTreeNode("Atlas settings")))
		{
			_fontAtlasSettings();
			mpImGUIContext->EndTreeNode();
		}
		
		// Characters that are included into the font
		mpImGUIContext->BeginHorizontal();
		{
			mpImGUIContext->Label("Characters:");

			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth() * 0.65f, [this]
			{
				mpImGUIContext->TextField("##Alphabet", mFontAlphabet);
			});
		}
		mpImGUIContext->EndHorizontal();
	}

	void CConfigWindow::_fontAtlasSettings()
	{
		mpImGUIContext->BeginHorizontal();
		{
			mpImGUIContext->Label("Font Atlas Name:");

			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth() * 0.45f, [this]
			{
				mpImGUIContext->TextField("##AtlasName", mFontAtlasName);
			});
		}
		mpImGUIContext->EndHorizontal();

		mpImGUIContext->BeginHorizontal();
		{
			mpImGUIContext->SetItemWidth(mpImGUIContext->GetWindowWidth() * 0.25f, [this]
			{
				mpImGUIContext->Label("Width");
				mpImGUIContext->IntField("##Width", mAtlasWidth);
				mpImGUIContext->Label("Height");
				mpImGUIContext->IntField("##Height", mAtlasHeight);
			});
		}
		mpImGUIContext->EndHorizontal();
	}


	struct TFontGeneratorParams
	{
		ITextureAtlas* mpFontTextureAtlas;
		IFont* mpFont;
		stbtt_fontinfo* mpFontInfo;
		std::string mCharacters;
		F32 mGlyphHeight;
	};

	
	static void GenerateFontTexture(const TFontGeneratorParams& params)
	{
		I32 width, height, xoff, yoff;
		I32 advance, leftBearing;

		const stbtt_fontinfo* pFontInfo = params.mpFontInfo;

		const F32 scale = stbtt_ScaleForPixelHeight(pFontInfo, params.mGlyphHeight);

		I32 ascent, descent, lineGap;
		stbtt_GetFontVMetrics(pFontInfo, &ascent, &descent, &lineGap);

		ascent = static_cast<I32>(ascent * scale);
		descent = static_cast<I32>(descent * scale);

		I32 x0, y0, x1, y1;
		stbtt_GetFontBoundingBox(pFontInfo, &x0, &y0, &x1, &y1);

		F32 w = scale * (x1 - x0);
		F32 h = scale * (y1 - y0);

		ITextureAtlas* pTexAtlas = params.mpFontTextureAtlas;
		IFont* pFontResource = params.mpFont;

		E_RESULT_CODE result = RC_OK;

		for (C8 ch : params.mCharacters)
		{
			U8* pBitmap = stbtt_GetCodepointSDF(pFontInfo, scale, ch, 10, 255, 20.0f, &width, &height, &xoff, &yoff);

			if (RC_OK != (result = pTexAtlas->AddRawTexture(std::string(1, ch), width, height, FT_NORM_UBYTE1, pBitmap)))
			{
				TDE2_ASSERT(false);
			}

			stbtt_GetCodepointHMetrics(pFontInfo, ch, &advance, &leftBearing);

			if (RC_OK != (result = pFontResource->AddGlyphInfo(ch, { static_cast<U16>(width), static_cast<U16>(height), static_cast<I16>(xoff), static_cast<I16>(yoff), scale * advance })))
			{
				TDE2_ASSERT(false);
			}
		}

		pTexAtlas->Bake();
	}


	void CConfigWindow::_updateFontsAtlas()
	{
		mpImGUIContext->Button("Update Font Atlas", TVector2(mpImGUIContext->GetWindowWidth() * 0.95f, 25.0f), [this]
		{
			auto fileOpenResult = mpFileSystem->Open<IBinaryFileReader>(mTTFFontFilePath);
			if (fileOpenResult.HasError())
			{
				return;
			}

			auto pFontFile = mpFileSystem->Get<IBinaryFileReader>(fileOpenResult.Get());
			if (!pFontFile)
			{
				return;
			}

			const U32 fileLength = static_cast<U32>(pFontFile->GetFileLength());

			std::unique_ptr<U8[]> pFontBuffer(new U8[fileLength]);

			if (RC_OK != pFontFile->Read(pFontBuffer.get(), fileLength))
			{
				return;
			}

			stbtt_fontinfo font;

			stbtt_InitFont(&font, pFontBuffer.get(), stbtt_GetFontOffsetForIndex(pFontBuffer.get(), 0));

			mFontTextureAtlasId = mpResourceManager->Create<ITextureAtlas>(mFontAtlasName, TDEngine2::TTexture2DParameters(mAtlasWidth, mAtlasHeight, TDEngine2::FT_NORM_UBYTE1));
			if (TResourceId::Invalid == mFontTextureAtlasId)
			{
				TDE2_ASSERT(false);
				return;
			}

			mFontResourceId = mpResourceManager->Create<IFont>(mFontFileName, TFontParameters{});
			if (TResourceId::Invalid == mFontResourceId)
			{
				TDE2_ASSERT(false);
				return;
			}

			GenerateFontTexture(
				{
					mpResourceManager->GetResource<ITextureAtlas>(mFontTextureAtlasId),
					mpResourceManager->GetResource<IFont>(mFontResourceId),
					&font,
					mFontAlphabet,
					mGlyphHeight
				});
		});
	}


	TDE2_API IEditorWindow* CreateConfigWindow(const TConfigWindowParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CConfigWindow, result, params);
	}
}