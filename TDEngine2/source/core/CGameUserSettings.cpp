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
	};


	const std::string TUserSettingsKeys::mMainGroupKeyId = "main";
	const std::string TUserSettingsKeys::mGraphicsGroupKeyId = "graphics";


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
				pConfigFileReader->ForEachParameter([pConfigFileReader](auto&& groupId, auto&& variableId)
				{
					CreateCVarFromString(groupId + mDefaultIdentifierSeparator + variableId, pConfigFileReader->GetString(groupId, variableId));
					return true;
				});

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

				return Wrench::TOkValue<TUserConfigVariableInfo*>(&mVariablesPerGroupTable[variableInfo.mGroup][variableInfo.mName]);
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

		mpWindowWidthCVar = std::make_unique<CInt32ConsoleVarDecl>(TUserSettingsKeys::mMainGroupKeyId + ".width", "Window's width", 640);
		mpWindowHeightCVar = std::make_unique<CInt32ConsoleVarDecl>(TUserSettingsKeys::mMainGroupKeyId + ".height", "Window's height", 480);

		mpFullscreenCVar = std::make_unique<CInt32ConsoleVarDecl>(TUserSettingsKeys::mMainGroupKeyId + ".fullscreen", 
			"Flag determines whether or not fullscreen enabled (0 - disabled, 1 - enabled)", 0);

		mpCurrLanguageCVar = std::make_unique<CStringConsoleVarDecl>(TUserSettingsKeys::mMainGroupKeyId + ".lang", "Current localization (ru|en|)", "en");

		// \note graphics section
		mpIsShadowMappingEnabledCVar = std::make_unique<CInt32ConsoleVarDecl>(TUserSettingsKeys::mGraphicsGroupKeyId + ".shadows",
			"Flag determines whether or not shadow mapping enabled (0 - disabled, 1 - enabled)", 1);
		mpShadowMapSizesCVar = std::make_unique<CInt32ConsoleVarDecl>(TUserSettingsKeys::mGraphicsGroupKeyId + ".shadow_map_size",
			"Shadow map sizes", 512);
		mpShadowCascadesCountCVar = std::make_unique<CInt32ConsoleVarDecl>(TUserSettingsKeys::mGraphicsGroupKeyId + ".shadow_cascades_count",
			"Amount of shadow maps cascades", 3);

		mpShadowCascadesSplitsCVar[0] = std::make_unique<CFloatConsoleVarDecl>(TUserSettingsKeys::mGraphicsGroupKeyId + ".shadow_cascades_splits_0",
			"Shadow cascade split's range", 0.020f);
		mpShadowCascadesSplitsCVar[1] = std::make_unique<CFloatConsoleVarDecl>(TUserSettingsKeys::mGraphicsGroupKeyId + ".shadow_cascades_splits_1",
			"Shadow cascade split's range", 0.045f);
		mpShadowCascadesSplitsCVar[2] = std::make_unique<CFloatConsoleVarDecl>(TUserSettingsKeys::mGraphicsGroupKeyId + ".shadow_cascades_splits_2",
			"Shadow cascade split's range", 0.08f);
		mpShadowCascadesSplitsCVar[3] = std::make_unique<CFloatConsoleVarDecl>(TUserSettingsKeys::mGraphicsGroupKeyId + ".shadow_cascades_splits_3",
			"Shadow cascade split's range", 1.0f);

		return mpCVarsStorage->Load(pConfigFileReader);
	}

	E_RESULT_CODE CGameUserSettings::Save(IConfigFileWriter* pConfigFileWriter)
	{
		if (!pConfigFileWriter)
		{
			return RC_INVALID_ARGS;
		}

		return mpCVarsStorage->Save(pConfigFileWriter);
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


	E_RESULT_CODE CreateCVarFromString(const std::string& name, const std::string& value)
	{
		E_RESULT_CODE result = RC_OK;

		if (CGameUserSettings::Get()->GetVariableInfo(name) != nullptr) // change value because variable already declared
		{
			if (value.find_first_of('.') != std::string::npos) // \note try parse float value
			{
				result = CGameUserSettings::Get()->SetFloatVariable(name, static_cast<F32>(atof(value.c_str())));
			}
			else if (value.find_first_of('\"') != std::string::npos) // \note string
			{
				result = CGameUserSettings::Get()->SetStringVariable(name, value.substr(1, value.length() - 2));
			}
			else
			{
				result = CGameUserSettings::Get()->SetInt32Variable(name, atoi(value.c_str()));
			}
		}
		else
		{
			if (value.find_first_of('.') != std::string::npos) // \note try parse float value
			{
				result = CGameUserSettings::Get()->CreateFloatVariable(name, Wrench::StringUtils::GetEmptyStr(), E_CONSOLE_VARIABLE_FLAGS::NONE, static_cast<F32>(atof(value.c_str())));
			}
			else if (value.find_first_of('\"') != std::string::npos) // \note string
			{
				result = CGameUserSettings::Get()->CreateStringVariable(name, Wrench::StringUtils::GetEmptyStr(), E_CONSOLE_VARIABLE_FLAGS::NONE, value.substr(1, value.length() - 2));
			}
			else
			{
				result = CGameUserSettings::Get()->CreateInt32Variable(name, Wrench::StringUtils::GetEmptyStr(), E_CONSOLE_VARIABLE_FLAGS::NONE, atoi(value.c_str()));
			}
		}

		return result;
	}


	/*!
		\brief CInt32ConsoleVarDecl's definition
	*/

	CInt32ConsoleVarDecl::CInt32ConsoleVarDecl(const std::string& name, const std::string& description, const I32& initialValue, E_CONSOLE_VARIABLE_FLAGS flags,
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

		mOnValueChanged.Notify(value);
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

	CFloatConsoleVarDecl::CFloatConsoleVarDecl(const std::string& name, const std::string& description, const F32& initialValue, E_CONSOLE_VARIABLE_FLAGS flags,
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

		mOnValueChanged.Notify(value);
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

	CStringConsoleVarDecl::CStringConsoleVarDecl(const std::string& name, const std::string& description, const std::string& initialValue, E_CONSOLE_VARIABLE_FLAGS flags,
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

		mOnValueChanged.Notify(value);
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