/*!
	\file IBuffer.h
	\date 03.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IBaseObject.h"
#include "../utils/Types.h"
#include "../utils/Utils.h"


namespace TDEngine2
{
	class IGraphicsContext;


	/*!
		enumeration E_BUFFER_USAGE_TYPE
	*/

	enum E_BUFFER_USAGE_TYPE: U8
	{
		BUT_DYNAMIC,				///< A CPU can write data into a buffer and read it
		BUT_STATIC,					///< A buffer's content cannot be changed after initialization
		BUT_DEFAULT					///< A GPU will have access to read and write
	};


	/*!
		\brief An enumeration of all possible ways of locking a buffer
	*/

	enum E_BUFFER_MAP_TYPE
	{
		BMT_WRITE_DISCARD,				///< Access to write data into the buffer, but all previous data will be discarded
		BMT_NOOVERWRITE,				///< 
		BMT_WRITE,						///<
		BMT_READ_WRITE,					///< Access to read / write operations, a buffer should be created with a corresponding usage type to provide this features
		BMT_READ,						///< 
		BMT_DEFAULT = BMT_WRITE_DISCARD	///< Default type of an access is BMT_WRITE_DISCARD
	};


	/*!
		\brief The enumeration contains all possible types, how COGLBuffer can be used
	*/

	enum class E_BUFFER_TYPE : U32
	{
		BT_VERTEX_BUFFER,			///< A buffer will be used as vertex buffer
		BT_INDEX_BUFFER,			///< A buffer will be used as index buffer
		BT_CONSTANT_BUFFER,			///< A buffer will be used as uniforms buffer object
		BT_STRUCTURED_BUFFER,
	};


	enum class E_STRUCTURED_BUFFER_TYPE : U32;


	struct TInitBufferParams
	{
		IGraphicsContext*        mpGraphicsContext;
		E_BUFFER_USAGE_TYPE      mUsageType;
		E_BUFFER_TYPE            mBufferType;
		USIZE                    mTotalBufferSize;
		const void*              mpDataPtr = nullptr;
		USIZE                    mDataSize = 0;
		/// Structured buffers part
		bool                     mIsUnorderedAccessResource = false;
		USIZE                    mElementStrideSize = 0;
		E_STRUCTURED_BUFFER_TYPE mStructuredBufferType;
	};

	
	/*!
		interface IBuffer

		\brief The interface describes a functionality of a hardware buffer.
		This term includes vertex/index/constant(D3D) and oher types.
	*/

	class IBuffer: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method locks a buffer to provide safe data reading/writing

				\param[in] mapType A way the data should be processed during a lock

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Map(E_BUFFER_MAP_TYPE mapType) = 0;

			/*!
				\brief The method unlocks a buffer, so GPU can access to it after that operation
			*/

			TDE2_API virtual void Unmap() = 0;

			/*!
				\brief The method writes data into a buffer

				\param[in] pData A pointer to data, which should be written
				\param[in] count A size of data in bytes

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Write(const void* pData, USIZE size) = 0;

			/*!
				\brief The method returns a pointer to buffer's data
				
				\return The method returns a pointer to buffer's data
			*/

			TDE2_API virtual void* Read() = 0;

			/*!
				\brief The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers

				\return The method returns an internal data of a buffer, which
				contains low-level platform specific buffer's handlers
			*/

			TDE2_API virtual const TBufferInternalData& GetInternalData() const = 0;
			
			/*!
				\brief The method returns buffer's size in bytes

				\return The method returns buffer's size in bytes
			*/

			TDE2_API virtual USIZE GetSize() const = 0;

			/*!
				\return The method returns an amount of occupied buffer's bytes
			*/

			TDE2_API virtual USIZE GetUsedSize() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IBuffer)
	};
}