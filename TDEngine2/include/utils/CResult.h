/*!
	\file CResult.h
	\date 16.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "Types.h"
#include <string>
#include <functional>


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
		union TBasicResultStorage

		\brief The union represents a basic storage of a result value. It could be
		either some useful data, or an error's object
	*/

	template <typename T, typename E>
	union TBasicResultStorage
	{
		T mData;

		E mError;
	};

	template <typename E>
	union TBasicResultStorage<void, E>
	{
		E mError;
	};


	/*!
		class CBasicResult<T, E>

		\brief The class contains a common logic of all result types
	*/

	template <typename T, typename E>
	class CBasicResult
	{
		public:
		protected:
			CBasicResult() = default;

			CBasicResult(bool hasError):
				mHasError(hasError)
			{
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
			bool mHasError;
	};


	/*!
		class CResult<T, E>

		\brief The class is a main type for error handling that the engine provides
		The implementation and interface are based on Rust's type Result<T,E>
		This implementation doesn't support deallocation of memory of a resource
		that goes out of scope, so its destructor does nothing
	*/

	template <typename T, typename E>
	class CResult: public CBasicResult<T, E>
	{
		public:
			CResult(const TOkValue<T>& okValue)
			{
				mStorage.mData = okValue.mData;
			}

			CResult(const TErrorValue<E>& errValue)
			{
				mStorage.mError = errValue.mError;
			}

			~CResult()
			{
			}

			T Get() const
			{
				if (!mHasError)
				{
					return mStorage.mData;
				}

				Panic("Try to get value that doesn't exist");
			}

			T GetOrDefault(const T& defaultValue) const
			{
				if (!mHasError)
				{
					return mStorage.mData;
				}

				return defaultValue;
			}

			CResult<T, E> AndThen(std::function<CResult<T, E>(T)> callback) const
			{
				if (!mHasError)
				{
					return callback(mStorage.mData);
				}

				return *this;
			}

			template <typename U>
			CResult<U, E> Map(std::function<CResult<U, E>(T)> converter) const
			{
				if (!mHasError)
				{
					return converter(mStorage.mData);
				}

				Panic("Try to map value that doesn't exist");
			}

			template <typename U>
			CResult<T, U> MapError(std::function<CResult<T, U>(E)> converter) const
			{
				if (mHasError)
				{
					return converter(mStorage.mError);
				}

				Panic("Try to map error's value when the result's object contains data");
			}
			
			CResult<T, E> operator= (const TOkValue<T>& okValue)
			{
				mStorage.mData = okValue.mValue;

				mHasError = false;

				return *this;
			}

			CResult<T, E> operator= (const TErrorValue<E>& errValue)
			{
				mStorage.mError = errValue.mError;

				mHasError = true;

				return *this;
			}
		protected:
			CResult():
				CBasicResult(true)
			{
			}
		protected:
			TBasicResultStorage<T, E> mStorage;
	};


	template <typename E>
	class CResult<void, E> : public CBasicResult<void, E>
	{
		public:
			CResult() :
				CBasicResult(false)
			{
			}

			CResult(const TErrorValue<E>& errValue)
			{
				mStorage.mError = errValue.mError;
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
			}

			CResult<void, E> operator= (const TErrorValue<E>& errValue)
			{
				mStorage.mError = errValue.mError;

				mHasError = true;

				return *this;
			}
		protected:
			TBasicResultStorage<void, E> mStorage;
	};


	template <typename T>
	using TResult = CResult<T, E_RESULT_CODE>;
}