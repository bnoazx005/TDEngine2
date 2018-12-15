/*!
	\file CResult.h
	\date 16.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "Types.h"
#include <string>
#include <functional>
#include <stdexcept>


namespace TDEngine2
{
	/*!
		\brief The function stops execution of a program
		The method is used to terminate a program when some error
		occurs instead of trying to solve it

		\param[in] message A message that will be printed as error output
	*/

	TDE2_API void Panic(const std::string& message);


	/*!
		struct TOkValue<T>

		\brief The structure contains some useful data. The type is
		used to create CResult<T, E> objects
	*/

	template <typename T>
	struct TOkValue
	{
		TOkValue(const T& value) :
			mData(value)
		{
		}

		T mData;
	};

	template <typename T>
	struct TOkValue<T*>
	{
		TOkValue(T* value) :
			mData(value)
		{
		}

		T* mData;
	};

	template <>
	struct TOkValue<void>
	{
	};


	/*!
		struct TErrorValue<E>

		\brief The structure contains data that represents an error. The type is
		used to create CResult<T, E> objects
	*/

	template <typename E>
	struct TErrorValue
	{
		TErrorValue(const E& error) :
			mError(error)
		{
		}

		E mError;
	};

	template <typename E>
	struct TErrorValue<E*>
	{
		TErrorValue(E* error) :
			mError(error)
		{
		}

		E mError;
	};


	/*!
		class CBasicResultStorage

		\brief The class represents a basic storage of a result value. It could be
		either some useful data, or an error's object
	*/

	template <typename T, typename E>
	class CBasicResultStorage
	{
		public:
			CBasicResultStorage():
				mHasError(false), mIsInitialized(false)
			{
			}

			CBasicResultStorage(const TOkValue<T>& okValue):
				mHasError(false), mIsInitialized(true)
			{
				new (&mValue.mData) T(okValue.mData);
			}

			CBasicResultStorage(const TErrorValue<E>& errValue) :
				mHasError(true), mIsInitialized(true)
			{
				new (&mValue.mError) E(errValue.mError);
			}

			CBasicResultStorage(const CBasicResultStorage<T, E>& storage):
				mHasError(storage.mHasError), mIsInitialized(storage.mIsInitialized)
			{
				_initialize(storage);
			}

			CBasicResultStorage(CBasicResultStorage<T, E>&& storage) :
				mHasError(storage.mHasError), mIsInitialized(storage.mIsInitialized)
			{
				_initialize(storage);
			}

			~CBasicResultStorage()
			{
				_release();
			}

			T GetData() const
			{
				return mValue.mData;
			}

			E GetError() const
			{
				return mValue.mError;
			}

			CBasicResultStorage<T, E> operator= (const TOkValue<T>& okValue)
			{
				new (&mValue.mData) T(okValue.mData);

				mIsInitialized = true;

				mHasError = false;

				return *this;
			}

			CBasicResultStorage<T, E> operator= (const TErrorValue<E>& errValue)
			{
				new (&mValue.mError) E(errValue.mError);

				mIsInitialized = true;

				mHasError = true;

				return *this;
			}
		protected:
			union TVariantStorage
			{
				TVariantStorage() {}

				~TVariantStorage() {}

				T mData;

				E mError;
			} mValue;
		protected:
			void _release()
			{
				if (!mIsInitialized)
				{
					return;
				}

				if (mHasError)
				{
					mValue.mError.~E();
					
					return;
				}

				mValue.mData.~T();

				mHasError = false;
			}

			void _initialize(const CBasicResultStorage<T, E>& storage)
			{
				if (storage.mHasError)
				{
					new (&mValue.mError) E(storage.mValue.mError);

					return;
				}

				new (&mValue.mData) T(storage.mValue.mData);

				mIsInitialized = true;
			}

			void _initialize(CBasicResultStorage<T, E>&& storage)
			{
				if (storage.mHasError)
				{
					new (&mValue.mError) E(storage.mValue.mError);
					
					storage.mHasError      = false;
					storage.mIsInitialized = false;
					
					memset(&mValue.mError, 0, sizeof(mValue.mError));

					return;
				}

				new (&mValue.mData) T(storage.mValue.mData);
				
				storage.mHasError = false;
				storage.mIsInitialized = false;

				memset(&mValue.mData, 0, sizeof(mValue.mData));

				mIsInitialized = true;
			}
		protected:
			bool mIsInitialized;

			bool mHasError;
	};

	template <typename E>
	class CBasicResultStorage<void, E>
	{
	public:
		CBasicResultStorage() :
			mHasError(false), mIsInitialized(false)
		{
		}

		CBasicResultStorage(const TErrorValue<E>& errValue) :
			mHasError(true), mIsInitialized(true), mError(errValue.mError)
		{
		}

		CBasicResultStorage(const CBasicResultStorage<void, E>& storage) :
			mHasError(storage.mHasError), mIsInitialized(storage.mIsInitialized), mError(storage.mError)
		{
		}

		CBasicResultStorage(CBasicResultStorage<void, E>&& storage) :
			mHasError(storage.mHasError), mIsInitialized(storage.mIsInitialized), mError(storage.mError)
		{
		}

		~CBasicResultStorage()
		{
		}

		E GetError() const
		{
			return mError;
		}

		CBasicResultStorage<void, E> operator= (const TErrorValue<E>& errValue)
		{
			mError = errValue.mError;

			mIsInitialized = true;

			mHasError = true;

			return *this;
		}
	protected:
		void _release()
		{
			if (!mIsInitialized)
			{
				return;
			}

			if (mHasError)
			{
				mError.~E();

				return;
			}

			mHasError = false;
		}
	protected:
		bool mIsInitialized;

		bool mHasError;

		E    mError;
	};


	/*!
		class CResult<T, E>

		\brief The class is a main type for error handling that the engine provides
		The implementation and interface are based on Rust's type Result<T,E>
		This implementation doesn't support deallocation of memory of a resource
		that goes out of scope, so its destructor does nothing
	*/

	template <typename T, typename E>
	class CResult
	{
		public:
			CResult(const TOkValue<T>& okValue):
				mHasError(false), mStorage(okValue)
			{
			}

			CResult(const TErrorValue<E>& errValue):
				mHasError(true), mStorage(errValue)
			{
			}

			CResult(const CResult<T, E>& result):
				mHasError(result.mHasError), mStorage(result.mStorage)
			{
			}

			CResult(CResult<T, E>&& result) :
				mHasError(result.mHasError), mStorage(result.mStorage)
			{
			}

			~CResult()
			{
			}

			T Get() const
			{
				if (!mHasError)
				{
					return mStorage.GetData();
				}

				Panic("Try to get value that doesn't exist");

				throw std::runtime_error("Try to get value that doesn't exist");
			}

			T GetOrDefault(const T& defaultValue) const
			{
				if (!mHasError)
				{
					return mStorage.GetData();
				}

				return defaultValue;
			}

			CResult<T, E> AndThen(std::function<CResult<T, E>(T)> callback) const
			{
				if (!mHasError)
				{
					return callback(mStorage.GetData());
				}

				return *this;
			}

			E GetError() const
			{
				if (!mHasError)
				{
					Panic("Try to get error when it doesn't exist");
				}

				return mStorage.GetError();
			}

			template <typename U>
			CResult<U, E> Map(std::function<CResult<U, E>(T)> converter) const
			{
				if (!mHasError)
				{
					return converter(mStorage.GetData());
				}

				Panic("Try to map value that doesn't exist");

				throw std::runtime_error("Try to map value that doesn't exist");
			}

			template <typename U>
			CResult<T, U> MapError(std::function<CResult<T, U>(E)> converter) const
			{
				if (mHasError)
				{
					return converter(mStorage.GetError());
				}

				Panic("Try to map error's value when the result's object contains data");

				throw std::runtime_error("Try to map error's value when the result's object contains data");
			}
			
			CResult<T, E> operator= (const TOkValue<T>& okValue)
			{
				mStorage = okValue;

				mHasError = false;

				return *this;
			}

			CResult<T, E> operator= (const TErrorValue<E>& errValue)
			{
				mStorage = errValue;

				mHasError = true;

				return *this;
			}

			/*!
				\brief The method returns true if a result object contains some useful data

				\return The method returns true if a result object contains some useful data
			*/

			bool IsOk() const
			{
				return !mHasError;
			}

			/*!
				\brief The method returns true if a result object contains some error

				\return The method returns true if a result object contains some error
			*/

			bool HasError() const
			{
				return mHasError;
			}
		protected:
			CResult():
				mHasError(true)
			{
			}
		protected:
			CBasicResultStorage<T, E> mStorage;

			bool                      mHasError;
	};


	template <typename E>
	class CResult<void, E>
	{
		public:
			CResult() :
				mHasError(false)
			{
			}

			CResult(const TErrorValue<E>& errValue):
				mHasError(true), mStorage(errValue)
			{
			}

			~CResult()
			{
			}

			template <typename U>
			CResult<void, U> MapError(std::function<CResult<void, U>(E)> converter) const
			{
				if (mHasError)
				{
					return converter(mStorage.mError);
				}

				Panic("Try to map error's value when the result's object contains data");

				throw std::runtime_error("Try to map error's value when the result's object contains data");
			}

			CResult<void, E> operator= (const TErrorValue<E>& errValue)
			{
				mStorage = errValue;

				mHasError = true;

				return *this;
			}

			/*!
				\brief The method returns true if a result object contains some useful data

				\return The method returns true if a result object contains some useful data
			*/

			bool IsOk() const
			{
				return mHasError;
			}

			/*!
				\brief The method returns true if a result object contains some error

				\return The method returns true if a result object contains some error
			*/

			bool HasError() const
			{
				return mHasError;
			}
		protected:
			CBasicResultStorage<void, E> mStorage;

			bool                         mHasError;
	};


	template <typename T>
	using TResult = CResult<T, E_RESULT_CODE>;
}