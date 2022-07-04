#include "../include/CResourceInfoSelectionManager.h"


namespace TDEngine2
{
	void CResourceInfoSelectionManager::SetSelection(const TSelectableItem& path)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mSelection.clear();
		mSelection.push_back(path);
	}

	void CResourceInfoSelectionManager::AddSelection(const TSelectableItem& path)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mSelection.push_back(path);
	}

	void CResourceInfoSelectionManager::ResetSelection()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mSelection.clear();
	}

	bool CResourceInfoSelectionManager::IsSelected(const TSelectableItem& path) const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return std::find(mSelection.cbegin(), mSelection.cend(), path) != mSelection.cend();
	}

	const CResourceInfoSelectionManager::TSelectableItems& CResourceInfoSelectionManager::GetSelectedEntities() const
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mSelection;
	}
}
