/**
 ****************************************************************************
 * <P> McbFormatError.hpp - simple class to format an error number (as 
 * returned from ::GetLastError() into a string.  Implemented as a template
 * class so no cpp file has to be included - don't use the class directly,
 * use the typedef at the bottom of the file.
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown
 *
 * @changeHistory  
 *	02nd August      	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */

/*
 ****************************************************************************
 * Include me once please
 ****************************************************************************
 */
#ifndef McbFormatError_Included
#define McbFormatError_Included

/*
 ****************************************************************************
 * Include all necessary include files
 ****************************************************************************
 */
#include <windows.h>
#include <tchar.h>

/**
 ****************************************************************************
 * <P> Simple template class to format an error as a string </P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	2nd August    	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */
template <int n = 0> class McbFormatErrorImpl
{
public:
   /*
    *************************************************************************
    * Construction/Destruction
    *************************************************************************
    */
	McbFormatErrorImpl() : m_lpMsg(NULL) {}
	~McbFormatErrorImpl() { McbFree(); }

   /*
    *************************************************************************
    * Read the description for the error.  If this is 0 then 
	* ::GetLastError() will be used to obtain it.
    *************************************************************************
    */
	LPCTSTR McbGetDescription(DWORD dwErr = 0)
	{
		McbFree();
		if (!dwErr) dwErr = ::GetLastError();		

       /*
        *********************************************************************
        * Format the message - the system will allocate the memory for us.
        *********************************************************************
        */
		if (!::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,    
			NULL,
			dwErr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &m_lpMsg, 
			0, 
			NULL))
		{
			McbFree();
		}

		return (LPCTSTR)m_lpMsg;		
	}
	

protected:
   /*
    *************************************************************************
    * Cleanup
    *************************************************************************
    */
	void McbFree()
	{
		if (m_lpMsg)
		{
			::LocalFree(m_lpMsg);
			m_lpMsg = NULL;
		}
	}

   /*
    *************************************************************************
    * Members
    *************************************************************************
    */
	LPVOID	m_lpMsg;

};

/**
 ****************************************************************************
 * <P> Simple template class to format an error as a string.  This version
 * terminates the string at the first non-printable character.</P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	2nd August    	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */
template <int n = 0> class McbFormatError2Impl
{
public:
   /*
    *************************************************************************
    * Construction/Destruction
    *************************************************************************
    */
	McbFormatError2Impl() : m_lpMsg(NULL) {}
	~McbFormatError2Impl() { McbFree(); }

   /*
    *************************************************************************
    * Read the description for the error.  If this is 0 then 
	* ::GetLastError() will be used to obtain it.
    *************************************************************************
    */
	LPCTSTR McbGetDescription(DWORD dwErr = 0)
	{
		if (!dwErr) dwErr = ::GetLastError();		

		McbFree();
		
       /*
        *********************************************************************
        * Format the message - the system will allocate the memory for us.
        *********************************************************************
        */
		if (!::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,    
			NULL,
			dwErr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &m_lpMsg, 
			0, 
			NULL))
		{
			McbFree();
		}
		else
		{
           /*
            *****************************************************************
            * Terminate the string at the first non-printable character
            *****************************************************************
            */
			LPTSTR lpMarker = (LPTSTR)m_lpMsg;
			for (; *lpMarker != NULL; lpMarker++)
			{
				if (!::_istprint(*lpMarker))
				{
					*lpMarker = NULL;
					break;
				}			
			}
		}

		return (LPCTSTR)m_lpMsg;		
	}
	
protected:
   /*
    *************************************************************************
    * Cleanup
    *************************************************************************
    */
	void McbFree()
	{
		if (m_lpMsg)
		{
			::LocalFree(m_lpMsg);
			m_lpMsg = NULL;
		}
	}

   /*
    *************************************************************************
    * Members
    *************************************************************************
    */
	LPVOID	m_lpMsg;

};

/*
 ****************************************************************************
 * Typedef for simple invocation
 ****************************************************************************
 */
typedef McbFormatErrorImpl<>		McbFormatError;
typedef McbFormatError2Impl<>	McbFormatError2;

#endif //McbFormatError_Included
