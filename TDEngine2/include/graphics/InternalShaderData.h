/*!
	/file InternalShaderData.h
	/date 27.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../math/TMatrix4.h"


namespace TDEngine2
{
#pragma pack(push, 16)

	/*!
		struct TPerFrameShaderData

		\brief The structure contains shader uniform data that are updated each frame
	*/

	typedef struct TPerFrameShaderData
	{
		TMatrix4 mProjMatrix;

		TMatrix4 mViewMatrix;
	} TPerFrameShaderData, *TPerFrameShaderDataPtr;


	/*!
		struct TPerObjectShaderData

		\brief The structure contains shader uniform data that are changed per object
	*/

	typedef struct TPerObjectShaderData
	{
		TMatrix4 mModelMatrix;
		
		U32      mObjectID;	///< The field is used by selection manager

		U32      mUnused[3];
	} TPerObjectShaderData, *TPerObjectShaderDataPtr;


	/*!
		struct TRareUpdateShaderData

		\brief The structure contains shader uniform data that are rare updated
	*/

	typedef struct TRareUpdateShaderData
	{
		TMatrix4 mUnused;
	} TRareUpdateShaderData, *TRareUpdateShaderDataPtr;


	/*!
		struct TConstantShaderData

		\brief The structure contains shader uniform data that stays constant during the application's execution
	*/

	typedef struct TConstantShaderData
	{
		TMatrix4 mUnused;
	} TConstantShaderData, *TConstantShaderDataPtr;

#pragma pack(pop)
}