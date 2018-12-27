/*!
	\file IGraphicsLayersInfo.h
	\date 12.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IBaseObject.h"


namespace TDEngine2
{
	/*!
		interface IGraphicsLayersInfo

		\brief The interface describes a functionality of a list
		 which contains an information about graphics layers
	*/

	class IGraphicsLayersInfo: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			/*!
				\brief The method adds a new layer into the list

				\param[in] depthValue A value of depth from which a new layer begins

				\param[in] layerName A name of a layer (optional)

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE AddLayer(F32 depthValue, const C8* layerName = nullptr) = 0;

			/*!
				\brief The method returns an information a given depthValue belongs to

				\param[in] depthValue A floating-point value which represents a depth of a scene

				\return The method returns an information a given depthValue belongs to
			*/

			TDE2_API virtual U16 GetLayerIndex(F32 depthValue) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGraphicsLayersInfo)
	};
}