/*!
	\file CGraphicsLayersInfo.h
	\date 12.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IGraphicsLayersInfo.h"
#include "./../core/CBaseObject.h"
#include <vector>
#include <tuple>
#include <string>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CGraphicsLayersInfo's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CGraphicsLayersInfo's implementation
	*/

	TDE2_API IGraphicsLayersInfo* CreateGraphicsLayersInfo(E_RESULT_CODE& result);


	/*!
		class CGraphicsLayersInfo

		\brief The class implements IGraphicsLayersInfo
	*/

	class CGraphicsLayersInfo : public CBaseObject, public IGraphicsLayersInfo
	{
		public:
			friend TDE2_API IGraphicsLayersInfo* CreateGraphicsLayersInfo(E_RESULT_CODE& result);
		protected:
			typedef std::vector<std::tuple<F32, std::string>> TGraphicsLayersArray;
		public:
			/*!
				\brief The method initializes an initial state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method adds a new layer into the list

				\param[in] depthValue A value of depth from which a new layer begins

				\param[in] layerName A name of a layer (optional)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddLayer(F32 depthValue, const C8* layerName = nullptr) override;

			/*!
				\brief The method returns an information a given depthValue belongs to

				\param[in] depthValue A floating-point value which represents a depth of a scene

				\return The method returns an information a given depthValue belongs to
			*/

			TDE2_API U16 GetLayerIndex(F32 depthValue) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGraphicsLayersInfo)
		protected:
			TGraphicsLayersArray mGraphicsLayers;
	};
}