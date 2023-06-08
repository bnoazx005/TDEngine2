/*!
	\file Meta.h
	\date 22.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include <functional>
#include <string>
#include <vector>


namespace TDEngine2
{
	/*!
		interface IPropertyWrapper

		\brief The interface describes an element that allows to get an access to internal values of some component
	*/

	class IPropertyWrapper : public virtual IBaseObject
	{
		public:
			template <typename T>
			E_RESULT_CODE Set(const T& value)
			{
				return _setInternal(static_cast<const void*>(&value), sizeof(value));
			}

			template <typename T>
			const T& Get() const
			{
				return *static_cast<const T*>(_getInternal());
			}

			TDE2_API virtual TypeId GetValueType() const = 0;

			TDE2_API virtual bool operator== (const CScopedPtr<IPropertyWrapper>& property) const = 0;
			TDE2_API virtual bool operator!= (const CScopedPtr<IPropertyWrapper>& property) const = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPropertyWrapper)

			TDE2_API virtual E_RESULT_CODE _setInternal(const void* pValue, size_t valueSize) = 0;
			TDE2_API virtual const void* _getInternal() const = 0;
	};


	typedef CScopedPtr<IPropertyWrapper> IPropertyWrapperPtr;


	/*!
		\brief The simple property wrapper's implementation via lambdas that have access to internal members of some class.
		Note that you won't get the access like described above if you create these wrappers outside of a class.

		\todo Also another problem that should be discussed here is safety. Because of template methods Set/Get in the interface 
		now we can't be sure that user doesn't mess up with types, etc.
	*/

	template <typename TValueType>
	class CBasePropertyWrapper : public IPropertyWrapper, public CBaseObject
	{
		public:
			typedef std::function<E_RESULT_CODE(const TValueType&)> TPropertySetterFunctor;
			typedef std::function<const TValueType*()> TPropertyGetterFunctor;

		public:
			static TDE2_API IPropertyWrapper* Create(const TPropertySetterFunctor& setter, const TPropertyGetterFunctor& getter)
			{
				return new (std::nothrow) CBasePropertyWrapper<TValueType>(setter, getter);
			}

			TDE2_API TypeId GetValueType() const override
			{
				return GetTypeId<TValueType>::mValue;
			}

			TDE2_API bool operator== (const IPropertyWrapperPtr& property) const override
			{
				if (!property)
				{
					return false;
				}

				if (property->GetValueType() != GetValueType())
				{
					return false;
				}

				return property->Get<TValueType>() == Get<TValueType>();
			}

			TDE2_API bool operator!= (const IPropertyWrapperPtr& property) const override
			{
				return !(*this == property);
			}

		protected:
			CBasePropertyWrapper(const TPropertySetterFunctor& setter, const TPropertyGetterFunctor& getter) : CBaseObject(), mSetterFunc(setter), mGetterFunc(getter) 
			{
				mIsInitialized = true;
			}

			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBasePropertyWrapper)

			TDE2_API E_RESULT_CODE _setInternal(const void* pValue, size_t valueSize)
			{
				return mSetterFunc ? mSetterFunc(*static_cast<const TValueType*>(pValue)) : RC_FAIL;
			}

			TDE2_API const void* _getInternal() const
			{
				return mGetterFunc ? mGetterFunc() : nullptr;
			}
		protected:
			TPropertySetterFunctor mSetterFunc;
			TPropertyGetterFunctor mGetterFunc;
	};


	template <typename TValueType> CBasePropertyWrapper<TValueType>::CBasePropertyWrapper() : CBaseObject() {}


	/*!
		\brief All classes that want to provide some level of reflection for a user should implement this interface
	*/

	class IIntrospectable
	{
		public:
			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API virtual const std::string& GetTypeName() const = 0;

			/*!
				\return The method returns a pointer to a type's property if the latter does exist or null pointer in other cases
			*/

			TDE2_API virtual IPropertyWrapperPtr GetProperty(const std::string& propertyName) = 0;

			/*!
				\brief The method returns an array of properties names that are available for usage
			*/

			TDE2_API virtual const std::vector<std::string>& GetAllProperties() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IIntrospectable)
	};


	class IWorld;
	class CEntity;
	class CEntityManager;

	/*!
	*	
	*	\brief The method resolved given binding path looking downwards entity's hierarchy
	
		The format of input binding is 
		Entity_1 / ... / Entity_N.ComponentName.property

	* 
	*/ 

	TDE2_API IPropertyWrapperPtr ResolveBinding(IWorld* pWorld, CEntity* pEntity, const std::string& path);
	TDE2_API IPropertyWrapperPtr ResolveBinding(CEntityManager* pEntityManager, CEntity* pEntity, const std::string& path);
}