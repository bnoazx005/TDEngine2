/*!
	\file CPrefabChangesList.h
	\date 19.05.2023
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/Serialization.h"
#include "../core/CBaseObject.h"
#include "../core/Meta.h"
#include "../ecs/CEntity.h"
#include <vector>
#include <string>


namespace TDEngine2
{
	class CPrefabChangesList; 
	class CEntityManager;

	struct TEntitiesMapper;


	enum class TEntityId : U32;


	/*!
		\brief A factory function for creation objects of CPrefabChangesList's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFont's implementation
	*/

	TDE2_API CPrefabChangesList* CreatePrefabChangesList(E_RESULT_CODE& result);


	/*!
		class CPrefabChangesList

		\brief The class stores list of properties of a prefab's instance that differ from the original template
	*/

	class CPrefabChangesList: public CBaseObject, public ISerializable, public ICloneable<CPrefabChangesList>
	{
		public:
			friend TDE2_API CPrefabChangesList* CreatePrefabChangesList(E_RESULT_CODE&);
		public:
			struct TChangeDesc
			{
				CEntityRef    mTargetLinkEntityId;
				std::string   mPropertyBinding;
				CValueWrapper mValue;
			};

			typedef std::vector<TChangeDesc> TChangesArray;
		public:
			/*!
				\brief The method initializes an internal state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

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

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same
			*/

			TDE2_API TPtr<CPrefabChangesList> Clone() const override;

			TDE2_API E_RESULT_CODE ApplyChanges(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesMappings);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPrefabChangesList)
		private:
			TChangesArray mChanges;
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(CPrefabChangesList)
}