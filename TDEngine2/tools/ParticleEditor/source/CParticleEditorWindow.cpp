#include "../include/CParticleEditorWindow.h"
#define META_EXPORT_GRAPHICS_SECTION
#include "../../include/metadata.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	std::vector<std::string> CParticleEditorWindow::mColorTypesIds {};
	std::vector<std::string> CParticleEditorWindow::mEmittersTypesIds { "Box", "Sphere", "Cone" };
	std::vector<std::string> CParticleEditorWindow::mSimulationSpaceTypesIds {};
	std::vector<std::string> CParticleEditorWindow::mVelocityParamTypesIds {};
	std::vector<std::string> CParticleEditorWindow::mEmitterShapesTypesIds {};


	CParticleEditorWindow::CParticleEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CParticleEditorWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pResourceManager)
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
		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);

		mpEditorHistory = CreateEditorActionsManager(result);
		if (RC_OK != result)
		{
			return result;
		}

		mCurrParticleEffectId = TResourceId::Invalid;

		mpCurrParticleEffect = nullptr;

		for (auto&& currField : Meta::EnumTrait<E_PARTICLE_COLOR_PARAMETER_TYPE>::fields)
		{
			mColorTypesIds.push_back(currField.name);
		}

		for (auto&& currField : Meta::EnumTrait<E_PARTICLE_SIMULATION_SPACE>::fields)
		{
			mSimulationSpaceTypesIds.push_back(currField.name);
		}

		for (auto&& currField : Meta::EnumTrait<E_PARTICLE_VELOCITY_PARAMETER_TYPE>::fields)
		{
			mVelocityParamTypesIds.push_back(currField.name);
		}

		for (auto&& currEmitterTypeInfo : CBaseParticlesEmitter::GetEmittersTypes())
		{
			mEmitterShapesTypesIds.push_back(std::get<std::string>(currEmitterTypeInfo));
		}

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	void CParticleEditorWindow::SetParticleEffectResourceHandle(TResourceId handle)
	{
		mCurrParticleEffectId = handle;
		mpCurrParticleEffect = mpResourceManager->GetResource<IParticleEffect>(mCurrParticleEffectId);
	}

	E_RESULT_CODE CParticleEditorWindow::ExecuteUndoAction()
	{
		if (!mpEditorHistory)
		{
			return RC_FAIL;
		}

		if (auto actionResult = mpEditorHistory->PopAction())
		{
			return actionResult.Get()->Restore();
		}

		return RC_OK;
	}

	E_RESULT_CODE CParticleEditorWindow::ExecuteRedoAction()
	{
		if (!mpEditorHistory)
		{
			return RC_FAIL;
		}

		if (auto actionResult = mpEditorHistory->PopAction())
		{
			return actionResult.Get()->Execute();
		}

		return RC_OK;
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

		E_RESULT_CODE result = RC_OK;

		// \note process shortcuts		
		if (mpInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
		{
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Z)) // \note Ctrl+Z
			{
				mpEditorHistory->ExecuteUndo();
				mpEditorHistory->Dump();
			}

			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Y)) // \note Ctrl+Y
			{
				mpEditorHistory->ExecuteRedo();
				mpEditorHistory->Dump();
			}
		}

		mIsVisible = isEnabled;
	}


#define STRINGIFY_COMMAND(...) __VA_ARGS__

#define MAKE_COMMAND(historyOwner, command, newValue, oldValue)									\
	do                                                                                          \
	{																							\
		E_RESULT_CODE result = RC_OK;                                                           \
                                                                                                \
		IEditorAction* pAction = CreateCommandAction(                                           \
			[this, value = newValue] { command(value); },                                       \
			[this, value = oldValue] { command(value); },                                       \
			result);                                                                            \
                                                                                                \
		historyOwner->PushAndExecuteAction(pAction);                                            \
	}                                                                                           \
	while (0)


#define MAKE_COMMAND_CAPTURE(historyOwner, command, capture, newValue, oldValue)				\
	do                                                                                          \
	{																							\
		E_RESULT_CODE result = RC_OK;                                                           \
                                                                                                \
		IEditorAction* pAction = CreateCommandAction(                                           \
			[capture, this, value = newValue] { command(value); },                              \
			[capture, this, value = oldValue] { command(value); },                              \
			result);                                                                            \
                                                                                                \
		historyOwner->PushAndExecuteAction(pAction);                                            \
	}                                                                                           \
	while (0)


	void CParticleEditorWindow::_onEmitterSettingsHandle()
	{
		/// \note Emission Rate
		{
			I32 emissionRate = static_cast<I32>(mpCurrParticleEffect->GetEmissionRate());

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Emission Rate: ");
			mpImGUIContext->IntField("##EmissionRate", emissionRate, [this, &emissionRate] 
			{ 
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetEmissionRate), emissionRate, mpCurrParticleEffect->GetEmissionRate());
			});
			mpImGUIContext->EndHorizontal();
		}

		/// \note Simulation space
		{			
			I32 currSimultionSpaceType = static_cast<I32>(mpCurrParticleEffect->GetSimulationSpaceType());
			const I32 prevSimulationSpaceType = currSimultionSpaceType;

			mpImGUIContext->Label("Simulation Space: ");
			currSimultionSpaceType = mpImGUIContext->Popup("##SimSpaceTypePopup", currSimultionSpaceType, mSimulationSpaceTypesIds);

			if (currSimultionSpaceType != prevSimulationSpaceType)
			{
				mpCurrParticleEffect->SetSimulationSpaceType(static_cast<E_PARTICLE_SIMULATION_SPACE>(currSimultionSpaceType));

				MAKE_COMMAND(mpEditorHistory,
					STRINGIFY_COMMAND(mpCurrParticleEffect->SetSimulationSpaceType),
					static_cast<E_PARTICLE_SIMULATION_SPACE>(currSimultionSpaceType),
					static_cast<E_PARTICLE_SIMULATION_SPACE>(prevSimulationSpaceType));

				mpEditorHistory->Dump();
			}			
		}

		/// \note Emitter's shape type
		{
			auto pSharedEmitter = mpCurrParticleEffect->GetSharedEmitter();

			auto&& emittersInfo = CBaseParticlesEmitter::GetEmittersTypes();
			
			auto it = std::find_if(emittersInfo.cbegin(), emittersInfo.cend(), [type = pSharedEmitter->GetEmitterTypeId()](auto&& emitterTypeInfo) 
			{ 
				return std::get<TypeId>(emitterTypeInfo) == type; 
			});

			mpImGUIContext->Label("Emitter Shape: ");

			I32 currTypeIndex = static_cast<I32>(std::distance(emittersInfo.cbegin(), it));
			const I32 newTypeIndex = mpImGUIContext->Popup("##ShapeType", currTypeIndex, mEmitterShapesTypesIds);

			if (currTypeIndex != newTypeIndex)
			{
				MAKE_COMMAND(
					mpEditorHistory, 
					STRINGIFY_COMMAND(mpCurrParticleEffect->SetSharedEmitter), 
					CreateEmitterByTypeId(mpCurrParticleEffect.Get(), std::get<TypeId>(emittersInfo[newTypeIndex])), 
					pSharedEmitter);
			}

			_displayEmitterShapeSettings();
		}
	}

	void CParticleEditorWindow::_onParticlesSettingsHandle()
	{
		/// \note Max particles count
		{
			I32 maxParticlesCount = static_cast<I32>(mpCurrParticleEffect->GetMaxParticlesCount());

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Max Particles: ");
			mpImGUIContext->IntField("##MaxParticles", maxParticlesCount, [this, &maxParticlesCount] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetMaxParticlesCount), maxParticlesCount, mpCurrParticleEffect->GetMaxParticlesCount());
			});
			mpImGUIContext->EndHorizontal();
		}

		/// \note Loop mode
		{
			bool isLoopModeEnabled = mpCurrParticleEffect->IsLoopModeActive();
			const bool prevLoopModeValue = isLoopModeEnabled;

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Loop Mode: ");
			mpImGUIContext->Checkbox("##LoopMode", isLoopModeEnabled);

			if (prevLoopModeValue != isLoopModeEnabled)
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetLoopMode), isLoopModeEnabled, prevLoopModeValue);
			}

			mpImGUIContext->EndHorizontal();
		}

		// \note Lifetime
		{
			TRange<F32> lifetime = mpCurrParticleEffect->GetLifetime();

			mpImGUIContext->Label("Lifetime: ");

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Min");
			mpImGUIContext->FloatField("##LifetimeMin", lifetime.mLeft, [this, &lifetime] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetLifetime), lifetime, mpCurrParticleEffect->GetLifetime());
			});
			mpImGUIContext->EndHorizontal();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Max");
			mpImGUIContext->FloatField("##LifetimeMax", lifetime.mRight, [this, &lifetime] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetLifetime), lifetime, mpCurrParticleEffect->GetLifetime());
			});
			mpImGUIContext->EndHorizontal();
		}

		// \note Initial size
		{
			TRange<F32> initialSize = mpCurrParticleEffect->GetInitialSize();

			mpImGUIContext->Label("Initial Size: ");

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Min");
			mpImGUIContext->FloatField("##SizeMin", initialSize.mLeft, [this, &initialSize] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetInitialSize), initialSize, mpCurrParticleEffect->GetInitialSize());
			});
			mpImGUIContext->EndHorizontal();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Max");
			mpImGUIContext->FloatField("##SizeMax", initialSize.mRight, [this, &initialSize] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetInitialSize), initialSize, mpCurrParticleEffect->GetInitialSize());
			});
			mpImGUIContext->EndHorizontal();
		}

		/// \note Initial rotation
		{
			TRange<F32> initialRotation = mpCurrParticleEffect->GetInitialRotation();

			mpImGUIContext->Label("Initial Rotation: ");

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Min");
			mpImGUIContext->FloatField("##RotationMin", initialRotation.mLeft, [this, &initialRotation] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetInitialRotation), initialRotation, mpCurrParticleEffect->GetInitialRotation());
			});
			mpImGUIContext->EndHorizontal();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Max");
			mpImGUIContext->FloatField("##RotationMax", initialRotation.mRight, [this, &initialRotation] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetInitialRotation), initialRotation, mpCurrParticleEffect->GetInitialRotation());
			});
			mpImGUIContext->EndHorizontal();
		}

		// \note Initial color
		{
			auto colorData = mpCurrParticleEffect->GetInitialColor();

			_drawColorDataModifiers("InitColor", colorData, [this, &colorData]
			{
				//MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetInitialColor), colorData, mpCurrParticleEffect->GetInitialColor());
				mpCurrParticleEffect->SetInitialColor(colorData);
			});
		}

		/// \note Initial move direction and speed factor
		{
			TVector3 moveDirection = mpCurrParticleEffect->GetInitialMoveDirection();
			F32 speedFactor = mpCurrParticleEffect->GetInitialSpeedFactor();

			auto updateInitialVelocityData = [&moveDirection, &speedFactor, this]
			{
				mpCurrParticleEffect->SetInitialVelocityData(moveDirection, speedFactor);
			};

			mpImGUIContext->Label("Initial Move Direction");
			mpImGUIContext->Vector3Field("##InitMoveDir", moveDirection, updateInitialVelocityData);

			mpImGUIContext->Label("Initial Speed Factor");
			mpImGUIContext->FloatField("##InitSpeedFactor", speedFactor, updateInitialVelocityData);
		}

		U32 modifiersFlags = static_cast<U32>(mpCurrParticleEffect->GetEnabledModifiersFlags());

		/// \note Gravity force modifier
		if (mpImGUIContext->CollapsingHeader("Gravity", true, false))
		{
			bool isModifierEnabled = static_cast<bool>(static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(modifiersFlags) & E_PARTICLE_EFFECT_INFO_FLAGS::E_GRAVITY_FORCE_ENABLED);

			mpImGUIContext->Checkbox("Enabled##Gravity", isModifierEnabled);

			if (isModifierEnabled)
			{
				modifiersFlags |= static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_GRAVITY_FORCE_ENABLED);
			}
			else
			{
				modifiersFlags &= ~static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_GRAVITY_FORCE_ENABLED);
			}

			F32 gravityModifier = mpCurrParticleEffect->GetGravityModifier();
			
			mpImGUIContext->Label("Gravity Modifier: ");
			mpImGUIContext->FloatField("##GravityModifier", gravityModifier, [this, &gravityModifier] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetGravityModifier), gravityModifier, mpCurrParticleEffect->GetGravityModifier());
			});
		}

		/// \note Size over lifetime 
		if (mpImGUIContext->CollapsingHeader("Size over Lifetime", true, false))
		{
			bool isModifierEnabled = static_cast<bool>(static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(modifiersFlags) & E_PARTICLE_EFFECT_INFO_FLAGS::E_SIZE_OVER_LIFETIME_ENABLED);

			mpImGUIContext->Checkbox("Enabled##0", isModifierEnabled);

			if (isModifierEnabled)
			{
				modifiersFlags |= static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_SIZE_OVER_LIFETIME_ENABLED);
			}
			else
			{
				modifiersFlags &= ~static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_SIZE_OVER_LIFETIME_ENABLED);
			}

			mpImGUIContext->Button("Edit Curve", TVector2(100.0f, 25.0f), [this] 
			{
				auto pSizeCurve = mpCurrParticleEffect->GetSizeCurve();
				mpCurveEditor->SetCurveForEditing(pSizeCurve.Get());
				mpCurveEditor->SetVisible(true);
			});
		}

		/// \note Color over lifetime 
		if (mpImGUIContext->CollapsingHeader("Color over Lifetime", true, false))
		{
			bool isModifierEnabled = static_cast<bool>(static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(modifiersFlags) & E_PARTICLE_EFFECT_INFO_FLAGS::E_COLOR_OVER_LIFETIME_ENABLED);

			mpImGUIContext->Checkbox("Enabled##1", isModifierEnabled);

			if (isModifierEnabled)
			{
				modifiersFlags |= static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_COLOR_OVER_LIFETIME_ENABLED);
			}
			else
			{
				modifiersFlags &= ~static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_COLOR_OVER_LIFETIME_ENABLED);
			}

			auto colorData = mpCurrParticleEffect->GetColorOverLifeTime();

			_drawColorDataModifiers("ColorOverTime", colorData, [this, &colorData]
			{
				mpCurrParticleEffect->SetColorOverLifeTime(colorData);
			});
		}

		/// \note Velocity over lifetime 
		if (mpImGUIContext->CollapsingHeader("Velocity over Lifetime", true, false))
		{
			bool isModifierEnabled = static_cast<bool>(static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(modifiersFlags) & E_PARTICLE_EFFECT_INFO_FLAGS::E_VELOCITY_OVER_LIFETIME_ENABLED);

			mpImGUIContext->Checkbox("Enabled##VelOvrTm", isModifierEnabled);

			if (isModifierEnabled)
			{
				modifiersFlags |= static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_VELOCITY_OVER_LIFETIME_ENABLED);
			}
			else
			{
				modifiersFlags &= ~static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_VELOCITY_OVER_LIFETIME_ENABLED);
			}

			auto velocityOverTimeData = mpCurrParticleEffect->GetVelocityOverTime();
			
			_drawVelocityDataModifiers(velocityOverTimeData, [this, &velocityOverTimeData]
			{
				mpCurrParticleEffect->SetVelocityOverTime(velocityOverTimeData);
			});
		}

		/// \note Rotation over lifetime 
		if (mpImGUIContext->CollapsingHeader("Rotation over Lifetime", true, false))
		{
			bool isModifierEnabled = static_cast<bool>(static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(modifiersFlags) & E_PARTICLE_EFFECT_INFO_FLAGS::E_ROTATION_OVER_LIFETIME_ENABLED);

			mpImGUIContext->Checkbox("Enabled##RotationOvrTm", isModifierEnabled);

			if (isModifierEnabled)
			{
				modifiersFlags |= static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_ROTATION_OVER_LIFETIME_ENABLED);
			}
			else
			{
				modifiersFlags &= ~static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_ROTATION_OVER_LIFETIME_ENABLED);
			}

			F32 angularSpeed = mpCurrParticleEffect->GetRotationOverTime();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Angular Speed");
			mpImGUIContext->FloatField("##AngularSpeed", angularSpeed, [&angularSpeed, this] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetRotationOverTime), angularSpeed, mpCurrParticleEffect->GetRotationOverTime());
			});
			mpImGUIContext->EndHorizontal();
		}

		/// \note Force over lifetime 
		if (mpImGUIContext->CollapsingHeader("Force over Lifetime", true, false))
		{
			bool isModifierEnabled = static_cast<bool>(static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(modifiersFlags) & E_PARTICLE_EFFECT_INFO_FLAGS::E_FORCE_OVER_LIFETIME_ENABLED);

			mpImGUIContext->Checkbox("Enabled##ForceOvrTm", isModifierEnabled);

			if (isModifierEnabled)
			{
				modifiersFlags |= static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_FORCE_OVER_LIFETIME_ENABLED);
			}
			else
			{
				modifiersFlags &= ~static_cast<U32>(E_PARTICLE_EFFECT_INFO_FLAGS::E_FORCE_OVER_LIFETIME_ENABLED);
			}

			auto forceOverTime = mpCurrParticleEffect->GetForceOverTime();

			mpImGUIContext->Label("Force: ");
			mpImGUIContext->Vector3Field("##Force", forceOverTime, [&forceOverTime, this] 
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetForceOverTime), forceOverTime, mpCurrParticleEffect->GetForceOverTime());
			});
		}

		mpCurrParticleEffect->SetModifiersFlags(static_cast<E_PARTICLE_EFFECT_INFO_FLAGS>(modifiersFlags));
	}

	void CParticleEditorWindow::_onRenderingSettingsHandle()
	{
		/// \note Material name
		{
			std::string materialName = mpCurrParticleEffect->GetMaterialName();

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Material Id: ");
			mpImGUIContext->TextField("##MaterialId", materialName, [this](auto&& value)
			{
				MAKE_COMMAND(mpEditorHistory, STRINGIFY_COMMAND(mpCurrParticleEffect->SetMaterialName), value, mpCurrParticleEffect->GetMaterialName());
			});

			mpImGUIContext->EndHorizontal();
		}

		/// \todo Add configuration of material's texture id 
	}
	
	void CParticleEditorWindow::_drawColorDataModifiers(const std::string& label, TParticleColorParameter& colorData, const std::function<void()>& onChangedAction)
	{
		I32 currSelectedColorType = static_cast<I32>(colorData.mType);

		currSelectedColorType = mpImGUIContext->Popup(Wrench::StringUtils::Format("##{0}", label), currSelectedColorType, mColorTypesIds);
		colorData.mType = static_cast<E_PARTICLE_COLOR_PARAMETER_TYPE>(currSelectedColorType);

		TColor32F colors[2]{ colorData.mFirstColor, colorData.mSecondColor };

		switch (colorData.mType)
		{
			case E_PARTICLE_COLOR_PARAMETER_TYPE::SINGLE_COLOR:

				mpImGUIContext->BeginHorizontal();
				mpImGUIContext->Label("Color");
				mpImGUIContext->ColorPickerField(Wrench::StringUtils::Format("##{0}Color0", label), colors[0], [&colors, &colorData] { colorData.mFirstColor = colors[0]; });
				mpImGUIContext->EndHorizontal();

				break;
			case E_PARTICLE_COLOR_PARAMETER_TYPE::TWEEN_RANDOM:
				mpImGUIContext->BeginHorizontal();
				mpImGUIContext->Label("Color0");
				mpImGUIContext->ColorPickerField(Wrench::StringUtils::Format("##{0}Color0", label), colors[0], [&colors, &colorData] { colorData.mFirstColor = colors[0]; });
				mpImGUIContext->Label("Color1");
				mpImGUIContext->ColorPickerField(Wrench::StringUtils::Format("##{0}Color1", label), colors[1], [&colors, &colorData] { colorData.mSecondColor = colors[1]; });
				mpImGUIContext->EndHorizontal();
				break;

			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_LERP:
			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_RANDOM:
				auto& gradientColor = *colorData.mGradientColor.Get();
				mpImGUIContext->GradientColorPicker("Color", gradientColor);
				break;
		}

		if (onChangedAction)
		{
			onChangedAction();
		}
	}

	void CParticleEditorWindow::_drawVelocityDataModifiers(TParticleVelocityParameter& velocityData, const std::function<void()>& onChangedAction)
	{
		I32 currSelectedVelocityParamType = static_cast<I32>(velocityData.mType);

		currSelectedVelocityParamType = mpImGUIContext->Popup("##VelocityParamType", currSelectedVelocityParamType, mVelocityParamTypesIds);
		velocityData.mType = static_cast<E_PARTICLE_VELOCITY_PARAMETER_TYPE>(currSelectedVelocityParamType);

		TVector3 velocity;

		switch (velocityData.mType)
		{
			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CURVES:
				mpImGUIContext->BeginHorizontal();
				mpImGUIContext->Label("Velocity: ");

				mpImGUIContext->Button("X", TVector2(50.0f, 25.0f), [this, &velocityData]
				{
					auto pCurve = velocityData.mXCurve;
					mpCurveEditor->SetCurveForEditing(pCurve.Get());
					mpCurveEditor->SetVisible(true);
				});

				mpImGUIContext->Button("Y", TVector2(50.0f, 25.0f), [this, &velocityData]
				{
					auto pCurve = velocityData.mYCurve;
					mpCurveEditor->SetCurveForEditing(pCurve.Get());
					mpCurveEditor->SetVisible(true);
				});

				mpImGUIContext->Button("Z", TVector2(50.0f, 25.0f), [this, &velocityData]
				{
					auto pCurve = velocityData.mZCurve;
					mpCurveEditor->SetCurveForEditing(pCurve.Get());
					mpCurveEditor->SetVisible(true);
				});

				mpImGUIContext->EndHorizontal();

				mpImGUIContext->Button("Speed Factor", TVector2(100.0f, 25.0f), [this, &velocityData]
				{
					auto pCurve = velocityData.mSpeedFactorCurve;
					mpCurveEditor->SetCurveForEditing(pCurve.Get());
					mpCurveEditor->SetVisible(true);
				});
				break;

			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CONSTANTS:
				mpImGUIContext->Label("Velocity: ");

				velocity = velocityData.mVelocityConst;
				mpImGUIContext->Vector3Field("##VelocityConst", velocity, [&velocity, &velocityData] { velocityData.mVelocityConst = velocity; });

				mpImGUIContext->BeginHorizontal();
				mpImGUIContext->Label("Speed Factor: ");
				mpImGUIContext->FloatField("##SpeedFactorConst", velocityData.mSpeedFactorConst);
				mpImGUIContext->EndHorizontal();

				break;

			default:
				TDE2_UNREACHABLE();
				break;
		}

		if (onChangedAction)
		{
			onChangedAction();
		}
	}

	void CParticleEditorWindow::_displayEmitterShapeSettings()
	{
		auto pSharedEmitter = mpCurrParticleEffect->GetSharedEmitter();

		/// \note Is 2D emitter
		{
			bool is2DMode = pSharedEmitter->Is2DModeEnabled();
			const bool prev2DModeState = is2DMode;

			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("2D mode");
			mpImGUIContext->Checkbox("##2DMode", is2DMode);
			mpImGUIContext->EndHorizontal();

			if (prev2DModeState != is2DMode)
			{
				MAKE_COMMAND_CAPTURE(mpEditorHistory, STRINGIFY_COMMAND(pSharedEmitter->Set2DMode), pSharedEmitter, is2DMode, prev2DModeState);
			}
		}

		static const std::unordered_map<TypeId, std::function<void()>> typesViewsTable
		{
			{ CBoxParticlesEmitter::GetTypeId(), [this, &pSharedEmitter]
				{
					CBoxParticlesEmitter* pBoxEmitter = dynamic_cast<CBoxParticlesEmitter*>(pSharedEmitter.Get());

					/// \note Box sizes
					{
						auto sizes = pBoxEmitter->GetSizes();

						mpImGUIContext->BeginHorizontal();
						mpImGUIContext->Label("Sizes:");
						mpImGUIContext->Vector3Field("##Sizes", sizes, [this, pBoxEmitter, &sizes]
						{
							MAKE_COMMAND_CAPTURE(mpEditorHistory, STRINGIFY_COMMAND(pBoxEmitter->SetSizes), pBoxEmitter, sizes, pBoxEmitter->GetSizes());
						});
						mpImGUIContext->EndHorizontal();
					}

					/// \note Box origin
					{
						auto origin = pBoxEmitter->GetOrigin();

						mpImGUIContext->BeginHorizontal();
						mpImGUIContext->Label("Origin:");
						mpImGUIContext->Vector3Field("##Origin", origin, [this, pBoxEmitter, &origin]
						{
							MAKE_COMMAND_CAPTURE(mpEditorHistory, STRINGIFY_COMMAND(pBoxEmitter->SetOrigin), pBoxEmitter, origin, pBoxEmitter->GetOrigin());
						});
						mpImGUIContext->EndHorizontal();
					}
				} },
			{ CSphereParticlesEmitter::GetTypeId(), [this, &pSharedEmitter]
				{
					CSphereParticlesEmitter* pSphereEmitter = dynamic_cast<CSphereParticlesEmitter*>(pSharedEmitter.Get());

					/// \note Radius
					{
						auto radius = pSphereEmitter->GetRadius();

						mpImGUIContext->BeginHorizontal();
						mpImGUIContext->Label("Radius:");
						mpImGUIContext->FloatField("##Radius", radius, [this, pSphereEmitter, &radius]
						{
							MAKE_COMMAND_CAPTURE(mpEditorHistory, STRINGIFY_COMMAND(pSphereEmitter->SetRadius), pSphereEmitter, radius, pSphereEmitter->GetRadius());
						});
						mpImGUIContext->EndHorizontal();
					}

					/// \note Sphere origin
					{
						auto origin = pSphereEmitter->GetOrigin();

						mpImGUIContext->BeginHorizontal();
						mpImGUIContext->Label("Origin:");
						mpImGUIContext->Vector3Field("##Origin", origin, [this, pSphereEmitter, &origin]
						{
							MAKE_COMMAND_CAPTURE(mpEditorHistory, STRINGIFY_COMMAND(pSphereEmitter->SetOrigin), pSphereEmitter, origin, pSphereEmitter->GetOrigin());
						});
						mpImGUIContext->EndHorizontal();
					}
				} },
			{ CConeParticlesEmitter::GetTypeId(),  [this, &pSharedEmitter]
				{
					CConeParticlesEmitter* pConeEmitter = dynamic_cast<CConeParticlesEmitter*>(pSharedEmitter.Get());

					/// \note Radius
					{
						auto radius = pConeEmitter->GetRadius();

						mpImGUIContext->BeginHorizontal();
						mpImGUIContext->Label("Radius:");
						mpImGUIContext->FloatField("##Radius", radius, [this, pConeEmitter, &radius]
						{
							MAKE_COMMAND_CAPTURE(mpEditorHistory, STRINGIFY_COMMAND(pConeEmitter->SetRadius), pConeEmitter, radius, pConeEmitter->GetRadius());
						});
						mpImGUIContext->EndHorizontal();
					}

					/// \note Height
					{
						auto height = pConeEmitter->GetHeight();

						mpImGUIContext->BeginHorizontal();
						mpImGUIContext->Label("Height:");
						mpImGUIContext->FloatField("##Height", height, [this, pConeEmitter, &height]
						{
							MAKE_COMMAND_CAPTURE(mpEditorHistory, STRINGIFY_COMMAND(pConeEmitter->SetHeight), pConeEmitter, height, pConeEmitter->GetHeight());
						});
						mpImGUIContext->EndHorizontal();
					}
				} },
		};

		auto it = typesViewsTable.find(pSharedEmitter->GetEmitterTypeId());
		if (it == typesViewsTable.cend())
		{
			TDE2_ASSERT(false);
			return;
		}

		(it->second)();
	}


	TDE2_API IEditorWindow* CreateParticleEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CParticleEditorWindow, result, pResourceManager, pInputContext);
	}
}

#endif