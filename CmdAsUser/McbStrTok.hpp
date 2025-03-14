/*
 ****************************************************************************
 * McbStrTok.hpp : Template class to duplicate ::Strtok() in a thread safe
 * manner (::strtok() uses static buffers).  Implemented as a template class
 * to eliminate need to include the cpp file.  Don't use the template class
 * itself, use the typedef at the bottom of the file.
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	Eve of birthday 	2001	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */

/*
 ****************************************************************************
 * Include only once
 ****************************************************************************
 */
#ifndef McbStrTok_Included
#define McbStrTok_Included

/*
 ****************************************************************************
 * Include all necessary include files
 ****************************************************************************
 */
#include <tchar.h>

#pragma warning(disable : 4786)
#include <string>

/**
 ****************************************************************************
 * <P> Template class to duplicate functionality of strtok but as thread-safe 
 * (strtok) uses static data.</P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	Eve of birthday 	2001	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */
template <int n=0> class McbStrTokImpl
{
public:

/*
 ****************************************************************************
 * Defines for generic text mappings with std::string
 ****************************************************************************
 */
#if defined(_UNICODE) || defined(UNICODE)
	typedef std::basic_string<WCHAR> tstring;	
#else //defined(_UNICODE) || defined(UNICODE)
	typedef std::string tstring;
#endif //defined(_UNICODE) || defined(UNICODE)

   /*
    *************************************************************************
    * Constructor
    *************************************************************************
    */
	McbStrTokImpl(LPCTSTR lpszParams, LPCTSTR lpszData) : m_lpszNext(NULL)
    {
        if (lpszData)
        {
           /*
            *****************************************************************
            * take copy of the buffer and append additional NULL
            *****************************************************************
            */      
            m_strBuffer.append(lpszData);
            m_strBuffer.append(_T("\0"), 1);

           /*
            *****************************************************************
            * replace any occurrence of the parameters with a NULL
            *****************************************************************
            */
            for(LPTSTR lpszMem=(LPTSTR)m_strBuffer.c_str(); *lpszMem; 
				lpszMem++)
            {
                LPCTSTR lpszChar = _tcschr(lpszParams, *lpszMem);

                if (lpszChar)
                {
                    *lpszMem = NULL;
                }
            }

            m_lpszNext = m_strBuffer.c_str();       
        }

    }

   /*
    *************************************************************************
    * Find the next token in a string.  Returns NULL after last token is 
    * found.
    *************************************************************************
    */
    LPCTSTR McbGetNext() const
    {
		if (!m_lpszNext || 
            m_lpszNext == m_strBuffer.c_str() + m_strBuffer.size())
        {
            return NULL;
        }

        LPCTSTR lpszResult = m_lpszNext;

       /*
        *********************************************************************
        * NULL should always be found so send MAXDWORD as length 
        * parameter
        *********************************************************************
        */
        if (!(*m_lpszNext))
        {
            m_lpszNext++;
        }
        else
        {
            m_lpszNext = (LPCTSTR)memchr(m_lpszNext+1, NULL, MAXDWORD) + 
                sizeof(TCHAR);
        }
    
        return lpszResult;
    }

protected:
   /*
    *************************************************************************
    * Members
    *************************************************************************
    */
	tstring m_strBuffer;
    mutable LPCTSTR m_lpszNext;	
};

/*
 ****************************************************************************
 * Typedef for ease of use
 ****************************************************************************
 */
typedef McbStrTokImpl<> McbStrTok;

#endif //McbStrTok_Included
