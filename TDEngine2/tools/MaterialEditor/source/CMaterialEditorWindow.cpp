#include "../include/CMaterialEditorWindow.h"
#define META_EXPORT_GRAPHICS_SECTION
#include "metadata.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	std::vector<std::string> BlendFactorTypes;
	std::vector<std::string> BlendOpTypes;
	std::vector<std::string> DSComparisonFunctions;
	std::vector<std::string> StencilOpTypes;


	CMaterialEditorWindow::CMaterialEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CMaterialEditorWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext)
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

		mpResourceManager = pResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);

		mpEditorHistory = CreateEditorActionsManager(result);
		if (RC_OK != result)
		{
			return result;
		}

		mCurrMaterialId = TResourceId::Invalid;
		mpCurrMaterial = nullptr;

		BlendFactorTypes.clear();
		for (auto&& currFieldInfo : Meta::EnumTrait<E_BLEND_FACTOR_VALUE>::fields)
		{
			BlendFactorTypes.push_back(currFieldInfo.name);
		}

		BlendOpTypes.clear();
		for (auto&& currFieldInfo : Meta::EnumTrait<E_BLEND_OP_TYPE>::fields)
		{
			BlendOpTypes.push_back(currFieldInfo.name);
		}

		DSComparisonFunctions.clear();
		for (auto&& currFieldInfo : Meta::EnumTrait<E_COMPARISON_FUNC>::fields)
		{
			DSComparisonFunctions.push_back(currFieldInfo.name);
		}

		StencilOpTypes.clear();
		for (auto&& currFieldInfo : Meta::EnumTrait<E_STENCIL_OP>::fields)
		{
			StencilOpTypes.push_back(currFieldInfo.name);
		}

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	void CMaterialEditorWindow::SetMaterialResourceHandle(TResourceId handle)
	{
		mCurrMaterialId = handle;
		mpCurrMaterial = mpResourceManager->GetResource<IMaterial>(mCurrMaterialId);
	}

	E_RESULT_CODE CMaterialEditorWindow::ExecuteUndoAction()
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

	E_RESULT_CODE CMaterialEditorWindow::ExecuteRedoAction()
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


	static void DrawTexturesSlotsGroup(IImGUIContext& imguiContext, TPtr<IMaterial> pMaterial)
	{
		if (!imguiContext.CollapsingHeader("Textures Slots", true, false))
		{
			return;
		}

		pMaterial->ForEachTextureSlot([&imguiContext](const std::string& slotName, ITexture* pTextureResource)
		{
			IResource* pResource = dynamic_cast<IResource*>(pTextureResource);

			if (std::get<0>(imguiContext.BeginTreeNode(slotName)))
			{
				std::string textureResourceStr = pResource ? pResource->GetName() : Wrench::StringUtils::GetEmptyStr();

				imguiContext.BeginHorizontal();
				{
					imguiContext.Label("Path:");
					imguiContext.TextField(Wrench::StringUtils::Format("##{0}_path", slotName), textureResourceStr, [](auto&& value)
					{
						/// \note Try to load a texture and if it's successfull assign it to the slot
					});
				}
				imguiContext.EndHorizontal();

				/// \note Texture's preview

				if (pResource) 
				{
					imguiContext.Image(pResource->GetId(), TVector2(imguiContext.GetWindowWidth() * 0.25f), pTextureResource->GetNormalizedTextureRect());
				}
				else
				{
					static constexpr F32 invalidImageSizes = 128.0f;

					auto&& cursorPos = imguiContext.GetCursorScreenPos();
					imguiContext.DrawRect(TRectF32(cursorPos.x, cursorPos.y, invalidImageSizes, invalidImageSizes), TColorUtils::mBlack, true);
					imguiContext.DrawText(cursorPos + TVector2(invalidImageSizes * 0.25f, invalidImageSizes * 0.5f), TColorUtils::mWhite, "NO IMAGE");
					imguiContext.SetCursorScreenPos(cursorPos + TVector2(invalidImageSizes));
				}

				imguiContext.EndTreeNode();
			}
		});
	}


	static const std::unordered_map<TypeId, std::function<void(IImGUIContext&, TPtr<IMaterial>, const std::string&, const void*)>> PropertyDrawers
	{
		{ 
			TDE2_TYPE_ID(F32), 
			[](IImGUIContext& imgui, TPtr<IMaterial> pMaterial, const std::string& variableId, const void* pData)
			{
				F32 value = *static_cast<const F32*>(pData);

				imgui.FloatField(Wrench::StringUtils::Format("##{0}", variableId), value, [pMaterial, &variableId, &value]
				{
					pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, variableId, value);
				});
			}
		},

		{
			TDE2_TYPE_ID(U32),
			[](IImGUIContext& imgui, TPtr<IMaterial> pMaterial, const std::string& variableId, const void* pData)
			{
				I32 value = *static_cast<const I32*>(pData);

				imgui.IntField(Wrench::StringUtils::Format("##{0}", variableId), value, [pMaterial, &variableId, &value]
				{
					pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, variableId, value < 0 ? 0 : value);
				});
			}
		},

		{
			TDE2_TYPE_ID(TVector2),
			[](IImGUIContext& imgui, TPtr<IMaterial> pMaterial, const std::string& variableId, const void* pData)
			{
				TVector2 value = *static_cast<const TVector2*>(pData);

				imgui.Vector2Field(Wrench::StringUtils::Format("##{0}", variableId), value, [pMaterial, &variableId, &value]
				{
					pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, variableId, value);
				});
			}
		},

		{
			TDE2_TYPE_ID(TVector3),
			[](IImGUIContext& imgui, TPtr<IMaterial> pMaterial, const std::string& variableId, const void* pData)
			{
				TVector3 value = *static_cast<const TVector3*>(pData);

				imgui.Vector3Field(Wrench::StringUtils::Format("##{0}", variableId), value, [pMaterial, &variableId, &value]
				{
					pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, variableId, value);
				});
			}
		},

		{
			TDE2_TYPE_ID(TVector4),
			[](IImGUIContext& imgui, TPtr<IMaterial> pMaterial, const std::string& variableId, const void* pData)
			{
				TVector4 value = *static_cast<const TVector4*>(pData);

				imgui.Vector4Field(Wrench::StringUtils::Format("##{0}", variableId), value, [pMaterial, &variableId, &value]
				{
					pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, variableId, value);
				});
			}
		},

		{
			TDE2_TYPE_ID(TColor32F),
			[](IImGUIContext& imgui, TPtr<IMaterial> pMaterial, const std::string& variableId, const void* pData)
			{
				TColor32F value = *static_cast<const TColor32F*>(pData);

				imgui.ColorPickerField(Wrench::StringUtils::Format("##{0}", variableId), value, [pMaterial, &variableId, &value]
				{
					pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, variableId, value);
				});
			}
		},
	};



	static void DrawVariablesGroup(IImGUIContext& imguiContext, TPtr<IMaterial> pMaterial)
	{
		if (!imguiContext.CollapsingHeader("Variables", true, false))
		{
			return;
		}

		pMaterial->ForEachVariable([&imguiContext, pMaterial](const TShaderUniformDesc& variableInfo, const void* pData)
		{
			if (variableInfo.mIsArray)
			{
				return; /// \note Arrays preview isn't supported for now
			}

			imguiContext.BeginHorizontal();
			imguiContext.Label(variableInfo.mName);

			auto it = PropertyDrawers.find(variableInfo.mTypeId);
			if (it != PropertyDrawers.cend())
			{
				(it->second)(imguiContext, pMaterial, variableInfo.mName, pData);
			}

			imguiContext.EndHorizontal();
		});
	}


	static void DrawBlendingConfigsGroup(IImGUIContext& imguiContext, TPtr<IMaterial> pMaterial)
	{
		if (!imguiContext.CollapsingHeader("Blending Params", true, false))
		{
			return;
		}

		TBlendStateDesc currBlengingParams = pMaterial->GetBlendingParams();

		imguiContext.BeginHorizontal();
		imguiContext.Label("Is Blending Enabled");

		if (imguiContext.Checkbox("##IsBlendingEnabled", currBlengingParams.mIsEnabled))
		{
			pMaterial->SetTransparentState(currBlengingParams.mIsEnabled);
		}

		imguiContext.EndHorizontal();

		// Src blend factor
		imguiContext.BeginHorizontal();
		imguiContext.Label("Src ");

		currBlengingParams.mScrValue = static_cast<E_BLEND_FACTOR_VALUE>(imguiContext.Popup("##SrcFactor", static_cast<I32>(currBlengingParams.mScrValue), BlendFactorTypes));

		imguiContext.EndHorizontal();

		// src alpha blend factor
		imguiContext.BeginHorizontal();
		imguiContext.Label("Src Alpha ");

		currBlengingParams.mScrAlphaValue = static_cast<E_BLEND_FACTOR_VALUE>(imguiContext.Popup("##SrcAlphaFactor", static_cast<I32>(currBlengingParams.mScrAlphaValue), BlendFactorTypes));

		imguiContext.EndHorizontal();

		// dest blend factor
		imguiContext.BeginHorizontal();
		imguiContext.Label("Dest ");

		currBlengingParams.mDestValue = static_cast<E_BLEND_FACTOR_VALUE>(imguiContext.Popup("##DstFactor", static_cast<I32>(currBlengingParams.mDestValue), BlendFactorTypes));

		imguiContext.EndHorizontal();

		// dest alpha blend factor
		imguiContext.BeginHorizontal();
		imguiContext.Label("Dest Alpha ");

		currBlengingParams.mDestAlphaValue = static_cast<E_BLEND_FACTOR_VALUE>(imguiContext.Popup("##DstAlphaFactor", static_cast<I32>(currBlengingParams.mDestAlphaValue), BlendFactorTypes));

		imguiContext.EndHorizontal();

		// op type
		imguiContext.BeginHorizontal();
		imguiContext.Label("Op Type ");

		currBlengingParams.mOpType = static_cast<E_BLEND_OP_TYPE>(imguiContext.Popup("##OpType", static_cast<I32>(currBlengingParams.mOpType), BlendOpTypes));

		imguiContext.EndHorizontal();

		// alpha op type
		imguiContext.BeginHorizontal();
		imguiContext.Label("Alpha Op Type ");

		currBlengingParams.mAlphaOpType = static_cast<E_BLEND_OP_TYPE>(imguiContext.Popup("##OpAlphaType", static_cast<I32>(currBlengingParams.mAlphaOpType), BlendOpTypes));

		imguiContext.EndHorizontal();

		pMaterial->SetBlendFactors(currBlengingParams.mScrValue, currBlengingParams.mDestValue, currBlengingParams.mScrAlphaValue, currBlengingParams.mDestAlphaValue);
		pMaterial->SetBlendOp(currBlengingParams.mOpType, currBlengingParams.mAlphaOpType);
	}


	static void DrawDepthConfigGroup(IImGUIContext& imguiContext, TPtr<IMaterial> pMaterial)
	{
		TDepthStencilStateDesc depthStencilConfig = pMaterial->GetDepthStencilParams();

		// Test function
		imguiContext.BeginHorizontal();
		imguiContext.Label("Compare Func ");

		depthStencilConfig.mDepthCmpFunc = static_cast<E_COMPARISON_FUNC>(imguiContext.Popup("##DepthCmpFunc", static_cast<I32>(depthStencilConfig.mDepthCmpFunc), DSComparisonFunctions));

		imguiContext.EndHorizontal();

		pMaterial->SetDepthComparisonFunc(depthStencilConfig.mDepthCmpFunc);
	}


	static void DrawStencilConfigGroup(const std::string& groupId, IImGUIContext& imguiContext, TPtr<IMaterial> pMaterial, TStencilOpDesc& opDesc)
	{		
		if (!std::get<0>(imguiContext.BeginTreeNode(groupId)))
		{			
			return;
		}

		// Test function
		imguiContext.BeginHorizontal();
		imguiContext.Label("Compare Func ");
		opDesc.mFunc = static_cast<E_COMPARISON_FUNC>(imguiContext.Popup(Wrench::StringUtils::Format("{0}_CmpFunc", groupId), static_cast<I32>(opDesc.mFunc), DSComparisonFunctions));
		imguiContext.EndHorizontal();

		// Keep Op
		imguiContext.BeginHorizontal();
		imguiContext.Label("On Pass ");
		opDesc.mPassOp = static_cast<E_STENCIL_OP>(imguiContext.Popup(Wrench::StringUtils::Format("{0}_OnPass", groupId), static_cast<I32>(opDesc.mPassOp), StencilOpTypes));
		imguiContext.EndHorizontal();

		// Fail Op
		imguiContext.BeginHorizontal();
		imguiContext.Label("On Fail ");
		opDesc.mFailOp = static_cast<E_STENCIL_OP>(imguiContext.Popup(Wrench::StringUtils::Format("{0}_OnFail", groupId), static_cast<I32>(opDesc.mFailOp), StencilOpTypes));
		imguiContext.EndHorizontal();

		// Keep Op
		imguiContext.BeginHorizontal();
		imguiContext.Label("On Depth Fail ");
		opDesc.mDepthFailOp = static_cast<E_STENCIL_OP>(imguiContext.Popup(Wrench::StringUtils::Format("{0}_OnDepthFail", groupId), static_cast<I32>(opDesc.mDepthFailOp), StencilOpTypes));
		imguiContext.EndHorizontal();

		imguiContext.EndTreeNode();
	}


	static void DrawStencilConfigGroup(IImGUIContext& imguiContext, TPtr<IMaterial> pMaterial)
	{
		TDepthStencilStateDesc depthStencilConfig = pMaterial->GetDepthStencilParams();

		// Stencil Ref
		imguiContext.BeginHorizontal();
		imguiContext.Label("Stencil Ref ");

		I32 refValue = static_cast<I32>(depthStencilConfig.mStencilRefValue);
		imguiContext.IntField("##StencilRef", refValue, [&refValue, pMaterial]
		{
			pMaterial->SetStencilRefValue(refValue);
		});

		imguiContext.EndHorizontal();

		// Stencil Read Mask
		imguiContext.BeginHorizontal();
		imguiContext.Label("Read Mask ");

		I32 readMask = static_cast<I32>(depthStencilConfig.mStencilReadMaskValue);
		imguiContext.IntField("##StencilReadMask", readMask, [&readMask, pMaterial]
		{
			pMaterial->SetStencilReadMask(readMask);
		});

		imguiContext.EndHorizontal();

		// Stencil Write Mask
		imguiContext.BeginHorizontal();
		imguiContext.Label("Write Mask ");

		I32 writeMask = static_cast<I32>(depthStencilConfig.mStencilWriteMaskValue);
		imguiContext.IntField("##StencilWriteMask", writeMask, [&writeMask, pMaterial]
		{
			pMaterial->SetStencilWriteMask(writeMask);
		});

		imguiContext.EndHorizontal();

		DrawStencilConfigGroup("Stencil Front Op", imguiContext, pMaterial, depthStencilConfig.mStencilFrontFaceOp);
		pMaterial->SetStencilFrontOp(depthStencilConfig.mStencilFrontFaceOp);

		DrawStencilConfigGroup("Stencil Back Op", imguiContext, pMaterial, depthStencilConfig.mStencilBackFaceOp);
		pMaterial->SetStencilBackOp(depthStencilConfig.mStencilBackFaceOp);
	}


	static void DrawDepthStencilConfigGroup(IImGUIContext& imguiContext, TPtr<IMaterial> pMaterial)
	{
		if (!imguiContext.CollapsingHeader("Depth-Stencil Config", true, false))
		{
			return;
		}

		TDepthStencilStateDesc depthStencilConfig = pMaterial->GetDepthStencilParams();

		// Depth write
		imguiContext.BeginHorizontal();
		imguiContext.Label("Depth Write");
		imguiContext.Checkbox("##IsDepthWriteEnabled", depthStencilConfig.mIsDepthWritingEnabled);
		imguiContext.EndHorizontal();

		pMaterial->SetDepthWriteEnabled(depthStencilConfig.mIsDepthWritingEnabled);

		// Depth Test
		imguiContext.BeginHorizontal();
		imguiContext.Label("Depth Test");
		imguiContext.Checkbox("##IsDepthTestEnabled", depthStencilConfig.mIsDepthTestEnabled);
		imguiContext.EndHorizontal();

		pMaterial->SetDepthBufferEnabled(depthStencilConfig.mIsDepthTestEnabled);

		if (depthStencilConfig.mIsDepthTestEnabled)
		{
			DrawDepthConfigGroup(imguiContext, pMaterial);
		}

		// Stencil 
		imguiContext.BeginHorizontal();
		imguiContext.Label("Stencil Test");
		imguiContext.Checkbox("##IsStencilTestEnabled", depthStencilConfig.mIsStencilTestEnabled);
		imguiContext.EndHorizontal();

		pMaterial->SetStencilBufferEnabled(depthStencilConfig.mIsStencilTestEnabled);

		if (depthStencilConfig.mIsStencilTestEnabled)
		{
			DrawStencilConfigGroup(imguiContext, pMaterial);
		}
	}


	void CMaterialEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(350.0f, 200.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (!mpCurrMaterial)
		{
			return;
		}

		if (mpImGUIContext->BeginWindow("Material Editor", isEnabled, params))
		{
			std::string shaderId = mpCurrMaterial->GetShaderId();
			
			mpImGUIContext->BeginHorizontal();
			mpImGUIContext->Label("Shader: ");
			mpImGUIContext->TextField("##Shader", shaderId, [this](auto&& value)
			{
				mpCurrMaterial->SetShader(value);
			});
			mpImGUIContext->EndHorizontal();

			DrawTexturesSlotsGroup(*mpImGUIContext, mpCurrMaterial);
			DrawVariablesGroup(*mpImGUIContext, mpCurrMaterial);
			DrawBlendingConfigsGroup(*mpImGUIContext, mpCurrMaterial);
			DrawDepthStencilConfigGroup(*mpImGUIContext, mpCurrMaterial);
		}

		mpImGUIContext->EndWindow();

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


	TDE2_API IEditorWindow* CreateMaterialEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CMaterialEditorWindow, result, pResourceManager, pInputContext);
	}
}

#endif