/*!
	\file CGameUserSettings.h
	\date 07.10.2021
	\author Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include "CProjectSettings.h"
#include <string>
#include <memory>
#include <functional>
#include <array>
#include "delegate.hpp"


namespace TDEngine2
{
	class IConfigFileReader;
	class IConfigFileWriter;
	class CConsoleVariablesStorage;


	enum class E_CONSOLE_VARIABLE_FLAGS : U32
	{
		NONE = 0x0,
		SYSTEM = 1 << 0,
	};

	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_CONSOLE_VARIABLE_FLAGS)


	enum class E_CONSOLE_VARIABLE_TYPE : U32 
	{
		INT, FLOAT, STRING
	};


	struct TUserConfigVariableInfo
	{
		std::string mName;
		std::string mGroup;
		std::string mDescription;

		E_CONSOLE_VARIABLE_FLAGS mFlags = E_CONSOLE_VARIABLE_FLAGS::NONE;

		E_CONSOLE_VARIABLE_TYPE mType = E_CONSOLE_VARIABLE_TYPE::INT;

		USIZE mHandle = (std::numeric_limits<U32>::max)();
	};


	struct CInt32ConsoleVarDecl;
	struct CFloatConsoleVarDecl;
	struct CStringConsoleVarDecl;

	typedef std::unique_ptr<CInt32ConsoleVarDecl>  CInt32ConsoleVarDeclPtr;
	typedef std::unique_ptr<CFloatConsoleVarDecl>  CFloatConsoleVarDeclPtr;
	typedef std::unique_ptr<CStringConsoleVarDecl> CStringConsoleVarDeclPtr;


	/*!
		class CGameUserSettings

		\brief The class is a singleton which contains all bunch of user related settings that
		vary from project to project.

		This is runtime settings of the application in contrast with CProjectSettings

		The object of CGameUserSettings consists of two parts prebuilt members which are placed in public 
		section and their serialization/deserialization are done manually. And the second part is 
		console variables API that allows to create variables in runtime and then save into same configuration file.

		There are also set of methods which support runtime modification via in-game console.
		You can define console variable (CVars in terms of Quake, Unreal) using CreateXVariable.
	*/

	class CGameUserSettings : public CBaseObject
	{
		public:
			TDE2_API E_RESULT_CODE Init(IConfigFileReader* pConfigFileReader);

			TDE2_API E_RESULT_CODE Save(IConfigFileWriter* pConfigFileWriter);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of CGameUserSettings type
			*/

			TDE2_API static TPtr<CGameUserSettings> Get();

			TDE2_API E_RESULT_CODE CreateInt32Variable(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const I32& initialValue);
			TDE2_API E_RESULT_CODE CreateFloatVariable(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const F32& initialValue);
			TDE2_API E_RESULT_CODE CreateStringVariable(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const std::string& initialValue);

			/*!
				\brief Allows to update value of the given console variable if it exists

				\param[in] name An identifier of variable that has the following structure "GroupId.VarId" or "VarId" if you want to
				use default group which is Locals
			*/

			TDE2_API E_RESULT_CODE SetInt32Variable(const std::string& name, I32 value);
			TDE2_API E_RESULT_CODE SetFloatVariable(const std::string& name, F32 value);
			TDE2_API E_RESULT_CODE SetStringVariable(const std::string& name, const std::string& value);

			TDE2_API const I32* GetInt32Variable(const std::string& name) const;
			TDE2_API const F32* GetFloatVariable(const std::string& name) const;
			TDE2_API const std::string& GetStringVariable(const std::string& name) const;

			TDE2_API const TUserConfigVariableInfo* GetVariableInfo(const std::string& name) const;

#if TDE2_EDITORS_ENABLED
			TDE2_API void RegisterObjectInProfiler(const std::string& id) override;
			TDE2_API void OnBeforeMemoryRelease() override;
#endif
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS_NO_DCTR(CGameUserSettings)
			TDE2_API virtual ~CGameUserSettings();
		public:
			CInt32ConsoleVarDeclPtr mpWindowWidthCVar;
			CInt32ConsoleVarDeclPtr mpWindowHeightCVar;

			CInt32ConsoleVarDeclPtr mpFullscreenCVar;

			CStringConsoleVarDeclPtr mpCurrLanguageCVar;

			CInt32ConsoleVarDeclPtr mpIsShadowMappingEnabledCVar;
			CInt32ConsoleVarDeclPtr mpShadowMapSizesCVar;
			CInt32ConsoleVarDeclPtr mpShadowCascadesCountCVar;

			std::array<CFloatConsoleVarDeclPtr, 4> mpShadowCascadesSplitsCVar;

		private:
			std::unique_ptr<CConsoleVariablesStorage> mpCVarsStorage;
	};


	/*!
		\brief The function parses value and creates or updates cvar based on the string's content
	*/

	TDE2_API E_RESULT_CODE CreateCVarFromString(const std::string& name, const std::string& value);


	template <typename T>
	struct CBaseConsoleVarDecl
	{
		Wrench::Delegate<const T&> mOnValueChanged;

		std::string mId;
	};


	struct CInt32ConsoleVarDecl : CBaseConsoleVarDecl<I32>
	{
		typedef std::function<void(I32)> TSetterType;
		typedef std::function<I32()>     TGetterType;

		TDE2_API CInt32ConsoleVarDecl(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const I32& initialValue,
			const TSetterType& setterCallback = nullptr, const TGetterType& getterCallback = nullptr);

		TDE2_API void Set(I32 value);
		TDE2_API I32 Get() const;

		TSetterType mSetterCallback = nullptr;
		TGetterType mGetterCallback = nullptr;
	};


	struct CFloatConsoleVarDecl : CBaseConsoleVarDecl<F32>
	{
		typedef std::function<void(F32)> TSetterType;
		typedef std::function<F32()>     TGetterType;

		TDE2_API CFloatConsoleVarDecl(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const F32& initialValue,
			const TSetterType& setterCallback = nullptr, const TGetterType& getterCallback = nullptr);

		TDE2_API void Set(F32 value);
		TDE2_API F32 Get() const;

		TSetterType mSetterCallback = nullptr;
		TGetterType mGetterCallback = nullptr;
	};


	struct CStringConsoleVarDecl : CBaseConsoleVarDecl<std::string>
	{
		typedef std::function<void(const std::string&)> TSetterType;
		typedef std::function<const std::string&()>     TGetterType;

		TDE2_API CStringConsoleVarDecl(const std::string& name, const std::string& description, E_CONSOLE_VARIABLE_FLAGS flags, const std::string& initialValue,
			const TSetterType& setterCallback = nullptr, const TGetterType& getterCallback = nullptr);

		TDE2_API void Set(const std::string& value);
		TDE2_API const std::string& Get() const;

		TSetterType mSetterCallback = nullptr;
		TGetterType mGetterCallback = nullptr;
	};
}