/*!
	\file CSaveData.h
	\date 31.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ISaveManager.h"
#include "../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSaveData's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSaveData's implementation
	*/

	TDE2_API ISaveData* CreateSaveData(E_RESULT_CODE& result);


	/*!
		class CSaveData

		\brief The class is a base for all game saves. To provide a support of your own type
		derive it from this one, define its factory method and implement own Save and Load methods
	*/

	class CSaveData : public CBaseObject, public ISaveData
	{
		public:
			friend TDE2_API ISaveData* CreateSaveData(E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			TDE2_API E_RESULT_CODE SetId(const std::string& saveId) override;
			TDE2_API E_RESULT_CODE SetPath(const std::string& savePath) override;

			TDE2_API const std::string& GetId() const override;
			TDE2_API const std::string& GetPath() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSaveData)
		protected:
			std::string mPath;
			std::string mId;
	};
}