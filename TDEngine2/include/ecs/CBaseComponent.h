/*!
	\file CBaseComponent.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IComponent.h"
#include "./../core/CBaseObject.h"
#include "./../core/Event.h"
#include <vector>


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

	class CBaseComponent: public IComponent, public CBaseObject
	{
		public:
			TDE2_REGISTER_TYPE(CBaseComponent)

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseComponent)
	};


	/*!
		class CComponentIterator

		\brief The class is a common implementation of a one way iterator over IComponent collection
	*/

	class CComponentIterator
	{
		protected:
			typedef std::vector<IComponent*> TComponentsArray;
		public:
			static CComponentIterator mInvalidIterator;
		public:
			/*!
				\brief The constructor creates a correct iterator using begin and end iterators of a collection
				and index that specifies an initial offset from the beginning

				\param[in, out] components An array of components

				\param[in] index An offset from a beginning of a collection where a result iterator
				will be placed at
			*/

			TDE2_API CComponentIterator(TComponentsArray& components, U32 index);

			TDE2_API ~CComponentIterator();

			/*!
				\brief The method increments current iterator so it point to next
				entity within a collection

				\return A reference to an iterator to next entity within a collection
			*/

			TDE2_API CComponentIterator& Next();

			/*!
				\brief The method is a predicate that is true if there is at least one
				entity next to the current position of the iterator

				\return The method returns true if there is at least one
				entity next to the current position of the iterator
			*/

			TDE2_API bool HasNext() const;

			/*!
				\brief The method resets current state of the iterator to the beginnning of
				a collection
			*/

			TDE2_API void Reset();

			/*!
				\brief The method returns an entity that is pointed by the iterator

				\return  The method returns a pointer to specific IComponent that is pointed by the iterator
			*/

			TDE2_API IComponent* Get() const;

			/*!
				\brief The method returns an entity that is pointed by the iterator

				\return  The method returns a pointer to specific IComponent that is pointed by the iterator
			*/

			TDE2_API IComponent* operator* () const;

			/*!
				\brief The method increments current iterator so it point to next
				entity within a collection

				\return A reference to an iterator to next entity within a collection
			*/

			TDE2_API CComponentIterator& operator++ ();

			TDE2_API bool operator== (const CComponentIterator& iter) const;

			TDE2_API bool operator!= (const CComponentIterator& iter) const;
		protected:
			TDE2_API CComponentIterator();
		protected:
			TComponentsArray::iterator mBegin;

			TComponentsArray::iterator mEnd;

			U32                        mCurrIndex;
	};


	/*!
		struct TOnComponentCreatedEvent

		\brief The structure represents an event which occurs
		when a new component was created
	*/

	typedef struct TOnComponentCreatedEvent: TBaseEvent
	{
		virtual ~TOnComponentCreatedEvent() = default;

		TDE2_REGISTER_TYPE(TOnComponentCreatedEvent)

		REGISTER_EVENT_TYPE(TOnComponentCreatedEvent)
			
		TEntityId        mEntityId;

		TComponentTypeId mCreatedComponentTypeId;
	} TOnComponentCreatedEvent, *TOnComponentCreatedEventPtr;


	/*!
		struct TOnComponentRemovedEvent

		\brief The structure represents an event which occurs
		when some component was removed from an entity
	*/

	typedef struct TOnComponentRemovedEvent : TBaseEvent
	{
		virtual ~TOnComponentRemovedEvent() = default;

		TDE2_REGISTER_TYPE(TOnComponentRemovedEvent)

		REGISTER_EVENT_TYPE(TOnComponentRemovedEvent)
		
		TEntityId        mEntityId;

		TComponentTypeId mRemovedComponentTypeId;
	} TOnComponentRemovedEvent, *TOnComponentRemovedEventPtr;
}