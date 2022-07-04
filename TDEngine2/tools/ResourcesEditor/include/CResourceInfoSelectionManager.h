/*!
	\file CResourceInfoSelectionManager.h
	\date 04.07.2022
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <string>
#include <vector>
#include <mutex>
#include <memory>


namespace TDEngine2
{
	/*!
		class CResourceInfoSelectionManager

		\brief The class is an implementation of a selection manager for resources items in resources browser
	*/

	class CResourceInfoSelectionManager
	{
		public:
			typedef std::unique_ptr<CResourceInfoSelectionManager> TUniquePtr;
			typedef std::string                                    TSelectableItem;
			typedef std::vector<TSelectableItem>                   TSelectableItems;
		public:
			void SetSelection(const TSelectableItem& path);
			void AddSelection(const TSelectableItem& path);

			void ResetSelection();

			bool IsSelected(const TSelectableItem& path) const;

			const TSelectableItems& GetSelectedEntities() const;
		private:
			TSelectableItems   mSelection;	

			mutable std::mutex mMutex;
	};
}