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
#include "CResult.h"


namespace TDEngine2
{
	/*!
		class CResourceContainer

		\brief The class implements a common container for resources of different types
		including files, textures, entities, components, etc...
		An instance of the class is thread-safe.
	*/

	template <typename T>
	class CResourceContainer
	{
		public:
			typedef std::vector<T> TElementsArray;
			typedef std::list<U32> TFreeEntitiesRegistry;
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

			U32 Add(const T& element)
			{
				std::lock_guard<std::mutex> lock(mMutex);

				U32 newEntryIndex = 0;

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

			E_RESULT_CODE RemoveAt(U32 index)
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

				U32 numOfElements = mElements.size();

				for (U32 i = 0; i < mElements.size(); ++i)
				{
					mElements[i] = (T)0;

					mFreeEntities.push_back(i);
				}
			}

			/*!
				\brief The operator returns an element which is placed at a given 
				position

				\param[in] index An index of an element

				\return The operator returns either an element which is placed at a given 
				position or an error code
			*/

			TResult<T> operator[] (U32 index) const
			{
				std::lock_guard<std::mutex> lock(mMutex);

				if (index >= mElements.size())
				{
					return TErrorValue<E_RESULT_CODE>(RC_INVALID_ARGS);
				}

				return TOkValue<T>(mElements[index]);
			}

			/*!
				\brief The method returns a number of elements within the container

				\return The method returns a number of elements within the container
			*/

			U32 GetSize() const
			{
				std::lock_guard<std::mutex> lock(mMutex);
				
				return mElements.size();
			}
		protected:
			E_RESULT_CODE _removeAt(U32 index)
			{
				if (index >= mElements.size())
				{
					return RC_FAIL;
				}

				mElements[index] = (T)0;

				mFreeEntities.push_back(index);

				return RC_OK;
			}

			U32 _findElement(const T& element)
			{
				auto iter = std::find(mElements.cbegin(), mElements.cend(), element);

				if (iter == mElements.cend())
				{
					return mInvalidIndex;
				}

				return iter - mElements.cbegin();
			}
		protected:
			TElementsArray        mElements;
			
			TFreeEntitiesRegistry mFreeEntities;
			
			mutable std::mutex    mMutex;

			static const U32      mInvalidIndex = (std::numeric_limits<U32>::max)();
	};
}