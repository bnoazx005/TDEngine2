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
				\brief The method enters into array's scope with given identifier

				\param[in] key A name of an array's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BeginArray(const std::string& key) = 0;

			/*!
				\brief The method goes out of current scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE EndArray() = 0;
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

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BeginGroup(const std::string& key) = 0;

			/*!
				\brief The method goes out of current scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE EndGroup() = 0;

			/*!
				\brief The method enters into array's scope with given identifier

				\param[in] key A name of an array's object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE BeginArray(const std::string& key) = 0;

			/*!
				\brief The method goes out of current scope

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE EndArray() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IArchiveWriter);
	};
}