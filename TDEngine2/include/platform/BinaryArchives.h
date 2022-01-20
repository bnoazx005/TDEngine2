/*!
	\file BinaryArchives.h
	\date 23.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IFile.h"
#include "../platform/CBaseFile.h"
#include <vector>
#include <string>
#include <memory>
#include <tuple>
#include <stack>
#include <variant.hpp>


namespace TDEngine2
{
	class IInputStream;
	class IOutputStream;


	struct TArchiveValue
	{
		static constexpr U32 mBeginBlockTag = 0x42;
		static constexpr U32 mEndBlockTag = 0x240000;

		using TValue = Wrench::Variant<F64, I64, bool, std::string>;
		using TArchiveValuesArray = std::vector<std::unique_ptr<TArchiveValue>>;

		std::string mName;

		TValue mValue;

		TArchiveValuesArray mpChildren;

		template <typename T>
		static constexpr TypeId GetValueTag()
		{
			return TypeId(0x42); ///< Default tag
		}
	};


	/*!
		\brief A factory function for creation objects of CBinaryArchiveWriter's type

		\return A pointer to CBinaryArchiveWriter's implementation
	*/

	TDE2_API IFile* CreateBinaryArchiveWriter(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result);


	/*!
		class CBinaryArchiveWriter

		\brief The class is an implementation of IBinaryArchiveWriter, which is contains
		an array of keyed chunks of data

		BEGIN

		[CHUNK_KEY] [CHUNK]
				...
		[CHUNK_KEY] [CHUNK]

		END

		*** Description:

		<CHUNK_KEY> ::= [STR_LENGTH] [STR]

		<CHUNK> ::= [CHUNK_ID] [CHUNK_DATA]

		<CHUNK_ID> - Allows reader/writer to correctly recognize a type of the chunk

	*/

	class CBinaryArchiveWriter : public IBinaryArchiveWriter, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateBinaryArchiveWriter(IMountableStorage*, TPtr<IStream>, E_RESULT_CODE&);

		public:
			TDE2_REGISTER_TYPE(CBinaryArchiveWriter)

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBinaryArchiveWriter)

			TDE2_API E_RESULT_CODE _onFree() override;
		protected:
			IOutputStream* mpCachedOutputStream;
	};


	/*!
		\brief A factory function for creation objects of CBinaryArchiveReader's type

		\return A pointer to CBinaryArchiveReader's implementation
	*/

	TDE2_API IFile* CreateBinaryArchiveReader(IMountableStorage* pStorage, TPtr<IStream> pStream, E_RESULT_CODE& result);


	/*!
		class CBinaryArchiveReader

		\brief The class is an implementation of IBinaryArchiveReader
	*/

	class CBinaryArchiveReader : public IBinaryArchiveReader, public CBaseFile
	{
		public:
			friend TDE2_API IFile* CreateBinaryArchiveReader(IMountableStorage*, TPtr<IStream>, E_RESULT_CODE&);

		public:
			TDE2_REGISTER_TYPE(CBinaryArchiveReader)

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

			TDE2_API U8 GetUInt8(const std::string& key) override;
			TDE2_API U16 GetUInt16(const std::string& key) override;
			TDE2_API U32 GetUInt32(const std::string& key) override;
			TDE2_API U64 GetUInt64(const std::string& key) override;

			TDE2_API I8 GetInt8(const std::string& key) override;
			TDE2_API I16 GetInt16(const std::string& key) override;
			TDE2_API I32 GetInt32(const std::string& key) override;
			TDE2_API I64 GetInt64(const std::string& key) override;

			TDE2_API F32 GetFloat(const std::string& key) override;
			TDE2_API F64 GetDouble(const std::string& key) override;

			TDE2_API bool GetBool(const std::string& key) override;

			TDE2_API std::string GetString(const std::string& key) override;

			/*!
				\brief The method returns an identifier of current active node
				\return The method returns an identifier of current active node
			*/

			TDE2_API std::string GetCurrKey() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBinaryArchiveReader)

			TDE2_API E_RESULT_CODE _onFree() override;

			TDE2_API E_RESULT_CODE _deserializeInternal();
			
			TDE2_API std::unique_ptr<TArchiveValue> _readChunkData();
			TDE2_API U32 _readChunkTag();
			TDE2_API std::string _readStringChunk();
			
			TDE2_API std::unique_ptr<TArchiveValue> _readIntValue(const std::string& key, TSizeType size);
			TDE2_API std::unique_ptr<TArchiveValue> _readFloatValue(const std::string& key, TSizeType size);
			TDE2_API std::unique_ptr<TArchiveValue> _readBoolValue(const std::string& key);
			TDE2_API std::unique_ptr<TArchiveValue> _readStringValue(const std::string& key);

			TDE2_API TArchiveValue::TArchiveValuesArray _readChunk();

			TDE2_API E_RESULT_CODE _readValue(void* pBuffer, TSizeType size);

			TDE2_API const TArchiveValue* _getContent(const std::string& key);

			template <typename T>
			T _getContentAsOrDefault(const std::string& key, T defaultValue)
			{
				const TArchiveValue* pValue = _getContent(key);
				if (!pValue)
				{
					return defaultValue;
				}

				return _tryToGetValue<T>(pValue->mValue, defaultValue);
			}

			template <typename T>
			T _tryToGetValue(const TArchiveValue::TValue& value, T defaultValue)
			{
				return !value.Is<T>() ? defaultValue : value.As<T>();
			}
		protected:
			IInputStream* mpCachedInputStream;
			
			TArchiveValue::TArchiveValuesArray mTopLevelDecls;

			TArchiveValue* mpCurrNode;
			std::stack<std::tuple<U32, TArchiveValue*>> mpHierarchyContext; // \note contains pointers to parents, U32 is current counter within the scope

			U32 mCurrElementIndex = 0;
	};
}