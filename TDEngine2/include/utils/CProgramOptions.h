/*!
	\file CProgramOptions.h
	\date 03.10.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "Utils.h"
#include "Types.h"
#include "../core/CBaseObject.h"
#include "variant.hpp"
#include <string>
#include <vector>


namespace TDEngine2
{
	/*!
		class CProgramOptions

		\brief The class implements an argument parser. This is one of the types that's a singleton for the sake
		of simplicity. Should be initialized 
	*/

	class CProgramOptions : public CBaseObject
	{
		public:
			struct TParseArgsParams
			{
				I32         mArgsCount;
				const C8**  mpArgsValues;
				std::string mProgramDescriptionStr;
				std::string mpProgramUsageStr;
			};


			struct TArgumentParams
			{
				typedef Wrench::Variant<I32, F32, std::string, bool> TValueType;

				C8          mSingleCharCommand = '\0';
				std::string mCommand;
				std::string mCommandInfo;

				TValueType  mValue;
			};
		public:
			/*!
				\brief The method should be called after all arguments are added

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ParseArgs(const TParseArgsParams& params);

			/*!
				\brief The method resets internal state of the options parser and removes all added arguments
			*/

			TDE2_API void Reset();

			TDE2_API E_RESULT_CODE AddArgument(const TArgumentParams& params);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime

				\return A pointer to an instance of CProgramOptions type
			*/

			TDE2_API static TPtr<CProgramOptions> Get();
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProgramOptions)
		private:
			std::vector<TArgumentParams> mArgumentsInfo;

			bool mIsInternalStateInitialized;
	};
}