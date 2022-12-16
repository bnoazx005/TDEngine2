/*!
	\file CResourceContainer.h
	\date 02.02.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "Types.h"
#include <vector>
#include <list>
#include <mutex>
#include <climits>
#include <algorithm>
#include "Utils.h"


namespace TDEngine2
{
	/*!
		class CResourceContainer

		\brief The class implements a common container for resources of different types
		including files, textures, entities, components, etc...
		An instance of the class is thread-safe.

		\todo For now it doesn't work with C-style arrays or any other non-assignable type
	*/

	template <typename T>
	class CResourceContainer
	{
		public:
			typedef std::vector<T>       TElementsArray;
			typedef USIZE                TSizeType;
			typedef std::list<TSizeType> TFreeEntitiesRegistry;
		public:
			CResourceContainer():
				mElements {}, mFreeEntities {}
			{
				std::lock_guard<std::mutex> lock(mMutex);
			}

			CResourceContainer(const CResourceContainer& container):
				mElements(container.mElements), mFreeEntities(container.mFreeEntities)
			{
				std::lock_guard<std::mutex> lock(mMutex);
			}

			CResourceContainer(CResourceContainer&& container):
				mElements(container.mElements), mFreeEntities(container.mFreeEntities)
			{
				std::lock_guard<std::mutex> lock(mMutex);
			}

			~CResourceContainer()
			{
				std::lock_guard<std::mutex> lock(mMutex);

				mElements.clear();

				mFreeEntities.clear();
			}

			/*!
				\brief The method adds a given element into the container. The new element
				will be added to the end of the container if there is no free entities
				before last element. A position of insertion will be returned as method's result

				\param[in] element A value of type T

				\return An index at which the given element is placed
			*/

			TSizeType Add(const T& element)
			{
				std::lock_guard<std::mutex> lock(mMutex);

				TSizeType newEntryIndex = 0;

				if (mFreeEntities.empty())
				{
					newEntryIndex = mElements.size();

					mElements.push_back(element);

					return newEntryIndex;
				}

				newEntryIndex = mFreeEntities.front();

				mFreeEntities.pop_front();

				mElements[newEntryIndex] = element;

				return newEntryIndex;
			}

			/*!
				\brief The method excludes (not releases memory occupied by the element) element from the container
				
				\param[in] element An element that should be excluded from the container

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			
			E_RESULT_CODE Remove(const T& element)
			{
				std::lock_guard<std::mutex> lock(mMutex);

				return _removeAt(_findElement(element));
			}

			/*!
				\brief The method excludes (not releases memory occupied by the element) element from the container
				using its explicit index

				\param[in] index An index of an element that should be excluded from the container

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE RemoveAt(TSizeType index)
			{
				std::lock_guard<std::mutex> lock(mMutex);
				
				return _removeAt(index);
			}

			/*!
				\brief The method cleans up the container's data
			*/

			void RemoveAll()
			{
				std::lock_guard<std::mutex> lock(mMutex);

				TSizeType numOfElements = mElements.size();

				for (TSizeType i = 0; i < mElements.size(); ++i)
				{
					mFreeEntities.push_back(i);
				}
			}

			/*!
				\brief The method replaces element with given index or returns error

				\param[in] index An index of an element that should be replaced within the container
				\param[in] element A value of type T

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE ReplaceAt(TSizeType index, const T& element)
			{
				std::lock_guard<std::mutex> lock(mMutex);

				if (index >= mElements.size())
				{
					return RC_INVALID_ARGS;
				}

				mElements[index] = element;

				return RC_OK;
			}

			TSizeType FindIndex(const T& element, TSizeType startIndex = 0) const
			{
				std::lock_guard<std::mutex> lock(mMutex);

				auto it = std::find(mElements.cbegin() + startIndex, mElements.cend(), element);
				if (it == mElements.cend())
				{
					return mInvalidIndex;
				}

				return std::distance(mElements.cbegin(), it);
			}

			/*!
				\brief The operator returns an element which is placed at a given 
				position

				\param[in] index An index of an element

				\return The operator returns either an element which is placed at a given 
				position or an error code
			*/

			TResult<T> operator[] (TSizeType index) const
			{
				std::lock_guard<std::mutex> lock(mMutex);

				if (index >= mElements.size())
				{
					return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
				}

				if (std::find(mFreeEntities.cbegin(), mFreeEntities.cend(), index) != mFreeEntities.cend())
				{
					return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
				}

				return Wrench::TOkValue<T>(mElements[index]);
			}

			/*!
				\brief The method returns a number of elements within the container

				\return The method returns a number of elements within the container
			*/

			TSizeType GetSize() const
			{
				std::lock_guard<std::mutex> lock(mMutex);
				
				return mElements.size();
			}
		protected:
			E_RESULT_CODE _removeAt(TSizeType index)
			{
				if (index >= mElements.size())
				{
					return RC_FAIL;
				}

				//mElements[index] = (T)0;

				mFreeEntities.push_back(index);

				return RC_OK;
			}

			TSizeType _findElement(const T& element)
			{
				auto iter = std::find(mElements.cbegin(), mElements.cend(), element);

				if (iter == mElements.cend())
				{
					return mInvalidIndex;
				}

				return iter - mElements.cbegin();
			}

		public:
			static constexpr TSizeType mInvalidIndex = (std::numeric_limits<TSizeType>::max)();

		protected:
			TElementsArray         mElements;
			
			TFreeEntitiesRegistry  mFreeEntities;
			
			mutable std::mutex     mMutex;

	};
}