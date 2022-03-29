/*!
	\file CYAMLFileWriter.h
	\date 09.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IFile.h"
#include "../platform/CBaseFile.h"
#include "../../deps/yaml/Yaml.hpp"
#include <stack>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CYAMLFileWriter's type

		\return A pointer to CYAMLFileWriter's implementation
	*/

	TDE2_API IFile* CreateYAMLFileWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result);


	/*!
		class CYAMLFileWriter

		\brief The class is an implementation of IYAMLFileWriter
	*/

	class CYAMLFileWriter : public IYAMLFileWriter, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateYAMLFileWriter(IMountableStorage*, TPtr<IStream>, E_RESULT_CODE&);
		public:
			typedef std::stack<Yaml::Node*> TScopesStack;
			typedef std::stack<U32>         TScopesIndexers;
			typedef std::stack<bool>        TArrayScopesStack;
		public:
			TDE2_REGISTER_TYPE(CYAMLFileWriter)

			TDE2_API E_RESULT_CODE Serialize(Yaml::Node& object) override;

			/*!
				\brief The method opens specified file

				\param[in,out] pStorage A pointer to implementation of IMountableStorage
				\param[in,out] pStream A pointer to IStream implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Open(IMountableStorage* pStorage, TPtr<IStream> pStream) override;
			
			/*!
				\brief The method enters into object's scope with given identifier

				\param[in] key A name of an object
				\param[in] isArray Tells whether a group should be array

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE BeginGroup(const std::string& key, bool isArray = false) override;

			/*!
				\brief The method goes out of current scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EndGroup() override;

			TDE2_API E_RESULT_CODE SetUInt8(const std::string& key, U8 value) override;
			TDE2_API E_RESULT_CODE SetUInt16(const std::string& key, U16 value) override;
			TDE2_API E_RESULT_CODE SetUInt32(const std::string& key, U32 value) override;
			TDE2_API E_RESULT_CODE SetUInt64(const std::string& key, U64 value) override;

			TDE2_API E_RESULT_CODE SetInt8(const std::string& key, I8 value) override;
			TDE2_API E_RESULT_CODE SetInt16(const std::string& key, I16 value) override;
			TDE2_API E_RESULT_CODE SetInt32(const std::string& key, I32 value) override;
			TDE2_API E_RESULT_CODE SetInt64(const std::string& key, I64 value) override;

			TDE2_API E_RESULT_CODE SetFloat(const std::string& key, F32 value) override;
			TDE2_API E_RESULT_CODE SetDouble(const std::string& key, F64 value) override;

			TDE2_API E_RESULT_CODE SetBool(const std::string& key, bool value) override;

			TDE2_API E_RESULT_CODE SetString(const std::string& key, const std::string& value) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CYAMLFileWriter)

			TDE2_API E_RESULT_CODE _internalSerialize(Yaml::Node& object);

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API Yaml::Node* _getCurrScope() const;

			template <typename T>
			E_RESULT_CODE _setContent(const std::string& key, const T& value)
			{
				std::string valueStr = std::to_string(value);

				if (mIsArrayScope)
				{
					(*_getCurrScope())[mCurrElementIndex++] = valueStr;
					return RC_OK;
				}

				(*_getCurrScope())[key] = valueStr;
				return RC_OK;
			}
		protected:
			Yaml::Node*       mpRootNode;

			TScopesStack      mpContext;

			U32               mCurrElementIndex = 0;

			TScopesIndexers   mpScopesIndexers;

			bool              mIsArrayScope = false;

			TArrayScopesStack mArrayContext;
	};


	/*!
		\brief A factory function for creation objects of CYAMLFileReader's type

		\return A pointer to CYAMLFileReader's implementation
	*/

	TDE2_API IFile* CreateYAMLFileReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result);


	/*!
		class CYAMLFileReader

		\brief The class is an implementation of IYAMLFileReader
	*/

	class CYAMLFileReader : public IYAMLFileReader, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateYAMLFileReader(IMountableStorage*, TPtr<IStream>, E_RESULT_CODE&);
		public:
			typedef std::stack<Yaml::Node*> TScopesStack;
			typedef std::stack<U32>         TScopesIndexers;
		public:
			TDE2_REGISTER_TYPE(CYAMLFileReader)

			TDE2_API E_RESULT_CODE Deserialize(Yaml::Node& outputObject) override;

			/*!
				\brief The method opens specified file

				\param[in,out] pStorage A pointer to implementation of IMountableStorage
				\param[in,out] pStream A pointer to IStream implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Open(IMountableStorage* pStorage, TPtr<IStream> pStream) override;

			/*!
				\brief The method enters into object's scope with given identifier

				\param[in] key A name of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE BeginGroup(const std::string& key) override;

			/*!
				\brief The method goes out of current scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EndGroup() override;
			
			/*!
				\brief The method returns true if there is at least single element within current scope

				\return The method returns true if there is at least single element within current scope
			*/

			TDE2_API bool HasNextItem() const override;

			TDE2_API U8 GetUInt8(const std::string& key, U8 defaultValue = 0) override;
			TDE2_API U16 GetUInt16(const std::string& key, U16 defaultValue = 0) override;
			TDE2_API U32 GetUInt32(const std::string& key, U32 defaultValue = 0) override;
			TDE2_API U64 GetUInt64(const std::string& key, U64 defaultValue = 0) override;

			TDE2_API I8 GetInt8(const std::string& key, I8 defaultValue = 0) override;
			TDE2_API I16 GetInt16(const std::string& key, I16 defaultValue = 0) override;
			TDE2_API I32 GetInt32(const std::string& key, I32 defaultValue = 0) override;
			TDE2_API I64 GetInt64(const std::string& key, I64 defaultValue = 0) override;

			TDE2_API F32 GetFloat(const std::string& key, F32 defaultValue = 0.0f) override;
			TDE2_API F64 GetDouble(const std::string& key, F64 defaultValue = 0.0) override;

			TDE2_API bool GetBool(const std::string& key, bool defaultValue = false) override;

			TDE2_API std::string GetString(const std::string& key, const std::string& defaultValue = "") override;

			/*!
				\brief The method returns an identifier of current active node
				\return The method returns an identifier of current active node
			*/

			TDE2_API std::string GetCurrKey() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CYAMLFileReader)

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API Yaml::Node* _getCurrScope() const;

			TDE2_API E_RESULT_CODE _internalDeserialize(Yaml::Node& outputObject);

			template <typename T>
			T _getContentAs(const std::string& key)
			{
				if (key.empty())
				{
					return (*_getCurrScope())[mCurrElementIndex++].As<T>();
				}

				return (*_getCurrScope())[key].As<T>();
			}

			template <typename T>
			T _getContentAsOrDefault(const std::string& key, const T& defaultValue)
			{
				if (key.empty())
				{
					return (*_getCurrScope())[mCurrElementIndex++].As<T>(defaultValue);
				}

				return (*_getCurrScope())[key].As<T>(defaultValue);
			}
		protected:
			Yaml::Node*     mpRootNode;

			TScopesStack    mpContext;

			U32             mCurrElementIndex = 0;

			TScopesIndexers mpScopesIndexers;
	};
}