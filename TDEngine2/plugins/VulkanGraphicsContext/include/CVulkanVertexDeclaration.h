/*!
	\file CVulkanVertexDeclaration.h
	\date 05.11.2023
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CVertexDeclaration.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CVulkanVertexDeclaration's type

		\return A pointer to CVulkanVertexDeclaration's implementation
	*/

	IVertexDeclaration* CreateVulkanVertexDeclaration(E_RESULT_CODE& result);
}