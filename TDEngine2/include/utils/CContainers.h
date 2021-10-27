/*!
	\file CContainers.h
	\date 23.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "Config.h"
#include "Types.h"
#include "../core/memory/IAllocator.h"
#include <cstring>


namespace TDEngine2
{
	/*!
		class StaticArray

		\brief The class is an implementation of fixed-size arrays the same
		as std::array, but wihout specifying size as template argument, which
		makes its usage within virtual methods impossible

		It's better to consider this class as wrappen which is a so called "fat array pointer",
		not an actual array
	*/

	template <typename T>
	class CStaticArray
	{
		public:
			typedef T   Type;
			typedef U64 SizeType;
		public:
			CStaticArray() = delete;
			CStaticArray(const CStaticArray<T>& arr);
			CStaticArray(CStaticArray<T>&& arr);
			CStaticArray(const Type* pBuffer, SizeType size);
			CStaticArray(const std::initializer_list<Type>& elements);
			~CStaticArray() = default;

			inline SizeType GetSize() const;
			inline bool IsEmpty() const;

			inline Type& operator[](SizeType index);
			inline const Type& operator[](SizeType index) const;

			CStaticArray<T>& operator= (const CStaticArray<T>& arr);
			CStaticArray<T>& operator= (CStaticArray<T>&& arr);
		protected:
			const Type* const mpBuffer;
			const SizeType mSize;
	};


	template <typename T>
	CStaticArray<T>::CStaticArray(const CStaticArray<T>& arr) :
		mpBuffer(arr.mpBuffer), mSize(arr.mSize)
	{
	}

	template <typename T>
	CStaticArray<T>::CStaticArray(CStaticArray<T>&& arr) :
		mpBuffer(arr.mpBuffer), mSize(arr.mSize)
	{
		arr.mpBuffer = nullptr;
		arr.mSize = 0;
	}

	template <typename T>
	CStaticArray<T>::CStaticArray(const Type* pBuffer, SizeType size) :
		mpBuffer(pBuffer), mSize(size)
	{
	}

	template <typename T>
	CStaticArray<T>::CStaticArray(const std::initializer_list<Type>& elements) :
		mpBuffer(elements.begin()), mSize(std::distance(elements.begin(), elements.end()))
	{
	}

	template <typename T>
	typename CStaticArray<T>::SizeType CStaticArray<T>::GetSize() const
	{
		return mSize;
	}

	template <typename T>
	bool CStaticArray<T>::IsEmpty() const
	{
		return !mSize;
	}

	template <typename T>
	const typename CStaticArray<T>::Type& CStaticArray<T>::operator[](SizeType index) const
	{
		TDE2_ASSERT(index >= 0 && index < mSize);

		return mpBuffer[index];
	}

	template <typename T>
	typename CStaticArray<T>::Type& CStaticArray<T>::operator[](SizeType index)
	{
		TDE2_ASSERT(index >= 0 && index < mSize);

		return mpBuffer[index];
	}

	template <typename T>
	CStaticArray<T>& CStaticArray<T>::operator= (const CStaticArray<T>& arr)
	{
		mpBuffer = arr.mpBuffer;
		mSize = arr.mSize;

		return *this;
	}

	template <typename T>
	CStaticArray<T>& CStaticArray<T>::operator= (CStaticArray<T>&& arr)
	{
		mpBuffer = arr.mpBuffer;
		mSize = arr.mSize;

		arr.mpBuffer = nullptr;
		arr.mSize = 0;

		return *this;
	}


	/*!
		class CDynamicArray

		\brief The class is a replacement of std::vector for high performant tasks,
		where the first one shows unacceptible results
	*/

	template <typename T>
	class CDynamicArray
	{
		public:
			typedef T   Type;
			typedef U64 SizeType;
		public:
			CDynamicArray(IAllocator& allocator, SizeType capacity = 0);
			CDynamicArray(const CDynamicArray<T>& arr);
			CDynamicArray(CDynamicArray<T>&& arr);
			~CDynamicArray();

			void PushBack(Type element);

			void Clear();

			inline SizeType GetSize() const;
			inline bool IsEmpty() const;

			inline Type& operator[](SizeType index);
			inline const Type& operator[](SizeType index) const;

			CDynamicArray<T>& operator= (const CDynamicArray<T>& arr);
			CDynamicArray<T>& operator= (CDynamicArray<T>&& arr);
		private:
			void _destroyElements(Type* pBuffer, SizeType count);

			void _grow();
		private:
			IAllocator* mpAllocator;
			Type*       mpBuffer;
			SizeType    mSize;
			SizeType    mCapacity;
	};


	template <typename T>
	CDynamicArray<T>::CDynamicArray(IAllocator& allocator, SizeType capacity):
		mpAllocator(&allocator), mCapacity(static_cast<U64>(capacity)), mSize(0), mpBuffer(static_cast<Type*>(allocator.Allocate(static_cast<U32>(sizeof(T) * capacity), __alignof(T))))
	{
	}

	template <typename T>
	CDynamicArray<T>::CDynamicArray(const CDynamicArray<T>& arr) :
		mpAllocator(arr.mpAllocator), mCapacity(arr.mCapacity), mSize(arr.mSize)
	{
		mpBuffer = static_cast<Type*>(mpAllocator->Allocate(sizeof(T) * mCapacity, __alignof(T)));

		memcpy(mpBuffer, arr.mpBuffer, sizeof(T) * mCapacity);
	}

	template <typename T>
	CDynamicArray<T>::CDynamicArray(CDynamicArray<T>&& arr) :
		mpAllocator(arr.mpAllocator), mCapacity(arr.mCapacity), mSize(arr.mSize), mpBuffer(arr.mpBuffer)
	{
	}

	template <typename T>
	CDynamicArray<T>::~CDynamicArray()
	{
		_destroyElements(mpBuffer, mSize); /// \note call destructors for all elements

		TDE2_ASSERT(mpAllocator->Deallocate(mpBuffer) == RC_OK);
	}

	template <typename T>
	void CDynamicArray<T>::PushBack(Type element)
	{
		if (mSize + 1 > mCapacity)
		{
			_grow();
		}

		mpBuffer[mSize++] = std::move(element);
	}

	template <typename T>
	void CDynamicArray<T>::Clear()
	{
		mSize = 0;
	}

	template <typename T>
	typename CDynamicArray<T>::SizeType CDynamicArray<T>::GetSize() const
	{
		return mSize;
	}

	template <typename T>
	bool CDynamicArray<T>::IsEmpty() const
	{
		return !mSize;
	}

	template <typename T>
	const typename CDynamicArray<T>::Type& CDynamicArray<T>::operator[](SizeType index) const
	{
		TDE2_ASSERT(index >= 0 && index < mSize);

		return mpBuffer[index];
	}

	template <typename T>
	typename CDynamicArray<T>::Type& CDynamicArray<T>::operator[](SizeType index)
	{
		TDE2_ASSERT(index >= 0 && index < mSize);

		return mpBuffer[index];
	}

	template <typename T>
	CDynamicArray<T>& CDynamicArray<T>::operator= (const CDynamicArray<T>& arr)
	{
		mpBuffer = arr.mpBuffer;
		mSize = arr.mSize;
		mCapacity = arr.mCapacity;
		mpAllocator = arr.mpAllocator;

		return *this;
	}

	template <typename T>
	CDynamicArray<T>& CDynamicArray<T>::operator= (CDynamicArray<T>&& arr)
	{
		mpAllocator = arr.mpAllocator;

		mSize = arr.mSize;
		mCapacity = arr.mCapacity;
		
		mpBuffer = static_cast<Type*>(mpAllocator->Allocate(sizeof(T) * mCapacity, __alignof(T)));

		memcpy(mpBuffer, arr.mpBuffer, sizeof(T) * mCapacity);

		arr.mpBuffer = nullptr;
		arr.mSize = 0;
		arr.mCapacity = 0;

		return *this;
	}

	template <typename T>
	void CDynamicArray<T>::_destroyElements(Type* pBuffer, SizeType count)
	{
		Type* pCurrElement = nullptr;

		for (SizeType i = 0; i < count; ++i)
		{
			if (!(pCurrElement = &pBuffer[i]))
			{
				continue;
			}

			pCurrElement->~Type();
		}
	}

	template <typename T>
	void CDynamicArray<T>::_grow()
	{
		mCapacity = (mCapacity < 8) ? 8 : (2 * mCapacity + 8);
		
		TDE2_ASSERT(mpAllocator->Deallocate(mpBuffer) == RC_OK);

		Type* pNewBuffer = static_cast<Type*>(mpAllocator->Allocate(static_cast<U32>(mCapacity * sizeof(T)), __alignof(T)));

		memcpy(pNewBuffer, mpBuffer, static_cast<size_t>(mSize));

		mpBuffer = pNewBuffer;
	}
}