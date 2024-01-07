#include "../../include/core/CGameUserSettings.h"
#include "../../include/platform/CConfigFileReader.h"
#include "../../include/platform/CConfigFileWriter.h"
#include "stringUtils.hpp"
#include "variant.hpp"
#include <mutex>


namespace TDEngine2
{
	struct TUserSettingsKeys
	{
		static const std::string mMainGroupKeyId;
		static const std::string mGraphicsGroupKeyId;

		static const std::string mWidthKeyId;
		static const std::string mHeightKeyId;
		static const std::string mIsFullscreenEnabledKeyId;
		static const std::string mShadowsKeyId;
		static const std::string mShadowMapSizesKeyId;
		static const std::string mShadowCascadesSplitsKeyId;
		static const std::string mShadowCascadesCountKeyId;
	};


	const std::string TUserSettingsKeys::mMainGroupKeyId = "main";
	const std::string TUserSettingsKeys::mGraphicsGroupKeyId = "graphics";

	const std::string TUserSettingsKeys::mWidthKeyId = "width";
	const std::string TUserSettingsKeys::mHeightKeyId = "height";
	const std::string TUserSettingsKeys::mIsFullscreenEnabledKeyId = "fullscreen";
	const std::string TUserSettingsKeys::mShadowCascadesSplitsKeyId = "shadow_cascades_splits.{0}";
	const std::string TUserSettingsKeys::mShadowCascadesCountKeyId = "shadow_cascades_count";
	const std::string TUserSettingsKeys::mShadowsKeyId = "shadows";
	const std::string TUserSettingsKeys::mShadowMapSizesKeyId = "shadow_map_size";


	template <typename T> struct TConvertToVariableType { static const E_CONSOLE_VARIABLE_TYPE mType = E_CONSOLE_VARIABLE_TYPE::INT; };
	template <> struct TConvertToVariableType<F32> { static const E_CONSOLE_VARIABLE_TYPE mType = E_CONSOLE_VARIABLE_TYPE::FLOAT; };
	template <>	struct TConvertToVariableType<std::string> { static const E_CONSOLE_VARIABLE_TYPE mType = E_CONSOLE_VARIABLE_TYPE::STRING; };


	static std::string PrepareIdentifierStr(const std::string& name);


	/*!
		\brief CConsoleVariablesStorage' definition
	*/

	class CConsoleVariablesStorage
	{
		public:
			typedef Wrench::Variant<I32, F32, std::string> TConsoleVariableValue;
		public:
			CConsoleVariablesStorage()
			{
				mVariablesPerGroupTable[mDefaultGroupId] = {};
			}

			E_RESULT_CODE Load(IConfigFileReader* pConfigFileReader)
			{
				return RC_OK;
			}

			E_RESULT_CODE Save(IConfigFileWriter* pConfigFileWriter)
			{
				E_RESULT_CODE result = RC_OK;

				static const std::unordered_map<E_CONSOLE_VARIABLE_TYPE, std::function<E_RESULT_CODE(const std::string&, const std::string&, const TConsoleVariableValue&)>> handlers
				{
					{ E_CONSOLE_VARIABLE_TYPE::INT, [pConfigFileWriter](const std::string& group, const std::string& name, const TConsoleVariableValue& value) { return pConfigFileWriter->SetInt(group, name, value.As<I32>()); } },
					{ E_CONSOLE_VARIABLE_TYPE::FLOAT, [pConfigFileWriter](const std::string& group, const std::string& name, const TConsoleVariableValue& value) { return pConfigFileWriter->SetFloat(group, name, value.As<F32>()); } },
					{ E_CONSOLE_VARIABLE_TYPE::STRING, [pConfigFileWriter](const std::string& group, const std::string& name, const TConsoleVariableValue& value) { return pConfigFileWriter->SetString(group, name, value.As<std::string>()); } },
				};

				for (auto&& currGroupEntry : mVariablesPerGroupTable)
				{
					for (auto&& currVariableEntry : currGroupEntry.second)
					{
						auto&& variableInfo = currVariableEntry.second;

						if (E_CONSOLE_VARIABLE_FLAGS::SYSTEM == (variableInfo.mFlags & E_CONSOLE_VARIABLE_FLAGS::SYSTEM))
						{
							continue;
						}

						result = result | handlers.at(variableInfo.mType)(currGroupEntry.first, currVariableEntry.first, mValues[variableInfo.mHandle]);
					}
				}

				return RC_OK;
			}

			template <typename T>
			TResult<TUserConfigVariableInfo*> CreateVariable(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const T& initialValue)
			{
				std::lock_guard<std::mutex> lock(mMutex);

				if (_getVariableInfoInternal(name))
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Variable with specified name already exists");
					return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
				}

				auto&& identifiers = _parseIdentifier(name);
				if (identifiers.size() != 2)
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Invalid number of tokens in cvar's identifier");
					return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
				}

				TUserConfigVariableInfo variableInfo{};
				variableInfo.mGroup = identifiers.front();
				variableInfo.mName = identifiers.back();
				variableInfo.mDescription = description;
				variableInfo.mFlags = flags;
				variableInfo.mType = TConvertToVariableType<T>::mType;
				variableInfo.mHandle = mValues.size();

				mVariablesPerGroupTable[variableInfo.mGroup].emplace(variableInfo.mName, variableInfo);

				mIdentifiersToVariablesHashTable[PrepareIdentifierStr(name)] = &mVariablesPerGroupTable[variableInfo.mGroup][variableInfo.mName];

				TConsoleVariableValue value;
				value = initialValue;

				mValues.emplace_back(value);

				return Wrench::TOkValue< TUserConfigVariableInfo*>(&mVariablesPerGroupTable[variableInfo.mGroup][variableInfo.mName]);
			}

			template <typename T>
			E_RESULT_CODE SetVariable(const std::string& name, const T& value)
			{
				std::lock_guard<std::mutex> lock(mMutex);

				auto it = mIdentifiersToVariablesHashTable.find(name);
				if (it == mIdentifiersToVariablesHashTable.cend())
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Variable with specified name doesn't exist");
					return RC_INVALID_ARGS;
				}

				auto pVariableInfo = it->second;
				if (TConvertToVariableType<T>::mType != pVariableInfo->mType)
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Types mismatch");
					return RC_FAIL;
				}

				mValues[pVariableInfo->mHandle] = value;

				return RC_OK;
			}

			template <typename T>
			const T* GetNumericVariable(const std::string& name) const
			{
				std::lock_guard<std::mutex> lock(mMutex);

				auto pVariableInfo = _getVariableInfoIfAllowed<T>(name);
				if (!pVariableInfo)
				{
					return nullptr;
				}

				return &mValues[pVariableInfo->mHandle].template As<T>();
			}

			const std::string& GetStringVariable(const std::string& name) const
			{
				std::lock_guard<std::mutex> lock(mMutex);

				auto pVariableInfo = _getVariableInfoIfAllowed<std::string>(name);
				if (!pVariableInfo)
				{
					return Wrench::StringUtils::GetEmptyStr();
				}

				return mValues[pVariableInfo->mHandle].As<std::string>();
			}

			const TUserConfigVariableInfo* GetVariableInfo(const std::string& name) const
			{
				std::lock_guard<std::mutex> lock(mMutex);
				return _getVariableInfoInternal(name);
			}
		private:
			const TUserConfigVariableInfo* _getVariableInfoInternal(const std::string& name) const
			{
				auto it = mIdentifiersToVariablesHashTable.find(name);
				if (it != mIdentifiersToVariablesHashTable.cend())
				{
					return it->second;
				}

				auto&& identifiers = _parseIdentifier(name);
				if (identifiers.size() != 2)
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Invalid number of tokens in cvar's identifier");
					return nullptr;
				}

				auto&& groupIt = mVariablesPerGroupTable.find(identifiers.front());
				if (groupIt == mVariablesPerGroupTable.cend())
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Specified group wasn't found");
					return nullptr;
				}

				auto&& variableIt = groupIt->second.find(identifiers.back());
				if (variableIt == groupIt->second.cend())
				{
					return nullptr;
				}

				return &variableIt->second;
			}

			template <typename T>
			TUserConfigVariableInfo* _getVariableInfoIfAllowed(const std::string& name) const
			{
				auto it = mIdentifiersToVariablesHashTable.find(name);
				if (it == mIdentifiersToVariablesHashTable.cend())
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Variable with specified name doesn't exist");
					return nullptr;
				}

				auto pVariableInfo = it->second;
				if (TConvertToVariableType<T>::mType != pVariableInfo->mType)
				{
					TDE2_ASSERT_MSG(false, "[CConsoleVariablesStorage] Types mismatch");
					return nullptr;
				}

				return pVariableInfo;
			}

			std::vector<std::string> _parseIdentifier(const std::string& compoundName) const
			{
				auto&& identifiers = Wrench::StringUtils::Split(compoundName, mDefaultIdentifierSeparator);
				if (identifiers.size() == 1)
				{
					identifiers.emplace(identifiers.begin(), mDefaultGroupId);
				}

				return identifiers;
			}
		public:
			static const std::string mDefaultGroupId;
			static const std::string mDefaultIdentifierSeparator;

		private:
			mutable std::mutex mMutex;
		
			std::unordered_map<std::string, std::unordered_map<std::string, TUserConfigVariableInfo>> mVariablesPerGroupTable;
			std::unordered_map<std::string, TUserConfigVariableInfo*> mIdentifiersToVariablesHashTable; ///< Converts string path to index within mVariablesPerGroupTable
			std::vector<TConsoleVariableValue> mValues;
	};


	const std::string CConsoleVariablesStorage::mDefaultGroupId = "Locals";
	const std::string CConsoleVariablesStorage::mDefaultIdentifierSeparator = ".";


	static std::string PrepareIdentifierStr(const std::string& name)
	{
		return name.find_first_of(CConsoleVariablesStorage::mDefaultIdentifierSeparator) == std::string::npos ? (CConsoleVariablesStorage::mDefaultGroupId + CConsoleVariablesStorage::mDefaultIdentifierSeparator + name) : name;
	}


	/*!
		\brief CGameUserSettings' definition
	*/

	CGameUserSettings::CGameUserSettings():
		CBaseObject(), mpCVarsStorage(std::make_unique<CConsoleVariablesStorage>())
	{
	}

	CGameUserSettings::~CGameUserSettings()
	{
	}

#if TDE2_EDITORS_ENABLED

	void CGameUserSettings::RegisterObjectInProfiler(const std::string& id)
	{
	}

	void CGameUserSettings::OnBeforeMemoryRelease()
	{
	}

#endif


	E_RESULT_CODE CGameUserSettings::Init(IConfigFileReader* pConfigFileReader)
	{
		if (!pConfigFileReader)
		{
			return RC_INVALID_ARGS;
		}

		mWindowWidth  = pConfigFileReader->GetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mWidthKeyId, 640);
		mWindowHeight = pConfigFileReader->GetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mHeightKeyId, 480);
	
		mIsFullscreenEnabled = pConfigFileReader->GetBool(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mIsFullscreenEnabledKeyId, false);

		mCurrent.mIsShadowMappingEnabled = pConfigFileReader->GetBool(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowsKeyId, true);
		mCurrent.mShadowMapSizes = static_cast<U32>(pConfigFileReader->GetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowMapSizesKeyId, 512));
		mCurrent.mShadowCascadesCount = static_cast<U32>(pConfigFileReader->GetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowCascadesCountKeyId, 3));

		mCurrent.mShadowCascadesSplits.x = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 0), 0.020f));
		mCurrent.mShadowCascadesSplits.y = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 1), 0.045f));
		mCurrent.mShadowCascadesSplits.z = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 2), 0.08f));
		mCurrent.mShadowCascadesSplits.w = static_cast<F32>(pConfigFileReader->GetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 3), 1.0f));

		return mpCVarsStorage->Load(pConfigFileReader);
	}

	E_RESULT_CODE CGameUserSettings::Save(IConfigFileWriter* pConfigFileWriter)
	{
		if (!pConfigFileWriter)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mWidthKeyId, mWindowWidth);
		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mMainGroupKeyId, TUserSettingsKeys::mHeightKeyId, mWindowHeight);

		result = result | pConfigFileWriter->SetBool(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowsKeyId, mCurrent.mIsShadowMappingEnabled);
		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowMapSizesKeyId, mCurrent.mShadowMapSizes);
		result = result | pConfigFileWriter->SetInt(TUserSettingsKeys::mGraphicsGroupKeyId, TUserSettingsKeys::mShadowCascadesCountKeyId, mCurrent.mShadowCascadesCount);

		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 0), mCurrent.mShadowCascadesSplits.x);
		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 1), mCurrent.mShadowCascadesSplits.y);
		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 2), mCurrent.mShadowCascadesSplits.z);
		result = result | pConfigFileWriter->SetFloat(TUserSettingsKeys::mGraphicsGroupKeyId, Wrench::StringUtils::Format(TUserSettingsKeys::mShadowCascadesSplitsKeyId, 3), mCurrent.mShadowCascadesSplits.w);

		result = result | mpCVarsStorage->Save(pConfigFileWriter);

		return result;
	}

	TDE2_API TPtr<CGameUserSettings> CGameUserSettings::Get()
	{
		static TPtr<CGameUserSettings> pInstance = TPtr<CGameUserSettings>(new (std::nothrow) CGameUserSettings());
		return pInstance;
	}

	E_RESULT_CODE CGameUserSettings::CreateInt32Variable(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const I32& initialValue)
	{
		auto result = mpCVarsStorage->CreateVariable(name, description, flags, initialValue);
		return result.IsOk() ? RC_OK : result.GetError();
	}

	E_RESULT_CODE CGameUserSettings::CreateFloatVariable(const std::string & name, const std::string & description, E_CONSOLE_VARIABLE_FLAGS flags, const F32 & initialValue)
	{
		auto result = mpCVarsStorage->CreateVariable(name, description, flags, initialValue);
		return result.IsOk() ? RC_OK : result.GetError();
	}

	E_RESULT_CODE CGameUserSettings::CreateStringVariable(const std::string & name, const std::string & description, E_CONSOLE_VARIABLE_FLAGS flags, const std::string & initialValue)
	{
		auto result = mpCVarsStorage->CreateVariable(name, description, flags, initialValue);
		return result.IsOk() ? RC_OK : result.GetError();
	}

	E_RESULT_CODE CGameUserSettings::SetInt32Variable(const std::string& name, I32 value)
	{
		return mpCVarsStorage->SetVariable(PrepareIdentifierStr(name), value);
	}

	E_RESULT_CODE CGameUserSettings::SetFloatVariable(const std::string& name, F32 value)
	{
		return mpCVarsStorage->SetVariable(PrepareIdentifierStr(name), value);
	}

	E_RESULT_CODE CGameUserSettings::SetStringVariable(const std::string& name, const std::string& value)
	{
		return mpCVarsStorage->SetVariable(PrepareIdentifierStr(name), value);
	}

	const I32* CGameUserSettings::GetInt32Variable(const std::string& name) const
	{
		return mpCVarsStorage->GetNumericVariable<I32>(PrepareIdentifierStr(name));
	}

	const F32* CGameUserSettings::GetFloatVariable(const std::string& name) const
	{
		return mpCVarsStorage->GetNumericVariable<F32>(PrepareIdentifierStr(name));
	}
	
	const std::string& CGameUserSettings::GetStringVariable(const std::string& name) const
	{
		return mpCVarsStorage->GetStringVariable(PrepareIdentifierStr(name));
	}

	const TUserConfigVariableInfo* CGameUserSettings::GetVariableInfo(const std::string& name) const
	{
		return mpCVarsStorage->GetVariableInfo(PrepareIdentifierStr(name));
	}


	/*!
		\brief CInt32ConsoleVarDecl's definition
	*/

	CInt32ConsoleVarDecl::CInt32ConsoleVarDecl(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const I32& initialValue,
		const TSetterType& setterCallback, const TGetterType& getterCallback):
		mSetterCallback(setterCallback), mGetterCallback(getterCallback)
	{
		E_RESULT_CODE result = CGameUserSettings::Get()->CreateInt32Variable(name, description, flags, initialValue);
		TDE2_ASSERT(RC_OK == result);

		mId = PrepareIdentifierStr(name);
	}

	void CInt32ConsoleVarDecl::Set(I32 value)
	{
		if (mSetterCallback)
		{
			mSetterCallback(value);
		}

		E_RESULT_CODE result = CGameUserSettings::Get()->SetInt32Variable(mId, value);
		TDE2_ASSERT(RC_OK == result);
	}

	I32 CInt32ConsoleVarDecl::Get() const
	{
		if (mGetterCallback)
		{
			return mGetterCallback();
		}

		const I32* pValue = CGameUserSettings::Get()->GetInt32Variable(mId);
		TDE2_ASSERT(pValue);

		return pValue ? *pValue : 0;
	}

	/*!
		\brief CFloatConsoleVarDecl's definition
	*/

	CFloatConsoleVarDecl::CFloatConsoleVarDecl(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const F32& initialValue,
		const TSetterType& setterCallback, const TGetterType& getterCallback) :
		mSetterCallback(setterCallback), mGetterCallback(getterCallback)
	{
		E_RESULT_CODE result = CGameUserSettings::Get()->CreateFloatVariable(name, description, flags, initialValue);
		TDE2_ASSERT(RC_OK == result);

		mId = PrepareIdentifierStr(name);
	}

	void CFloatConsoleVarDecl::Set(F32 value)
	{
		if (mSetterCallback)
		{
			mSetterCallback(value);
		}

		E_RESULT_CODE result = CGameUserSettings::Get()->SetFloatVariable(mId, value);
		TDE2_ASSERT(RC_OK == result);
	}

	F32 CFloatConsoleVarDecl::Get() const
	{
		const F32* pValue = CGameUserSettings::Get()->GetFloatVariable(mId);
		TDE2_ASSERT(pValue);

		return pValue ? *pValue : 0.0f;
	}

	/*!
		\brief CStringConsoleVarDecl's definition
	*/

	CStringConsoleVarDecl::CStringConsoleVarDecl(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const std::string& initialValue,
		const TSetterType& setterCallback, const TGetterType& getterCallback) :
		mSetterCallback(setterCallback), mGetterCallback(getterCallback)
	{
		E_RESULT_CODE result = CGameUserSettings::Get()->CreateStringVariable(name, description, flags, initialValue);
		TDE2_ASSERT(RC_OK == result);

		mId = PrepareIdentifierStr(name);
	}

	void CStringConsoleVarDecl::Set(const std::string& value)
	{
		if (mSetterCallback)
		{
			mSetterCallback(value);
		}

		E_RESULT_CODE result = CGameUserSettings::Get()->SetStringVariable(mId, value);
		TDE2_ASSERT(RC_OK == result);
	}

	const std::string& CStringConsoleVarDecl::Get() const
	{
		if (mGetterCallback)
		{
			return mGetterCallback();
		}

		return CGameUserSettings::Get()->GetStringVariable(mId);
	}
}