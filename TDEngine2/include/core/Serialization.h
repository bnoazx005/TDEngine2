/*!
	\file Serialization.h
	\date 29.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"
#include <string>


namespace TDEngine2
{
	class IArchiveReader;
	class IArchiveWriter;


	/*!
		interface ISerializable 

		\brief The interface represents a functionality of any serializable entity within the engine. 
		Mostly it's some resource type or components
	*/

	class ISerializable
	{
		public:
			/*!
				\brief The method deserializes object's state from given reader 

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Load(IArchiveReader* pReader) = 0;
			
			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Save(IArchiveWriter* pWriter) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISerializable);
	};


	/*!
		interface IArchiveReader

		\brief The interface describes a functionality of serialization stream for reading
	*/

	class IArchiveReader
	{
		public:
			TDE2_REGISTER_TYPE(IArchiveReader)

			/*!
				\brief The method enters into object's scope with given identifier

				\param[in] key A name of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BeginGroup(const std::string& key) = 0;

			/*!
				\brief The method goes out of current scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE EndGroup() = 0;

			/*!
				\brief The method returns true if there is at least single element within current scope

				\return The method returns true if there is at least single element within current scope
			*/

			TDE2_API virtual bool HasNextItem() const = 0;

			TDE2_API virtual U8 GetUInt8(const std::string& key, U8 defaultValue = 0) = 0;
			TDE2_API virtual U16 GetUInt16(const std::string& key, U16 defaultValue = 0) = 0;
			TDE2_API virtual U32 GetUInt32(const std::string& key, U32 defaultValue = 0) = 0;
			TDE2_API virtual U64 GetUInt64(const std::string& key, U64 defaultValue = 0) = 0;

			TDE2_API virtual I8 GetInt8(const std::string& key, I8 defaultValue = 0) = 0;
			TDE2_API virtual I16 GetInt16(const std::string& key, I16 defaultValue = 0) = 0;
			TDE2_API virtual I32 GetInt32(const std::string& key, I32 defaultValue = 0) = 0;
			TDE2_API virtual I64 GetInt64(const std::string& key, I64 defaultValue = 0) = 0;

			TDE2_API virtual F32 GetFloat(const std::string& key, F32 defaultValue = 0.0f) = 0;
			TDE2_API virtual F64 GetDouble(const std::string& key, F64 defaultValue = 0.0) = 0;

			TDE2_API virtual bool GetBool(const std::string& key, bool defaultValue = false) = 0;

			TDE2_API virtual std::string GetString(const std::string& key, const std::string& defaultValue = "") = 0;

			/*!
				\brief The method returns an identifier of current active node
				\return The method returns an identifier of current active node
			*/

			TDE2_API virtual std::string GetCurrKey() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IArchiveReader);
	};


	/*!
		interface IArchiveWriter

		\brief The interface describes a functionality of serialization stream for writing
	*/

	class IArchiveWriter
	{
		public:
			TDE2_REGISTER_TYPE(IArchiveWriter)

			/*!
				\brief The method enters into object's scope with given identifier

				\param[in] key A name of an object
				\param[in] isArray Tells whether a group should be array

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BeginGroup(const std::string& key, bool isArray = false) = 0;

			/*!
				\brief The method goes out of current scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE EndGroup() = 0;

			TDE2_API virtual E_RESULT_CODE SetUInt8(const std::string& key, U8 value) = 0;
			TDE2_API virtual E_RESULT_CODE SetUInt16(const std::string& key, U16 value) = 0;
			TDE2_API virtual E_RESULT_CODE SetUInt32(const std::string& key, U32 value) = 0;
			TDE2_API virtual E_RESULT_CODE SetUInt64(const std::string& key, U64 value) = 0;

			TDE2_API virtual E_RESULT_CODE SetInt8(const std::string& key, I8 value) = 0;
			TDE2_API virtual E_RESULT_CODE SetInt16(const std::string& key, I16 value) = 0;
			TDE2_API virtual E_RESULT_CODE SetInt32(const std::string& key, I32 value) = 0;
			TDE2_API virtual E_RESULT_CODE SetInt64(const std::string& key, I64 value) = 0;

			TDE2_API virtual E_RESULT_CODE SetFloat(const std::string& key, F32 value) = 0;
			TDE2_API virtual E_RESULT_CODE SetDouble(const std::string& key, F64 value) = 0;

			TDE2_API virtual E_RESULT_CODE SetBool(const std::string& key, bool value) = 0;

			TDE2_API virtual E_RESULT_CODE SetString(const std::string& key, const std::string& value) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IArchiveWriter);
	};


	template <typename T>
	class ICloneable
	{
		public:
			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same
			*/

			TDE2_API virtual TPtr<T> Clone() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ICloneable);
	};


	/*!
		interface INonAllocCloneable

		\brief The main difference from ICloneable that's the current one doesn't present functionality of 
		allocation a memory for the target object and just copies all the data from current to specified instance
	*/

	template <typename T>
	class INonAllocCloneable
	{
		public:
			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API virtual E_RESULT_CODE Clone(T*& pDestObject) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(INonAllocCloneable);
	};


	template <typename T> E_RESULT_CODE Serialize(IArchiveWriter* pWriter, T value) { return RC_FAIL; }

	/*!
		\brief serialization helpers for built-in types
	*/

	template <> TDE2_API E_RESULT_CODE Serialize<I8>(IArchiveWriter* pWriter, I8 value);
	template <> TDE2_API E_RESULT_CODE Serialize<I16>(IArchiveWriter* pWriter, I16 value);
	template <> TDE2_API E_RESULT_CODE Serialize<I32>(IArchiveWriter* pWriter, I32 value);
	template <> TDE2_API E_RESULT_CODE Serialize<I64>(IArchiveWriter* pWriter, I64 value);

	template <> TDE2_API E_RESULT_CODE Serialize<U8>(IArchiveWriter* pWriter, U8 value);
	template <> TDE2_API E_RESULT_CODE Serialize<U16>(IArchiveWriter* pWriter, U16 value);
	template <> TDE2_API E_RESULT_CODE Serialize<U32>(IArchiveWriter* pWriter, U32 value);
	template <> TDE2_API E_RESULT_CODE Serialize<U64>(IArchiveWriter* pWriter, U64 value);

	template <> TDE2_API E_RESULT_CODE Serialize<F32>(IArchiveWriter* pWriter, F32 value);
	template <> TDE2_API E_RESULT_CODE Serialize<F64>(IArchiveWriter* pWriter, F64 value);

	template <> TDE2_API E_RESULT_CODE Serialize<bool>(IArchiveWriter* pWriter, bool value);
	template <> TDE2_API E_RESULT_CODE Serialize<std::string>(IArchiveWriter* pWriter, std::string value);
}