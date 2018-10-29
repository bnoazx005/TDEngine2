/*!
	\file TInternalUniformsBufferData.h
	\date 29.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"


namespace TDEngine2
{
	/*!
		struct TPerFrameInternalData

		\brief The structure contains shader uniform variables, which are updated each frame
	*/

	typedef struct TPerFrameInternalData
	{

	} TPerFrameInternalData;


	/*!
		struct TPerObjectInternalData

		\brief The structure contains shader uniform variables, which are object specific
	*/

	typedef struct TPerObjectInternalData
	{
	} TPerObjectInternalData;


	/*!
		struct TRareUpdatingInternalData

		\brief The structure contains shader uniform variables, which are update
		rarely
	*/

	typedef struct TRareUpdatingInternalData
	{

	} TRareUpdatingInternalData;


	/*!
		struct TInternalConstantData

		\brief The structure contains in-engine constants that are available in shaders
	*/

	typedef struct TInternalConstantData
	{
	} TInternalConstantData;


	/*!
		struct TInternalUniformsBufferData

		\brief The structure contains internal data structures that are used to
		fill in internal uniform buffers
	*/

	typedef struct TInternalUniformsBufferData
	{
		union
		{
			TPerFrameInternalData     mPerFrameData;

			TPerObjectInternalData    mPerObjectData;

			TRareUpdatingInternalData mRareUpdatingData;

			TInternalConstantData     mContantData;
		} mData;
		
		E_INTERNAL_UNIFORM_BUFFER_REGISTERS mUsedSlot;
	} TInternalUniformsBufferData, *TInternalUniformsBufferDataPtr;
}