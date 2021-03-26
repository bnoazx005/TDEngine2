#include "../include/CParticleEditorWindow.h"


namespace TDEngine2
{
	CParticleEditorWindow::CParticleEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CParticleEditorWindow::Init(IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		mpCurveEditor = dynamic_cast<CAnimationCurveEditorWindow*>(CreateAnimationCurveEditorWindow(result));
		if (RC_OK != result)
		{
			return result;
		}

		mpCurveEditor->SetCurveForEditing(nullptr);

		mpResourceManager = pResourceManager;

		mCurrParticleEffectId = TResourceId::Invalid;

		mpCurrParticleEffect = nullptr;

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CParticleEditorWindow::Free()
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

	void CParticleEditorWindow::SetParticleEffectResourceHandle(TResourceId handle)
	{
		mCurrParticleEffectId = handle;
		mpCurrParticleEffect = mpResourceManager->GetResource<IParticleEffect>(mCurrParticleEffectId);
	}

	void CParticleEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(350.0f, 200.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (!mpCurrParticleEffect)
		{
			return;
		}

		if (mpImGUIContext->BeginWindow("Particle Editor", isEnabled, params))
		{
			if (mpImGUIContext->CollapsingHeader("Emitter Settings", true, false))
			{
				_onEmitterSettingsHandle();
			}

			if (mpImGUIContext->CollapsingHeader("Particles Settings", true, false))
			{
				_onParticlesSettingsHandle();
			}

			if (mpImGUIContext->CollapsingHeader("Rendering Settings", true, false))
			{
				_onRenderingSettingsHandle();
			}
		}

		mpImGUIContext->EndWindow();

		if (mpCurveEditor)
		{
			mpCurveEditor->Draw(mpImGUIContext, 0.0f); /// \todo Fix dt passing
		}

		mIsVisible = isEnabled;
	}

	void CParticleEditorWindow::_onEmitterSettingsHandle()
	{
		/// \note Emission Rate
		{
			I32 emissionRate = static_cast<I32>(mpCurrParticleEffect->GetEmissionRate());

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Emission Rate: ");
			mpImGUIContext->IntField("##EmissionRate", emissionRate, [this, &emissionRate] { mpCurrParticleEffect->SetEmissionRate(emissionRate); });
			mpImGUIContext->EndHorizontal();
		}
	}

	void CParticleEditorWindow::_onParticlesSettingsHandle()
	{
		I32 maxParticlesCount = static_cast<I32>(mpCurrParticleEffect->GetMaxParticlesCount());
		
		mpImGUIContext->BeginHorizontal();
		mpImGUIContext->Label("Max Particles: ");
		mpImGUIContext->IntField("##MaxParticles", maxParticlesCount, [this, &maxParticlesCount] { mpCurrParticleEffect->SetMaxParticlesCount(maxParticlesCount); });
		mpImGUIContext->EndHorizontal();

		/// \note Loop mode
		{
			bool isLoopModeEnabled = mpCurrParticleEffect->IsLoopModeActive();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Loop Mode: ");
			mpImGUIContext->Checkbox("##LoopMode", isLoopModeEnabled);
			mpCurrParticleEffect->SetLoopMode(isLoopModeEnabled);
			mpImGUIContext->EndHorizontal();
		}

		// \note Lifetime
		{
			TRange<F32> lifetime = mpCurrParticleEffect->GetLifetime();

			mpImGUIContext->Label("Lifetime: ");

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Min");
			mpImGUIContext->FloatField("##LifetimeMin", lifetime.mLeft, [this, &lifetime] { mpCurrParticleEffect->SetLifetime(lifetime); });
			mpImGUIContext->EndHorizontal();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Max");
			mpImGUIContext->FloatField("##LifetimeMax", lifetime.mRight, [this, &lifetime] { mpCurrParticleEffect->SetLifetime(lifetime); });
			mpImGUIContext->EndHorizontal();
		}

		// \note Initial size
		{
			TRange<F32> initialSize = mpCurrParticleEffect->GetInitialSize();

			mpImGUIContext->Label("Initial Size: ");

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Min");
			mpImGUIContext->FloatField("##SizeMin", initialSize.mLeft, [this, &initialSize] { mpCurrParticleEffect->SetInitialSize(initialSize); });
			mpImGUIContext->EndHorizontal();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Max");
			mpImGUIContext->FloatField("##SizeMax", initialSize.mRight, [this, &initialSize] { mpCurrParticleEffect->SetInitialSize(initialSize); });
			mpImGUIContext->EndHorizontal();
		}

		/// \note Size over lifetime 
		if (mpImGUIContext->CollapsingHeader("Size over Lifetime", true, false))
		{
			mpImGUIContext->Button("Edit Curve", TVector2(100.0f, 25.0f), [this] 
			{
				auto pSizeCurve = mpCurrParticleEffect->GetSizeCurve();
				mpCurveEditor->SetCurveForEditing(pSizeCurve.Get());
				mpCurveEditor->SetVisible(true);
			});
		}
	}

	void CParticleEditorWindow::_onRenderingSettingsHandle()
	{
		/// \note Material name
		{
			std::string materialName = mpCurrParticleEffect->GetMaterialName();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Material Id: ");
			mpImGUIContext->TextField("##MaterialId", materialName, [this, &materialName] { mpCurrParticleEffect->SetMaterialName(materialName); });
		}

		/// \todo Add configuration of material's texture id 
	}


	TDE2_API IEditorWindow* CreateParticleEditorWindow(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CParticleEditorWindow, result, pResourceManager);
	}
}