/*!
\file COGLShader.h
\date 22.10.2018
\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseShader.h>


namespace TDEngine2
{
	/*!
		class COGLShader

		\brief The class is a common implementation for all platforms
	*/

	class COGLShader : public IShader, public CBaseShader
	{
		public:
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(COGLShader)
		protected:
	};
}
