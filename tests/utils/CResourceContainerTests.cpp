#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <vector>

using namespace TDEngine2;


TEST_CASE("CResourceContainer Tests")
{
	SECTION("TestAdd_AddValuesWithoutDeleting_SavesOrder")
	{
		CResourceContainer<int> container;

		std::vector<int> expectedContainer { 0, 1, 2, 3, 4, 5 };

		for (int value : expectedContainer)
		{
			container.Add(value);
		}

		REQUIRE(container.GetSize() == expectedContainer.size());

		for (U32 i = 0; i < container.GetSize(); ++i)
		{
			REQUIRE(expectedContainer[i] == container[i].Get());
		}
	}

	SECTION("TestAdd_AddValuesAndRemoveSome_PackValuesToSaveSpace")
	{
		CResourceContainer<int> container;

		std::vector<int> initialValues { 0, 1, 2, 3, 4, 5 };
		std::vector<int> additionalValues { 8, 9, 10 }; /// new values will be added at 2, 3 and 6 positions

		std::vector<int> expectedContainer { 0, 1, 8, 9, 4, 5, 10 };

		for (int value : initialValues)
		{
			container.Add(value);
		}

		/// remove 2nd and 3rd elements from the container
		REQUIRE(container.Remove(2) == RC_OK);		/// check deletion using search alogrithm
		REQUIRE(container.RemoveAt(3) == RC_OK);	/// check deletion using index

		for (int value : additionalValues)
		{
			container.Add(value);
		}

		REQUIRE(container.GetSize() == expectedContainer.size());

		for (U32 i = 0; i < container.GetSize(); ++i)
		{
			REQUIRE(expectedContainer[i] == container[i].Get());
		}
	}

	SECTION("TestRemoveAll_RemoveAllElements_ContainerIsEmpty")
	{
		CResourceContainer<int> container;

		std::vector<int> expectedContainer{ 0, 1, 2, 3, 4, 5 };

		for (int value : expectedContainer)
		{
			container.Add(value);
		}

		REQUIRE(container.GetSize() == expectedContainer.size());

		container.RemoveAll();

		REQUIRE(container.GetSize() == expectedContainer.size()); /// the size of the container remains the same
	}
}