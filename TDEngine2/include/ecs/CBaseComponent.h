/*!
	\file CBaseComponent.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IComponent.h"
#include "./../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		class CBaseComponent

		\brief The class is a base class for all the components, which can be
		declared.

		The example of custom component declaration:

		class CCustomComponent: public CBaseComponent<CCustomComponent>
		{
			//...
		};

		The following statement is needed to implement proper registration of component's type
	*/

	template <typename T>
	class CBaseComponent: public IComponent, public CBaseObject
	{
		public:
			/*!
				\brief The method initializes an entity

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init();

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			static TComponentTypeId GetTypeId()
			{
				return mComponentTypeId;
			}
		protected:
			TDE2_API CBaseComponent();
			TDE2_API virtual ~CBaseComponent() = default;
			TDE2_API CBaseComponent(const CBaseComponent& component) = delete;
			TDE2_API virtual CBaseComponent& operator=(const CBaseComponent& component) = delete;
		protected:
			static TComponentTypeId mComponentTypeId;
	};


	template<typename T>
	CBaseComponent<T>::CBaseComponent() :
		CBaseObject()
	{
	}

	template<typename T>
	E_RESULT_CODE CBaseComponent<T>::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	/*!
	\brief The method frees all memory occupied by the object

	\return RC_OK if everything went ok, or some other code, which describes an error
	*/

	template<typename T>
	E_RESULT_CODE CBaseComponent<T>::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}
	
	/*!
	\brief The function generates a new type's identifier each call

	\return A value of component's type
	*/

	static TComponentTypeId GenerateTypeId()
	{
		static TComponentTypeId mCounter = 0;

		return mCounter++;
	}

	template<typename T>
	TComponentTypeId CBaseComponent<T>::mComponentTypeId = GenerateTypeId();
}