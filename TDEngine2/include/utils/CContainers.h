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
#include <mutex>
#include <condition_variable>
#include <atomic>


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


	/*!
		class CFixedVector<T, USIZE>

		\brief The type represents std::vector<T> functionality but with predefined maximal capacity. All the elements are stored on stack
		and API is std::vector<T>'s compatible except resize and reserve methods
	*/

	template <typename T, USIZE maxCapacity = 16>
	class CFixedVector
	{
		public:
			typedef T                                     value_type;
			typedef USIZE                                 size_type;
			typedef std::ptrdiff_t                        difference_type;
			typedef value_type&                           reference;
			typedef const value_type&                     const_reference;
			typedef value_type*                           pointer;
			typedef const value_type*                     const_pointer;
			typedef pointer                               iterator;
			typedef const_pointer                         const_iterator;
			typedef std::reverse_iterator<iterator>       reverse_iterator;
			typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		public:
			CFixedVector() noexcept = default;

			CFixedVector(size_type count) noexcept:
				mSize(count)
			{
				TDE2_ASSERT(count <= maxCapacity);
			}

			CFixedVector(size_type count, const_reference value) noexcept:
				mSize(count)
			{
				TDE2_ASSERT(count <= maxCapacity);
				std::fill(mElementsStorage.begin(), mElementsStorage.end(), value);
			}

			CFixedVector(std::initializer_list<value_type> initializer) noexcept:
				mSize(initializer.size())
			{
				TDE2_ASSERT(mSize <= maxCapacity);
				std::uninitialized_copy(initializer.begin(), initializer.end(), mElementsStorage.begin());
			}

			CFixedVector(const CFixedVector& other) noexcept:
				mSize(other.mSize)
			{
				std::uninitialized_copy(other.mElementsStorage.begin(), other.mElementsStorage.end(), mElementsStorage.begin());
			}

			CFixedVector(CFixedVector&& other) noexcept :
				mSize(other.mSize),
				mElementsStorage(std::move(other.mElementsStorage))
			{
			}

			~CFixedVector()
			{
				mSize = 0;
			}

			reference at(size_type index)
			{
				if (index < mSize)
				{
					return mElementsStorage[index];
				}

				TDE2_ASSERT(index < mSize);
				return back();
			}

			const_reference at(size_type index) const
			{
				if (index < mSize)
				{
					return mElementsStorage[index];
				}

				TDE2_ASSERT(index < mSize);
				return back();
			}

			reference operator[](size_type index) { return at(index); }
			const_reference operator[](size_type index) const { return at(index); }

			reference front() noexcept { return mElementsStorage.front(); }
			const_reference front() const noexcept { return mElementsStorage.front(); }

			reference back() noexcept { return mElementsStorage[mSize - 1]; }
			const_reference back() const noexcept { return mElementsStorage[mSize - 1]; }

			pointer data() noexcept { return mElementsStorage.data(); }
			const_pointer data() const noexcept { return mElementsStorage.data(); }

			iterator begin() noexcept { return &mElementsStorage[0]; }
			const_iterator cbegin() const noexcept { return &mElementsStorage[0]; }
			reverse_iterator rbegin() noexcept { return mElementsStorage.begin() + mSize; }
			const_reverse_iterator crbegin() const noexcept { return mElementsStorage.cbegin() + mSize; }

			iterator end() noexcept { return data() + mSize; }
			const_iterator cend() const noexcept { return data() + mSize; }
			reverse_iterator rend() noexcept { return mElementsStorage.rbegin(); }
			const_reverse_iterator crend() const noexcept { return mElementsStorage.crbegin(); }

			CFixedVector& operator= (const CFixedVector& other) noexcept
			{
				if (&other == this)
				{
					return *this;
				}

				clear();

				mSize = other.mSize;
				std::uninitialized_copy(other.mElementsStorage.begin(), other.mElementsStorage.end(), mElementsStorage.begin());

				return *this;
			}

			CFixedVector& operator= (CFixedVector&& other) noexcept
			{
				if (&other == this)
				{
					return *this;
				}

				clear();

				mSize = other.mSize;
				mElementsStorage = std::move(other.mElementsStorage);

				return *this;
			}

			size_type size() const noexcept { return mSize; }

			bool empty() const noexcept { return !mSize; }

			size_type capacity() const noexcept { return maxCapacity; }

			void clear()
			{
				if (!std::is_trivially_destructible<value_type>::value)
				{
					for (auto& currValue : mElementsStorage)
					{
						currValue.~value_type();
					}
				}

				mSize = 0;
			}

			iterator insert(const_iterator pos, const value_type& value) noexcept
			{
				if (mSize >= maxCapacity)
				{
					TDE2_ASSERT(false);
					return end();
				}

				const size_type index = static_cast<size_type>(std::distance(cbegin(), pos));
				mElementsStorage[index] = value;

				++mSize;

				return begin() + index;
			}

			iterator insert(const_iterator pos, value_type&& value) noexcept
			{
				if (mSize >= maxCapacity)
				{
					TDE2_ASSERT(false);
					return end();
				}

				const size_type index = static_cast<size_type>(std::distance(cbegin(), pos));
				mElementsStorage[index] = std::move(value);

				++mSize;

				return begin() + index;
			}

			template <typename... TArgs>
			iterator emplace(const_iterator pos, TArgs&&... args) noexcept
			{
				if (mSize >= maxCapacity)
				{
					TDE2_ASSERT(false);
					return end();
				}

				const size_type index = static_cast<size_type>(std::distance(cbegin(), pos));
				new (&mElementsStorage[index]) value_type(std::forward<TArgs>(args)...);

				++mSize;

				return begin() + index;
			}

			iterator erase(const_iterator pos) noexcept
			{
				const size_type index = static_cast<size_type>(std::distance(cbegin(), pos));
				if (index >= mSize)
				{
					TDE2_ASSERT(false);
					return end();
				}

				mElementsStorage[index].~value_type();
				std::move(begin() + index + 1, end(), begin() + index);
				--mSize;

				return begin() + index;
			}

			iterator push_back(const value_type& value) noexcept
			{
				if (mSize >= maxCapacity)
				{
					TDE2_ASSERT(false);
					return end();
				}

				mElementsStorage[mSize++] = value;

				return &mElementsStorage[mSize - 1];
			}

			iterator push_back(value_type&& value) noexcept
			{
				if (mSize >= maxCapacity)
				{
					TDE2_ASSERT(false);
					return end();
				}

				mElementsStorage[mSize++] = std::move(value);

				return &mElementsStorage[mSize - 1];
			}

			template <typename... TArgs>
			iterator emplace_back(TArgs... args) noexcept
			{
				if (mSize >= maxCapacity)
				{
					TDE2_ASSERT(false);
					return end();
				}

				new (&mElementsStorage[mSize++]) value_type(std::forward<TArgs>(args)...);

				return &mElementsStorage[mSize - 1];
			}
		private:
			std::array<T, maxCapacity> mElementsStorage{};
			size_type                  mSize = 0;
	};


	template <typename TType>
	inline USIZE PlaceObjectAtFirstNullPosition(std::vector<TPtr<TType>>& container, TPtr<TType> pObject)
	{
		const USIZE placementIndex = static_cast<USIZE>(std::distance(container.cbegin(), std::find(container.cbegin(), container.cend(), nullptr)));

		if (placementIndex >= container.size())
		{
			container.emplace_back(pObject);
			return placementIndex;
		}

		container[placementIndex] = pObject;

		return placementIndex;
	}


	template <typename T, USIZE maxCapacity = 16>
	class CFixedEventQeueue
	{
		public:
			void Push(T&& value)
			{
				std::scoped_lock<std::mutex> lock(mMutex);

				mEvents.insert(mEvents.begin() + mTailPos, std::forward<T>(value));
				mTailPos = (mTailPos + 1) & (maxCapacity - 1);

				mCondVariable.notify_one();
			}

			template <typename... TArgs>
			void Emplace(TArgs&&... args)
			{
				std::scoped_lock<std::mutex> lock(mMutex);

				mEvents.emplace(mEvents.begin() + mTailPos, std::forward<TArgs>(args)...);
				mTailPos = (mTailPos + 1) & (maxCapacity - 1);

				mCondVariable.notify_one();
			}

			T& TryPop()
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mCondVariable.wait(lock, [this] { return !mEvents.empty(); });

				const USIZE lastHeadPos = mHeadPos;
				mHeadPos = (mHeadPos + 1) & (maxCapacity - 1);

				return mEvents.at(static_cast<USIZE>(lastHeadPos & (maxCapacity - 1)));
			}

			bool IsEmpty() const
			{
				return mHeadPos == mTailPos;
			}
		private:
			CFixedVector<T, maxCapacity> mEvents{};
			std::mutex                   mMutex{};
			std::condition_variable      mCondVariable{};
			std::atomic<USIZE>           mHeadPos = 0;
			std::atomic<USIZE>           mTailPos = 0;
	};
}