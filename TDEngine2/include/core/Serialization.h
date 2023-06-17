/*!
	\file Serialization.h
	\date 29.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "Meta.h"
#include <string>
#include <memory>


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
	template <typename T> TResult<T> Deserialize(IArchiveReader* pReader) { return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL); }

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

	/*!
		\brief deserialization helpers for built-in types
	*/

	template <> TDE2_API TResult<I8> Deserialize<I8>(IArchiveReader* pReader);
	template <> TDE2_API TResult<I16> Deserialize<I16>(IArchiveReader* pReader);
	template <> TDE2_API TResult<I32> Deserialize<I32>(IArchiveReader* pReader);
	template <> TDE2_API TResult<I64> Deserialize<I64>(IArchiveReader* pReader);
									   
	template <> TDE2_API TResult<U8> Deserialize<U8>(IArchiveReader* pReader);
	template <> TDE2_API TResult<U16> Deserialize<U16>(IArchiveReader* pReader);
	template <> TDE2_API TResult<U32> Deserialize<U32>(IArchiveReader* pReader);
	template <> TDE2_API TResult<U64> Deserialize<U64>(IArchiveReader* pReader);
									   
	template <> TDE2_API TResult<F32> Deserialize<F32>(IArchiveReader* pReader);
	template <> TDE2_API TResult<F64> Deserialize<F64>(IArchiveReader* pReader);
									   
	template <> TDE2_API TResult<bool> Deserialize<bool>(IArchiveReader* pReader);
	template <> TDE2_API TResult<std::string> Deserialize<std::string>(IArchiveReader* pReader);


	class IPropertyWrapper;


	struct IValueConcept : ISerializable
	{
		TDE2_API virtual ~IValueConcept() = default;

		template <typename U>
		const U* Get() const
		{
			return static_cast<const U*>(GetInternal(::TDEngine2::GetTypeId<U>::mValue));
		}

		TDE2_API virtual E_RESULT_CODE Apply(TPtr<IPropertyWrapper> pPropertyWrapper) = 0;
		TDE2_API virtual E_RESULT_CODE Set(TPtr<IPropertyWrapper> pPropertyWrapper) = 0;

		TDE2_API virtual std::unique_ptr<IValueConcept> Clone() = 0;

		TDE2_API virtual TypeId GetTypeId() const = 0;

		TDE2_API virtual const void* GetInternal(TypeId inputTypeId) const = 0;
	};


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


	template <typename T>
	struct CTypedValue : IValueConcept
	{
		explicit CTypedValue(T&& value) :
			mValue(std::forward<T>(value)), mTypeId(::TDEngine2::GetTypeId<T>::mValue)
		{
		}

		std::unique_ptr<IValueConcept> Clone() override
		{
			return std::make_unique<CTypedValue<T>>(std::forward<T>(mValue));
		}

		E_RESULT_CODE Apply(TPtr<IPropertyWrapper> pPropertyWrapper)
		{
			return pPropertyWrapper->template Set<T>(mValue);
		}

		E_RESULT_CODE Set(TPtr<IPropertyWrapper> pPropertyWrapper)
		{
			mValue = pPropertyWrapper->template Get<T>();
			return RC_OK;
		}

		const void* GetInternal(TypeId inputTypeId) const
		{
			if (::TDEngine2::GetTypeId<T>::mValue != inputTypeId)
			{
				TDE2_ASSERT(::TDEngine2::GetTypeId<T>::mValue != inputTypeId);
				return nullptr;
			}

			return static_cast<const void*>(&mValue);
		}

		/*!
			\brief The method deserializes object's state from given reader

			\param[in, out] pReader An input stream of data that contains information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		E_RESULT_CODE Load(IArchiveReader* pReader) override
		{
			auto result = Deserialize<T>(pReader);
			if (result.HasError())
			{
				return result.GetError();
			}

			mValue = result.Get();

			return RC_OK;
		}

		/*!
			\brief The method serializes object's state into given stream

			\param[in, out] pWriter An output stream of data that writes information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		E_RESULT_CODE Save(IArchiveWriter* pWriter) override
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(mTypeId));
			return Serialize<T>(pWriter, mValue);
		}

		TypeId GetTypeId() const override
		{
			return mTypeId;
		}

		T mValue;
		TypeId mTypeId;
	};


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
			typedef std::function<const TValueType* ()> TPropertyGetterFunctor;

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
		class CValueWrapper

		\brief The class allows to store values of many in-engine types and work with them in common fashion.

		Two requirements should be satisfied:
			- type should provide implementation of meta-function GetTypeId
			- type should be serializable (either Serialize/Deserialize overloads provided or type derives ISerializable
	*/

	class CValueWrapper: public ISerializable
	{
		public:
			TDE2_API CValueWrapper() = default;
			TDE2_API CValueWrapper(const CValueWrapper& object);
			TDE2_API CValueWrapper(CValueWrapper&& object);

			template <typename T>
			explicit CValueWrapper(T&& value):
				mpImpl(new CTypedValue<T>(std::forward<T>(value)))
			{
			}

			/*!
				\brief The method assigns internal wrapper's value into given pPropertyWrapper object	

				\param[out] pPropertyWrapper Non empty object of IPropertyWrapper type the value of the wrapper will be assigned into

				\return RC_OK if everything went ok, or some other code, which describes an error			
			*/
			TDE2_API E_RESULT_CODE Apply(TPtr<IPropertyWrapper> pPropertyWrapper);

			/*!
				\brief The method assigns a value from IPropertyWrapper into the object. If the TypeId of IPropertyWrapper and
				TypeId of the wrapper aren't same the previous one will be replaced with the newer
			*/

			TDE2_API E_RESULT_CODE Set(TPtr<IPropertyWrapper> pPropertyWrapper);

			template <typename U>
			const U* CastTo() const
			{
				return mpImpl ? mpImpl->Get<U>() : nullptr;
			}

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

			TDE2_API TypeId GetTypeId() const;

			TDE2_API CValueWrapper& operator= (CValueWrapper object);
					
			
		private:
			std::unique_ptr<IValueConcept> mpImpl = nullptr;
	};


	/*!
		\brief The function tries to read value and create its representation in CValueWrapper. That one
		simplifies work with values that can have different types
	*/

	TDE2_API TResult<CValueWrapper> DeserializeValue(IArchiveReader* pReader);
}