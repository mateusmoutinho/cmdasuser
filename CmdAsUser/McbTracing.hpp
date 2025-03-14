/**
 ****************************************************************************
 * <P> Simple template class to log to stdout or the debug stream (which can 
 * then be monitored with dbmon).</P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown
 *
 * @changeHistory  
 *	26th October 	2000	 - 	(V1.0) Creation (MCB)
 *	22nd October	2001	 -  Modified to log to the event log if 
 *								McbLOGTOEVENTLOG is defined.
 ****************************************************************************
 */

/*
 ****************************************************************************
 * Include only once
 ****************************************************************************
 */
#ifndef McbTracing_Included
#define McbTracing_Included

/*
 ****************************************************************************
 * This define can be used to filter which events are logged.  The value of
 * this constant is bitwise AND'ed with the level passed to the trace macro
 * and will only invoke tracing if the result is non zero.
 *
 * If logging level not defined then assume we are logging everything
 ****************************************************************************
 */

#ifndef McbLOGGINGLEVEL 
	#define McbLOGGINGLEVEL 0xFFFFFFFF
#endif

/*
 ****************************************************************************
 * Defines for selective tracing for common header files
 ****************************************************************************
 */
#define McbTRACERESOURCEMAP				0x80000000// for McbResourceMap2.hpp
#define McbTRACEPOOLOFTHREADSNORMAL		0x40000000
												// for McbPoolOfThreads3.hpp	
#define McbTRACEPOOLOFTHREADSEXCEPTION	0x20000000	
#define McbTRACEPOOLOFTHREADSLOCKS		0x10000000
#define McbTRACEPOOLOFTHREADSTHREADING	0x08000000
#define McbTRACESERVICE					0x04000000// for McbService.hpp
#define McbTRACELESSSTRING				0x02000000// for 
												  // McbSTLStringCompare.hpp
#define	McbTRACEFUNCTIONTIMINGS			0x01000000// for McbTimingTracker.hpp
#define McbTRACESMARTCLEANUPDEADLOCKS	0x00800000// for McbSmartCleanup.hpp
#define McbTRACETHREADGROUP				0x00400000// for McbThreadGroup.hpp
#define McbTRACENOTIFYMAP2				0x00200000// for McbNotifyMap2.hpp
#define	McbTRACESOCKETS					0x00100000// for 
												// McbAsynchronousSockets.hpp
#define McbTRACEREGMAP					0x00080000// for McbRegistryMap.hpp
#define McbTRACETHREADID					0x00040000

/*
 ****************************************************************************
 * Include all necessary include files
 ****************************************************************************
 */
#include <stdio.h>
#include <tchar.h>

/*
 ****************************************************************************
 * If we are not logging to console then we need windows.h for 
 * ::OutputDebugString().
 ****************************************************************************
 */
#ifndef McbLOGTOCONSOLE
	
#endif //McbLOGTOCONSOLE

/*
 ****************************************************************************
 * If logging to console 
 ****************************************************************************
 */
#ifdef McbLOGTOCONSOLE
            
/*
 ****************************************************************************
 * If logging to the event log
 ****************************************************************************
 */
#elif defined(McbLOGTOEVENTLOG) //McbLOGTOCONSOLE

	extern "C" 
	{
		#include "McbEventLogDll.h"
	}

/*
 ****************************************************************************
 * If logging to the debug stream
 ****************************************************************************
 */
#else
   /*
    *************************************************************************
    * We need windows.h for ::OutputDebugString().
    *************************************************************************
    */
	#include <windows.h>

#endif //McbLOGTOCONSOLE

/**
 ****************************************************************************
 * <P> Template class with one static function for tracing.  In a template
 * means ya dont need a cpp file. </P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	26th October   	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */
template <int n> class McbTraceImpl
{
public:
   /*
    *************************************************************************
    * Output to debug stream or stdout.
    *************************************************************************
    */
	static void McbOutput(unsigned long nLevel, const TCHAR * lpszFirst, ...)
    {

       /*
        *********************************************************************
        * Selective logging based on the value if McbLOGGINGLEVEL
        *********************************************************************
        */
        if (nLevel == 0 || nLevel & McbLOGGINGLEVEL)
        {

/*
 ****************************************************************************
 * If logging to console 
 ****************************************************************************
 */
#ifdef McbLOGTOCONSOLE
            
            va_list marker;
            va_start(marker, lpszFirst);

            _vtprintf(lpszFirst, marker);

            va_end(marker);

/*
 ****************************************************************************
 * If logging to the event log
 ****************************************************************************
 */
#elif defined(McbLOGTOEVENTLOG) //McbLOGTOCONSOLE

			static unsigned long lEventLog = ::McbOpenLog(McbGetEventSource());

			if (lEventLog)
			{			
				TCHAR szBuffer[1024*10];

				va_list marker;
				va_start(marker, lpszFirst);

				_vstprintf(szBuffer, lpszFirst, marker);

				va_end(marker);

				::McbLog(lEventLog, szBuffer, EVENTLOG_INFORMATION_TYPE, NULL,
					0, McbCATEGORYDEBUGGING);
			}

/*
 ****************************************************************************
 * If logging to the debug stream
 ****************************************************************************
 */
#else

            TCHAR szBuffer[1024*10];

            va_list marker;
            va_start(marker, lpszFirst);

            _vstprintf(szBuffer, lpszFirst, marker);

            va_end(marker);

            ::OutputDebugString(szBuffer);

#endif //McbLOGTOCONSOLE
        }
    }

protected:
/*
 ****************************************************************************
 * If logging to the event log
 ****************************************************************************
 */
#ifdef McbLOGTOEVENTLOG

   /*
    *************************************************************************
    * Simple class to obtain the event source for the event log.
    *************************************************************************
    */
	class McbEventSource
	{
	public:
		McbEventSource() : m_bInit(false) {}

		LPCTSTR McbGetName(HMODULE hModule=NULL)
		{
			if (!m_bInit)
			{
				TCHAR szPath[MAX_PATH+1];

				if (::GetModuleFileName(hModule, szPath, sizeof(szPath)-1) 
					== 0)
				{
					m_szFile[0] = _T('\0');
				}
				else
				{
					::_splitpath(szPath, NULL, NULL, m_szFile, NULL);
				}

				m_bInit = true;
			}

			return m_szFile;
		}

	protected:
		TCHAR	m_szFile[400];
		bool	m_bInit;
		
	};

   /*
    *************************************************************************
    * Static function to obtain the event source
    *************************************************************************
    */
	static LPCTSTR McbGetEventSource()
	{
		static McbEventSource source;

		return source.McbGetName();
	}

#endif //McbLOGTOEVENTLOG

};

/*
 ****************************************************************************
 * Trace Macro
 * This should be invoked as the following (note the double brackets):
 * McbTRACE((1, "ThreadId: %d, %s\n", dwThreadId, "Dispatcher Thread"))
 ****************************************************************************
 */
#ifdef _DEBUG
#define McbTRACE(stuff) ::McbTraceImpl<0>::McbOutput stuff;
#else //_DEBUG
#define McbTRACE(stuff)
#endif //_DEBUG

#endif //McbTracing_Included
