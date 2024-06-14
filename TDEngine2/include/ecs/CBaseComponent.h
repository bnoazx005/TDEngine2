/*!
	\file CBaseComponent.h
	\date 28.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IComponent.h"
#include "../core/CBaseObject.h"
#include "../core/Event.h"
#include "../core/memory/CPoolAllocator.h"
#include "../ecs/IComponentFactory.h"
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

	class CBaseComponent : public virtual IComponent, public CBaseObject
	{
		public:
			TDE2_REGISTER_TYPE(CBaseComponent)

			/*!
				\brief The method initializes an internal state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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
				\brief The method is called after all entities of particular scene were loaded. It remaps all identifiers to
				make them correctly corresponds to saved state

				\param[in, out] pEntityManager A pointer to entities manager
				\param[in] entitiesIdentifiersRemapper A structure that maps saved identifier to current runtime equivalent
			*/

			TDE2_API virtual E_RESULT_CODE PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper) override;

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;

			/*!
				\return The method returns a pointer to a type's property if the latter does exist or null pointer in other cases
			*/

			TDE2_API IPropertyWrapperPtr GetProperty(const std::string& propertyName) override;

			/*!
				\brief The method returns an array of properties names that are available for usage
			*/

			TDE2_API const std::vector<std::string>& GetAllProperties() const override;

			/*!
				\return The method returns true if the given component type is for runtime purposes only
			*/

			TDE2_API bool IsRuntimeOnly() const override;
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

			bool                       mIsValid = false;
	};


	/*!
		struct TOnComponentCreatedEvent

		\brief The structure represents an event which occurs
		when a new component was created
	*/

	typedef struct TOnComponentCreatedEvent : TBaseEvent
	{
		virtual ~TOnComponentCreatedEvent() = default;

		TDE2_REGISTER_TYPE(TOnComponentCreatedEvent)
		REGISTER_EVENT_TYPE(TOnComponentCreatedEvent)
		
		TEntityId mEntityId = TEntityId::Invalid;

		TypeId    mCreatedComponentTypeId = TypeId::Invalid;
	} TOnComponentCreatedEvent, * TOnComponentCreatedEventPtr;


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

		TEntityId           mEntityId = TEntityId::Invalid;

		std::vector<TypeId> mRemovedComponentsTypeId {};
	} TOnComponentRemovedEvent, * TOnComponentRemovedEventPtr;


	/*!
		struct TOnEntityActivityChangedEvent

		\brief The structure represents an event which occurs when an activity state of the entity's changed
	*/

	typedef struct TOnEntityActivityChangedEvent : TBaseEvent
	{
		virtual ~TOnEntityActivityChangedEvent() = default;

		TDE2_REGISTER_TYPE(TOnEntityActivityChangedEvent)
		REGISTER_EVENT_TYPE(TOnEntityActivityChangedEvent)

		TEntityId mEntityId = TEntityId::Invalid;
		bool      mNewActivityState = false;
	} TOnEntityActivityChangedEvent, *TOnEntityActivityChangedEventPtr;


#define TDE2_COMPONENT_CLASS_NAME(ComponentName)			C ## ComponentName
#define TDE2_COMPONENT_FUNCTION_NAME(ComponentName)			Create ## ComponentName
#define TDE2_COMPONENT_FACTORY_NAME(ComponentName)			C ## ComponentName ## Factory
#define TDE2_COMPONENT_FACTORY_FUNCTION_NAME(ComponentName)	Create ## ComponentName ## Factory


	struct TComponentTypeInfo
	{
		std::string mName;
		TypeId      mTypeId;
	};


	typedef std::function<IComponentFactory* (E_RESULT_CODE&)> TComponentFactoryFunctor;


	/*!
		\brief The template is used to declare concrete component factories
	*/

	template <typename TComponentType, typename TComponentParamsType>
	class CBaseComponentFactory: public IGenericComponentFactory<>, public CBaseObject
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override
			{
				if (mIsInitialized)
				{
					return RC_FAIL;
				}

				mIsInitialized = true;

				return RC_OK;
			}

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override
			{
				if (IComponent* pComponent = CreateDefault())
				{
					SetupComponent(dynamic_cast<TComponentType*>(pComponent), *dynamic_cast<const TComponentParamsType*>(pParams));
				}

				return nullptr;
			}

			TDE2_API virtual E_RESULT_CODE SetupComponent(TComponentType* pComponent, const TComponentParamsType&) const = 0;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override
			{
				return TComponentType::GetTypeId();
			}

			TDE2_API const std::string& GetComponentTypeStr() const override
			{
				return TComponentType::GetClassId();
			}
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseComponentFactory)
	};


	template <typename TComponentType, typename TComponentParamsType>
	CBaseComponentFactory<TComponentType, TComponentParamsType>::CBaseComponentFactory():
		CBaseObject()
	{
	}


#define TDE2_DECLARE_COMPONENT_FACTORY_IMPL(ComponentNameStr, ComponentName, ComponentParamsType, ComponentFactoryName, ComponentFactoryFunctor)		\
	TDE2_API ::TDEngine2::IComponentFactory* ComponentFactoryFunctor(::TDEngine2::E_RESULT_CODE& result);												\
																																						\
	class ComponentFactoryName : public ::TDEngine2::CBaseComponentFactory<ComponentName, ComponentParamsType>											\
	{																																					\
		public:																																			\
			friend TDE2_API ::TDEngine2::IComponentFactory* ComponentFactoryFunctor(::TDEngine2::E_RESULT_CODE&);										\
																																						\
			TDE2_API ::TDEngine2::IComponent* CreateDefault() const override;																			\
			TDE2_API ::TDEngine2::E_RESULT_CODE SetupComponent(ComponentName* pComponent, const ComponentParamsType& params) const override;			\
		protected:																																		\
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(ComponentFactoryName)																				\
	}

#define TDE2_DECLARE_COMPONENT_FACTORY(ComponentName, ComponentParamsType) \
	TDE2_DECLARE_COMPONENT_FACTORY_IMPL(#ComponentName, TDE2_COMPONENT_CLASS_NAME(ComponentName), ComponentParamsType, TDE2_COMPONENT_FACTORY_NAME(ComponentName), TDE2_COMPONENT_FACTORY_FUNCTION_NAME(ComponentName))


	/*!
		\brief Macros to declare component flags
	*/


#define TDE2_DECLARE_FLAG_COMPONENT_IMPL(ComponentNameStr, ComponentName, ComponentFuncName, ComponentFactoryName, ComponentFactoryFuncName)						\
	TDE2_API ::TDEngine2::IComponent* ComponentFuncName(::TDEngine2::E_RESULT_CODE& result);																		\
																																									\
	class ComponentName : public ::TDEngine2::CBaseComponent, public ::TDEngine2::CPoolMemoryAllocPolicy<ComponentName, 1 << 10>									\
	{																																								\
		public:																																						\
			friend TDE2_API TDEngine2::IComponent* ComponentFuncName(::TDEngine2::E_RESULT_CODE&);																	\
			TDE2_REGISTER_COMPONENT_TYPE(ComponentName)																												\
																																									\
			TDE2_API ::TDEngine2::E_RESULT_CODE Init();																												\
																																									\
			TDE2_API ::TDEngine2::E_RESULT_CODE Load(::TDEngine2::IArchiveReader* pReader) override;																\
			TDE2_API ::TDEngine2::E_RESULT_CODE Save(::TDEngine2::IArchiveWriter* pWriter) override;																\
			TDE2_API ::TDEngine2::E_RESULT_CODE Clone(class ::TDEngine2::IComponent*& pDestObject) const override;													\
																																									\
			TDE2_API const std::string& GetTypeName() const override;																								\
			TDE2_API bool IsRuntimeOnly() const override;																											\
		protected:																																					\
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(ComponentName)																									\
	};																																								\
																																									\
	TDE2_DECLARE_COMPONENT_FACTORY_IMPL(ComponentNameStr, ComponentName, ::TDEngine2::TBaseComponentParameters, ComponentFactoryName, ComponentFactoryFuncName);

	/*!
		\brief The macro is used to define so called component-flag which just marks an entity and doesn't
		contain any data
	*/

#define TDE2_DECLARE_FLAG_COMPONENT(ComponentName) TDE2_DECLARE_FLAG_COMPONENT_IMPL(#ComponentName, TDE2_COMPONENT_CLASS_NAME(ComponentName),				\
																					TDE2_COMPONENT_FUNCTION_NAME(ComponentName),							\
																					TDE2_COMPONENT_FACTORY_NAME(ComponentName),								\
																					TDE2_COMPONENT_FACTORY_FUNCTION_NAME(ComponentName))

	/*!
		\brief The macro is used to define all methods which were declared using TDE2_DECLARE_FLAG_COMPONENT.
		Note that the macro should be invoked only within *.cpp files to avoid circular dependencies
	*/

#define TDE2_DEFINE_FLAG_COMPONENT_IMPL(ComponentName, ComponentFuncName, ComponentFactoryName, ComponentFactoryFuncName, IsRuntimeOnlyFlag)					\
	TDE2_REGISTER_COMPONENT_FACTORY(ComponentFactoryFuncName)																									\
																																								\
	ComponentName::ComponentName() : ::TDEngine2::CBaseComponent() { }																							\
																																								\
	::TDEngine2::E_RESULT_CODE ComponentName::Init()																											\
	{ 																																							\
		mIsInitialized = true;																																	\
		return ::TDEngine2::RC_OK;																																\
	}																																							\
																																								\
	::TDEngine2::E_RESULT_CODE ComponentName::Load(::TDEngine2::IArchiveReader* pReader)																		\
	{																																							\
		return CBaseComponent::Load(pReader);																													\
	}																																							\
																																								\
	::TDEngine2::E_RESULT_CODE ComponentName::Save(::TDEngine2::IArchiveWriter* pWriter)																		\
	{																																							\
		if (!pWriter)																																			\
		{																																						\
			return ::TDEngine2::RC_FAIL;																														\
		}																																						\
																																								\
		pWriter->BeginGroup("component");																														\
		pWriter->SetUInt32("type_id", static_cast<::TDEngine2::U32>(ComponentName::GetTypeId()));																\
		pWriter->EndGroup();																																	\
																																								\
		return ::TDEngine2::RC_OK;																																\
	}																																							\
																																								\
	::TDEngine2::E_RESULT_CODE ComponentName::Clone(::TDEngine2::IComponent*& pDestObject) const																\
	{																																							\
		return ::TDEngine2::RC_OK;																																\
	}																																							\
																																								\
	const std::string& ComponentName::GetTypeName() const																										\
	{																																							\
		static const std::string componentName(#ComponentName);																									\
		return componentName;																																	\
	}																																							\
																																								\
	bool ComponentName::IsRuntimeOnly() const { return IsRuntimeOnlyFlag; }																						\
																																								\
	::TDEngine2::IComponent* ComponentFuncName(::TDEngine2::E_RESULT_CODE& result)																				\
	{																																							\
		return CREATE_IMPL(::TDEngine2::IComponent, ComponentName, result);																						\
	}																																							\
																																								\
	ComponentFactoryName::ComponentFactoryName(): CBaseComponentFactory() { }																					\
																																								\
	::TDEngine2::IComponent* ComponentFactoryName::CreateDefault() const																						\
	{																																							\
		::TDEngine2::E_RESULT_CODE result = ::TDEngine2::RC_OK;																									\
		return ComponentFuncName(result);																														\
	}																																							\
																																								\
	::TDEngine2::E_RESULT_CODE ComponentFactoryName::SetupComponent(ComponentName* pComponent, const ::TDEngine2::TBaseComponentParameters& params) const		\
	{																																							\
		if (!pComponent)																																		\
		{																																						\
			return ::TDEngine2::RC_INVALID_ARGS;																												\
		}																																						\
																																								\
		return ::TDEngine2::RC_OK;																																\
	}																																							\
																																								\
	::TDEngine2::IComponentFactory* ComponentFactoryFuncName(::TDEngine2::E_RESULT_CODE& result)																\
	{																																							\
		return CREATE_IMPL(::TDEngine2::IComponentFactory, ComponentFactoryName, result);																		\
	}																					


#define TDE2_DEFINE_FLAG_COMPONENT(ComponentName) TDE2_DEFINE_FLAG_COMPONENT_IMPL(TDE2_COMPONENT_CLASS_NAME(ComponentName),					\
																					TDE2_COMPONENT_FUNCTION_NAME(ComponentName),			\
																					TDE2_COMPONENT_FACTORY_NAME(ComponentName),				\
																					TDE2_COMPONENT_FACTORY_FUNCTION_NAME(ComponentName), false)


#define TDE2_DEFINE_RUNTIME_ONLY_FLAG_COMPONENT(ComponentName) TDE2_DEFINE_FLAG_COMPONENT_IMPL(TDE2_COMPONENT_CLASS_NAME(ComponentName),		  \
																					TDE2_COMPONENT_FUNCTION_NAME(ComponentName),			      \
																					TDE2_COMPONENT_FACTORY_NAME(ComponentName),				      \
																					TDE2_COMPONENT_FACTORY_FUNCTION_NAME(ComponentName), true)



	/*!
		\brief The components are used to hide the entity from the processing.
		The difference between Deactivated and DeactivatedGroup could be corresponded to local and global activity of the entity in other engines.
		The Deactivated component is a kind of local state of the entity while DeactivedGroup is related with the entity's parent activity 
	*/

	TDE2_DECLARE_FLAG_COMPONENT(DeactivatedComponent)
	TDE2_DECLARE_FLAG_COMPONENT(DeactivatedGroupComponent)


#define TDE2_REGISTER_COMPONENT_PROPERTIES												\
	TDE2_API const std::string& GetTypeName() const override;							\
	TDE2_API IPropertyWrapperPtr GetProperty(const std::string& propertyName) override; \
	TDE2_API const std::vector<std::string>& GetAllProperties() const override;



	struct TComponentFactoryRegister
	{
		TDE2_API TComponentFactoryRegister(TComponentFactoryFunctor&& factory);
	};


#define TDE2_REGISTER_COMPONENT_FACTORY(ComponentFactoryFunctor) \
	static ::TDEngine2::TComponentFactoryRegister TDE2_CONCAT(ComponentFactoryRegister_, __LINE__)(ComponentFactoryFunctor);
}