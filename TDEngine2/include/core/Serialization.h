/*!
	\file Serialization.h
	\date 29.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "IFile.h"
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

			TDE2_API virtual U8 GetUInt8(const std::string& key) = 0;
			TDE2_API virtual U16 GetUInt16(const std::string& key) = 0;
			TDE2_API virtual U32 GetUInt32(const std::string& key) = 0;
			TDE2_API virtual U64 GetUInt64(const std::string& key) = 0;

			TDE2_API virtual I8 GetInt8(const std::string& key) = 0;
			TDE2_API virtual I16 GetInt16(const std::string& key) = 0;
			TDE2_API virtual I32 GetInt32(const std::string& key) = 0;
			TDE2_API virtual I64 GetInt64(const std::string& key) = 0;

			TDE2_API virtual F32 GetFloat(const std::string& key) = 0;
			TDE2_API virtual F64 GetDouble(const std::string& key) = 0;

			TDE2_API virtual bool GetBool(const std::string& key) = 0;

			TDE2_API virtual std::string GetString(const std::string& key) = 0;

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
}