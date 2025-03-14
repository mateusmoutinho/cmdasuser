/*
 ****************************************************************************
 * McbService.hpp : Be noticed at parties, amaze your family and friends - 
 * implement an NT Service.
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	3rd October   	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 *
 * This set of classes and template classes eases the implementation of an NT
 * Service.  An NT Service executable consists of a series of services it 
 * supports (one or more).
 *
 * To implement a service you need to inherit from the McbServiceEntry class.
 * Then override the McbRun() to implement the required functionality for the 
 * service.  The McbRun() should not perform lengthy operations and should 
 * return as quick as possible so that the service seems responsive (ie stops
 * and starts quickly).  If you require lengthy operations in here then you
 * should delegate to another thread.
 *
 * Once McbServiceEntry has been derived from (for as many services the exe 
 * will support) the McbBEGINSERVICEMAP(), McbSERVICEENTRY() and 
 * McbENDSERVICEMAP() macros need to be implemented to register the 
 * service(s).
 *
 * The McbSERVICEENTRYMAP() macro takes four parameters including:
 *
 *  the name of the class derived from McbServiceEntry,
 *
 *	the name of service (no spaces),
 *
 *  a number starting at 0 incrementing by one for each service in the exe.
 *		THIS IS IMPORTANT because using an index like this is the only way I 
 *		could get round the fact that you cant pass non-implemented strings 
 *		as template parameters (that is, strings without stack space),
 *
 *  a value specifying what the service is cabable of doing.  
 *		This is a bit mask of ORed values dicating whether the service is 
 *		capable of stopping (SERVICE_ACCEPT_STOP) or pausing and restarting
 *		(SERVICE_ACCEPT_PAUSE_CONTINUE).
 *
 * Then to start the services you need to call the macro McbStartServices()
 * and this must be called within a short period (about 200ms I think) of
 * the executable starting.  If this is not called quickly then the Service
 * Control Manager (SCM) thinks the service has failed.  Note also that this
 * macro calls ::StartServiceCtrlDispatcher() which will not return until all
 * services this exe is servicing have finished.  This macro should be called
 * promptly from your main/winmain.
 *
 * Other optional overridables from McbServiceEntry include:
 *	McbStart()		-	called when the service starts for initialisation,
 *	McbStop()		-	called when the service is ending for cleanup,
 *	McbPause() 		-	called when the service is about to pause,
 *	McbContinue()	-	called when a paused service is about to continue.
 * 
 * Return NO_ERROR from each of these functions if the operation is 
 * successful.  If you return a win32 error code from these functions it will
 * be reported back to the user (either via a message box or the event 
 * viewer.)  You can also specify a specific error code by populating the 
 * m_dwSpecificError member variable.
 * If processing any of these functions will take a lengthy amount of time 
 * you also need to override the appropriate following function(s) and return 
 * the approximate amount of milliseconds the service will take to change its 
 * state.  This is used as a wait hint to the SCM:
 *	McbGetStartWaitHint()
 *	McbGetStopWaitHint()
 *	McbGetPauseWaitHint()
 *	McbGetContinueWaitHint()
 *	
 * Use 'SC Create' from the NT Resource kit and 'SC delete' to 
 * install/deinstall your service(s) once you have created the exe.  This 
 * needs the service name (as specified in parameter 2 of the 
 * McbSERVICEENTRYMAP() macro) as a parameter.  'SC.exe' also allows you to 
 * specify a user friendly display name for each service.
 *
 * If you want to debug the service as an executable then define McbRUNASEXE.
 * This will remove all calls to Service specific stuff and will call 
 * McbServiceMain() for the first service in the service map when you call the
 * McbStartServices() macro.
 * I found this useful cos boundschecker doesnt seem to let you attach to a 
 * running process (only kick off a new one).  Services are traditionally 
 * running by the time you can get your grubby little mits on them so using
 * boundschecker to debug a RUNNING service is a no no.
 *
 * For more implementation details see the example of a trivial NT Service.
 *
 ****************************************************************************
 */

#ifdef ThisIsOnlyAnExample

#include "McbService.hpp"

/*
 ****************************************************************************
 * Implement the first service - derived from McbServiceEntry and override
 * McbRun().
 ****************************************************************************
 */
class McbService1 : public McbServiceEntry
{
public:
	virtual void McbRun() 
	{
		for(int n = 0; n < 10; n++)
		{			
			McbTRACE((McbTRACESERVICE, _T("This is call %d of Service1"), n))
			::Sleep(10);
		}
	}
};

/*
 ****************************************************************************
 * Implement the second service
 ****************************************************************************
 */
class McbService2 : public McbServiceEntry
{
public:
	virtual void McbRun()
	{
		for(int n = 0; n < 10; n++)
		{			
			McbTRACE((McbTRACESERVICE, _T("This is call %d of Service2"), n))
			::Sleep(10);			
		}
	}
};

/*
 ****************************************************************************
 * Implement the service map - this lists all the services controlled by the
 * exe. 
 *
 * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPO
 * Ensure the Third parameter is an incrementing counter starting at zero.
 * IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPO
 *
 * This example implements a service called Service1 which can be stopped,
 * paused/continued and a service called Service2 which can be stopped
 * but not paused/continued.
 ****************************************************************************
 */
McbBEGINSERVICEMAP()
	McbSERVICEENTRY(McbService1, _T("Service1"), 0, 
		SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE)

	McbSERVICEENTRY(McbService2, _T("Service2"), 1, SERVICE_ACCEPT_STOP)	
McbENDSERVICEMAP()

/*
 ****************************************************************************
 * Entry point
 ****************************************************************************
 */
void main() 
{	
   /*
    *************************************************************************
    * Start the services.  This API, if successful will not return until
	* both Service1 and Service2 have stopped.
    *************************************************************************
    */
	if (!McbStartServices(0, 0)) 
    { 
		McbTRACE((McbTRACESERVICE, 
			_T("[%d] Main thread failed to register services.  Last error:")
			_T("%d!!!"), ::GetCurrentThreadId(), ::GetLastError()))
    } 

}/* main */


#endif //ThisIsOnlyAnExample

/*
 ****************************************************************************
 * Include only once
 ****************************************************************************
 */
#ifndef McbService_Included
#define McbService_Included

/*
 ****************************************************************************
 * Include all necessary include files
 ****************************************************************************
 */
//#include <tchar.h>
#include <windows.h>
#include "McbTracing.hpp"

#ifdef _DEBUG
	#include <string>
#endif //_DEBUG

/*
 ****************************************************************************
 * Custom windows event so the thread running McbServiceMain can respond to 
 * the thread running McbServiceHandler.
 ****************************************************************************
 */
#define WM_SERVICE_START		WM_USER+1
#define WM_SERVICE_PAUSE		WM_USER+2
#define WM_SERVICE_CONTINUE		WM_USER+3
#define WM_SERVICE_INTEROGATE	WM_USER+4
#define WM_SERVICE_STOP			WM_USER+5

/*
 ****************************************************************************
 * Prototypes
 ****************************************************************************
 */
typedef VOID (WINAPI * PFNSERVICEHANDLER)(DWORD);
const SERVICE_TABLE_ENTRY * McbGetDispatchTable();

/**
 ****************************************************************************
 * <P> Abstract base class to be dervice from to implement an NT Service.  
 * This is then used with the McbBEGINSERVICEMAP() macros to work around the 
 * static nature of the ServiceMain and Handler functions required for an NT 
 * Service to work. </P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	3rd October   	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */
class McbServiceEntry
{
   /*
    *************************************************************************
    * Declare our everlasting friendship to the service controller base class
    *************************************************************************
    */	
	friend class McbServiceBase;

/*
 ****************************************************************************
 * Debug and non-debug versions to update the service status (called when it 
 * has changed from paused to running for example)
 ****************************************************************************
 */
#ifdef _DEBUG
   /*
    *************************************************************************
    * Debug version to update service status with extra logging
    *************************************************************************
    */
	BOOL McbSetServiceStatus()
	{
       /*
        *********************************************************************
        * If requested  to run as a server then ignore service specific APIs
        *********************************************************************
        */
		#ifdef McbRUNASEXE
			BOOL bUpdateStatus = TRUE;

		#else //McbRUNASEXE
			BOOL bUpdateStatus = ::SetServiceStatus(m_hServiceStatus, 
				&m_ServiceStatus);

           /*
            *****************************************************************
            * Parse the service type into something readable
            *****************************************************************
            */
            LPCTSTR lpszServiceType;

            switch(m_ServiceStatus.dwServiceType)
            {
                case SERVICE_WIN32_OWN_PROCESS: 
                    lpszServiceType = _T("SERVICE_WIN32_OWN_PROCESS");
                    break;
                case SERVICE_WIN32_SHARE_PROCESS:
                    lpszServiceType = _T("SERVICE_WIN32_SHARE_PROCESS");
                    break;
                case SERVICE_KERNEL_DRIVER: 
                    lpszServiceType = _T("SERVICE_KERNEL_DRIVER");
                    break;
                case SERVICE_FILE_SYSTEM_DRIVER: 
                    lpszServiceType = _T("SERVICE_FILE_SYSTEM_DRIVER");
                    break;
                case SERVICE_INTERACTIVE_PROCESS: 
                    lpszServiceType = _T("SERVICE_INTERACTIVE_PROCESS");
                    break;
                default:
                    lpszServiceType = _T("Unknown");
            }

           /*
            *****************************************************************
            * Parse the current state into something readable
            *****************************************************************
            */
            LPCTSTR lpszCurrentState;			

            switch(m_ServiceStatus.dwCurrentState)
            {
            case SERVICE_STOPPED:
                lpszCurrentState = _T("SERVICE_STOPPED");
                break;
            case SERVICE_START_PENDING:
                lpszCurrentState = _T("SERVICE_START_PENDING");
                break;
            case SERVICE_STOP_PENDING:
                lpszCurrentState = _T("SERVICE_STOP_PENDING");
                break;
            case SERVICE_RUNNING:
                lpszCurrentState = _T("SERVICE_RUNNING");
                break;
            case SERVICE_CONTINUE_PENDING:
                lpszCurrentState = _T("SERVICE_CONTINUE_PENDING");
                break;
            case SERVICE_PAUSE_PENDING:
                lpszCurrentState = _T("SERVICE_PAUSE_PENDING");
                break;
            case SERVICE_PAUSED:
                lpszCurrentState = _T("SERVICE_PAUSED");
                break;
            default:
                lpszCurrentState = _T("Unknown");
            }

           /*
            *****************************************************************
            * Parse the controls accepted into something readable
            *****************************************************************
            */
			std::basic_string<TCHAR> str;

            if (m_ServiceStatus.dwControlsAccepted & SERVICE_ACCEPT_STOP)
            {
				str.append(_T("SERVICE_ACCEPT_STOP, "));
			}

            if (m_ServiceStatus.dwControlsAccepted & 
				SERVICE_ACCEPT_PAUSE_CONTINUE)
			{
                str.append(_T("SERVICE_ACCEPT_PAUSE_CONTINUE, "));
			}

			if (m_ServiceStatus.dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN)
			{
                str.append(_T("SERVICE_ACCEPT_SHUTDOWN, "));
			}

			if (str.size())
			{
				str.resize(str.size()-2);
			}
			else
			{
				str.append(_T("Unknown"));
			}

           /*
            *****************************************************************
            * Tracing
            *****************************************************************
            */
            McbTRACE((McbTRACESERVICE,
                _T("[%d] Update %s service status: %s\n")
                _T("\t\tdwServiceType: %s\n")
                _T("\t\tdwCurrentState: %s\n")
                _T("\t\tdwControlsAccepted: %s\n")
                _T("\t\tdwWin32ExitCode: %d\n")
                _T("\t\tdwServiceSpecificExitCode: %d\n")
                _T("\t\tdwCheckPoint: %d\n")
                _T("\t\tdwWaitHint: %d\n"),
                ::GetCurrentThreadId(), 
                m_lpszName,
                (bUpdateStatus ? _T("Succeeded") : _T("Failed")), 
                lpszServiceType, 
                lpszCurrentState, 
                str.c_str(), 
                m_ServiceStatus.dwWin32ExitCode, 
                m_ServiceStatus.dwServiceSpecificExitCode, 
                m_ServiceStatus.dwCheckPoint, 
                m_ServiceStatus.dwWaitHint))

        #endif //McbRUNASEXE

		return bUpdateStatus;
	}
#else //_DEBUG
	#ifdef McbRUNASEXE
       /*
        *********************************************************************
        * If requested  to run as a server then ignore service specific APIs
        *********************************************************************
        */
		#define McbSetServiceStatus()	TRUE
	#else //McbRUNASEXE
       /*
        *********************************************************************
        * non-debug version to update the service status calls 
		* ::SetServiceStatus() directly.
        *********************************************************************
        */
		#define McbSetServiceStatus() ::SetServiceStatus(m_hServiceStatus,	\
			&m_ServiceStatus)
	#endif //McbRUNASEXE
#endif //_DEBUG


public:
   /*
    *************************************************************************
    * C'tor
    *************************************************************************
    */
	McbServiceEntry() : m_pfnHandler(NULL) {}	
	
   /*
    *************************************************************************
    * Ensure destructor is virtual
    *************************************************************************
    */
	virtual ~McbServiceEntry() {}

   /*
    *************************************************************************
    * Obtains estimate of how long the service will take to perform 
	* operations.
	* This specifies an estimate of the amount of time, in milliseconds, that 
	* the service expects a pending start, stop, pause, or continue operation 
	* to take.  
	* See mk:@MSITStore:E:\MSDNJA~1\MSDN\dllproc.chm::/hh/winbase/
	* services_9s36.htm in the MSDN for more details.
    *************************************************************************
    */
	virtual DWORD McbGetStartWaitHint() const { return 0; }
	virtual DWORD McbGetStopWaitHint() const	{ return 0; }
	virtual DWORD McbGetPauseWaitHint() const { return 0; }
	virtual DWORD McbGetContinueWaitHint() const	{ return 0; }

   /*
    *************************************************************************
    * Overridable to control the service.  The amount of time spent in each
	* of these functions should be indicated by the wait hint functions 
	* above.
	* McbStart - should be overriden to implement initialisation
	* McbStop - should be overriden to implement stopping the service
	* McbPause - should be overriden to implement pausing the service
	* McbContinue - should be overriden to implement continuing a paused
	* 
	* Each of these functions should return NO_ERROR unless something 
	* critical occurs.  In which case the member variable m_dwSpecificError
	* can be used along with the return code to return error values to the
	* system.
	* 
	* Within each override the arguments passed to the function can be 
	* obtained with McbGetArguments().
	* 
	* Note that these functions only need implementing if the appropriate
	* control code is specified (see McbBEGINSERVICEMAP).  For example, if
	* SERVICE_ACCEPT_PAUSE_CONTINUE is not specified as a control code then
	* McbPause()/McbContinue() will never be called.
    *************************************************************************
    */
	virtual DWORD McbStart()		{ return NO_ERROR; }
	virtual DWORD McbStop()		{ return NO_ERROR; }
	virtual DWORD McbPause()		{ return NO_ERROR; }
	virtual DWORD McbContinue()	{ return NO_ERROR; }

   /*
    *************************************************************************
    * Override this to actually implement the required functionality for the
	* service.  This function will be repeatedly called until the service is
	* stopped/paused.  It should yield as quickly as possible so that the
	* thread running McbServiceMain can be responsive (and therefore the 
	* service can be stopped/started/paused/etc quickly).  If the service
	* needs to perform lengthy processing then this should be implemented on
	* another thread and CONTROLLED within McbRun().
    *************************************************************************
    */
	virtual void McbRun() = 0;

   /*
    *************************************************************************
    * Return arguments associated with the call to ServiceMain.
    *************************************************************************
    */
	LPTSTR * McbGetArguments(DWORD &dwArgs)
	{ 
		dwArgs = m_dwArgs; 
		return m_lppszArgs;
	}

   /*
    *************************************************************************
    * Implement the service main.  This is the main bones of the NT Service
	* and will be called on a different thread from the handler 
	* (McbServiceHandler).  It is designed to respond to messages from the 
	* handler (via Windows messaging) to control the service (ie stop/start/
	* pause/etc).
    *************************************************************************
    */
	void McbServiceMain(DWORD dwArgc, LPTSTR * lppszArgs)
	{
		McbTRACE((McbTRACESERVICE, _T("[%d] Service %s Main\n"), 
			::GetCurrentThreadId(), m_lpszName))
		
       /*
        *********************************************************************
        * Cache arguments
        *********************************************************************
        */
		m_dwArgs = dwArgc;
		m_lppszArgs = lppszArgs;
		m_dwSpecificError = 0;

       /*
        *********************************************************************
        * Cache the thread id so that handler can post messages to us.
        *********************************************************************
        */
		m_dwThreadId = ::GetCurrentThreadId();

       /*
        *********************************************************************
        * Call a Win32 user API and ignore the result.  This call is required 
        * because in Win32 all threads are initially created WITHOUT a 
		* message queue.  The system creates a thread-specific message queue 
		* only when the thread makes its first call to one of the Win32 User 
		* or GDI functions.  
        *
        * See Queued Messages in the MSDN for details.  Ref:
        * mk:@MSITStore:E:\MSDNJA~1\MSDN\winui.chm::/hh/winui/
		* messques_9bub.htm
        *********************************************************************
        */
        ::GetMessageTime();

/*
 ****************************************************************************
 * If requested  to run as a server then ignore service specific APIs
 ****************************************************************************
 */
#ifndef McbRUNASEXE
       /*
        *********************************************************************
        * Register the handler function for the service.  The handler 
		* function is a callback function called by the system to change the 
		* state of the service (ie stop it with SERVICE_CONTROL_STOP).
        *********************************************************************
        */
        m_hServiceStatus = ::RegisterServiceCtrlHandler(m_lpszName, 
			m_pfnHandler); 
 
       /*
        *********************************************************************
        * Check the handler was successfully registered.
        *********************************************************************
        */
        if (m_hServiceStatus == (SERVICE_STATUS_HANDLE)0) 
        { 
            McbTRACE((McbTRACESERVICE, 
				_T("[%d] Service %s RegisterServiceCtrlHandler failed.  ")
				_T("Last Error: %d\n"), ::GetCurrentThreadId(), m_lpszName, 
				::GetLastError())) 

            return; 
        } 
#endif //McbRUNASEXE
                    
        DWORD dwStatus; 

        m_ServiceStatus.dwServiceType				= 
			SERVICE_WIN32_OWN_PROCESS; 
        m_ServiceStatus.dwCurrentState				= SERVICE_START_PENDING; 
        m_ServiceStatus.dwControlsAccepted			= m_dwControlsAccepted;
        m_ServiceStatus.dwWin32ExitCode				= 0; 
        m_ServiceStatus.dwServiceSpecificExitCode	= 0; 
        m_ServiceStatus.dwCheckPoint				= 0; 
        
       /*
        *********************************************************************
        * Obtain the wait hint from the object - this an approximation of how
		* long the service will take to start.
        *********************************************************************
        */
		m_ServiceStatus.dwWaitHint					= McbGetStartWaitHint(); 

       /*
        *********************************************************************
        * Report that we are currently attempting to start...
        *********************************************************************
        */
		if (!McbSetServiceStatus()) 
        {             
            McbTRACE((McbTRACESERVICE, 
				_T("[%d] Service %s SetServiceStatus error %d\n"), 
				::GetCurrentThreadId(), m_lpszName,
				::GetLastError()))
        } 
		
       /*
        *********************************************************************
        * Call the implementation specific startup code
        *********************************************************************
        */
		McbTRACE((McbTRACESERVICE, _T("[%d] Service %s McbStart (hint %d)\n"),
			::GetCurrentThreadId(), m_lpszName, m_ServiceStatus.dwWaitHint))

        dwStatus = McbStart(); 
 
       /*
        *********************************************************************
        * Handle error condition 
        *********************************************************************
        */  
        if (dwStatus != NO_ERROR) 
        { 
            m_ServiceStatus.dwCurrentState              = SERVICE_STOPPED; 
            m_ServiceStatus.dwCheckPoint                = 0; 
            m_ServiceStatus.dwWaitHint                  = 0; 
            m_ServiceStatus.dwWin32ExitCode             = dwStatus; 
            m_ServiceStatus.dwServiceSpecificExitCode   = m_dwSpecificError; 

            McbSetServiceStatus(); 

            return; 
        } 
  
       /*
        *********************************************************************
        * Initialization complete - report running status. 
        *********************************************************************
        */
        m_ServiceStatus.dwCurrentState		 = SERVICE_RUNNING; 
        m_ServiceStatus.dwCheckPoint         = 0; 
        m_ServiceStatus.dwWaitHint           = 0; 
 
        if (!McbSetServiceStatus()) 
        {             
            McbTRACE((McbTRACESERVICE, 
				_T("[%d] Service %s SetServiceStatus error %d\n"), 
				::GetCurrentThreadId(), m_lpszName, ::GetLastError()))
        } 
               
       /*
        *********************************************************************
        * Simple message pump awaits thread calling McbServiceHandler to tell 
		* us what to do...
        *********************************************************************
        */
        MSG msg;
        bool bExit = false;		
		bool bMsg = true;
		        
        while(!bExit)
        {   			
           /*
            *****************************************************************
            * Peek for a message if the service is running and remove it from 
			* the message queue.
			* ::PeekMessage() does not block if no messages are available in
			* which case we can call code to actually implement the service.
            *****************************************************************
            */
			if (m_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
			{
				bMsg = ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE;
			}
           /*
            *****************************************************************
            * If we are not running then ::GetMessage() will do because we 
			* want to block until we get a message from the handler
            *****************************************************************
            */
            else 
			{
               /*
                *************************************************************
                * Always process the message by setting bMsg to true.
                *************************************************************
                */
				::GetMessage(&msg, NULL, 0, 0);
				bMsg = true;
			}

           /*
            *****************************************************************
            * If a message from the handler is to be processed 
            *****************************************************************
            */
			if (bMsg)
			{
#ifdef _DEBUG
                switch(m_ServiceStatus.dwCurrentState)
                {
                case SERVICE_STOPPED:            
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SERVICE_STOPPED\n"), 
						::GetCurrentThreadId(), m_lpszName))
                    break;

                case SERVICE_START_PENDING:
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SERVICE_START_PENDING\n"), 
						::GetCurrentThreadId(), m_lpszName))
                    break;
                case SERVICE_STOP_PENDING:
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SERVICE_STOP_PENDING\n"), 
						::GetCurrentThreadId(), m_lpszName))
                    break;
                case SERVICE_RUNNING:
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SERVICE_RUNNING\n"), 
						::GetCurrentThreadId(), m_lpszName))
                    break;
                case SERVICE_CONTINUE_PENDING:      
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SERVICE_CONTINUE_PENDING\n"), 
						::GetCurrentThreadId(), m_lpszName))
                    break;
                case SERVICE_PAUSE_PENDING:
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SERVICE_PAUSE_PENDING\n"), 
						::GetCurrentThreadId(), m_lpszName))
                    break;
                case SERVICE_PAUSED:
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SERVICE_PAUSED\n"), 
						::GetCurrentThreadId(), m_lpszName))
                    break;
                default:
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s Unknown state\n"), 
						::GetCurrentThreadId(), m_lpszName))

                }
#endif //_DEBUG

               /*
                *************************************************************
                * Parse wot the handler wants us to do...
                *************************************************************
                */
                switch(msg.message)
                {
               /*
                *************************************************************
                * Stop the service
                *************************************************************
                */
                case WM_SERVICE_STOP:                
                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s WM_SERVICE_STOP\n"),
						::GetCurrentThreadId(), m_lpszName))						

                    m_ServiceStatus.dwWin32ExitCode = 0; 
                    m_ServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
                    m_ServiceStatus.dwCheckPoint    = 0; 

                   /*
                    *********************************************************
                    * Obtain wait hint from the object - this an 
					* approximation of how long the service will take to 
					* stop.
                    *********************************************************
                    */
					m_ServiceStatus.dwWaitHint		= McbGetStopWaitHint();

                    if (!McbSetServiceStatus())
                    {                         
                        McbTRACE((McbTRACESERVICE, 
							_T("[%d] Service %s SetServiceStatus error ")
							_T("(while STOP_PENDING) %d\n"), 
							::GetCurrentThreadId(), m_lpszName, 
							::GetLastError()))
                    } 

                   /*
                    *********************************************************
                    * Call implementation specific stop
                    *********************************************************
                    */					
					McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s McbStop (hint %d)\n"),
						::GetCurrentThreadId(), m_lpszName, 
						m_ServiceStatus.dwWaitHint))
					
					dwStatus = McbStop(); 

                   /*
                    *********************************************************
                    * Handler errors
                    *********************************************************
                    */
					if (dwStatus != NO_ERROR) 
					{ 						
						m_ServiceStatus.dwCurrentState	= SERVICE_RUNNING; 
						m_ServiceStatus.dwCheckPoint    = 0; 
						m_ServiceStatus.dwWaitHint      = 0; 
						m_ServiceStatus.dwWin32ExitCode = dwStatus; 
						m_ServiceStatus.dwServiceSpecificExitCode   
								= m_dwSpecificError; 					

					}
                   /*
                    *********************************************************
                    * If no errors occurred then we can exit the loop and 
					* terminate the thread
                    *********************************************************
                    */
                    else 
					{           
						m_ServiceStatus.dwWin32ExitCode = 0; 
						m_ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
						m_ServiceStatus.dwCheckPoint    = 0; 
						m_ServiceStatus.dwWaitHint      = 0;             

						bExit = true;
					}
                    break;

               /*
                *************************************************************
                * Pause the service
                *************************************************************
                */
                case WM_SERVICE_PAUSE:

                    McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s WM_SERVICE_PAUSE\n"),
						::GetCurrentThreadId(), m_lpszName))						


                    m_ServiceStatus.dwWin32ExitCode = 0; 
                    m_ServiceStatus.dwCurrentState  = SERVICE_PAUSE_PENDING; 
                    m_ServiceStatus.dwCheckPoint    = 0; 

                   /*
                    *********************************************************
                    * Obtain wait hint from the object - this an 
					* approximation of how long the service will take to 
					* pause.
                    *********************************************************
                    */
					m_ServiceStatus.dwWaitHint		= McbGetPauseWaitHint();

                    if (!McbSetServiceStatus())
                    {   
                        McbTRACE((McbTRACESERVICE, 
							_T("[%d] Service %s SetServiceStatus error ")
							_T("(while PAUSE_PENDING) %d\n"), 
							::GetCurrentThreadId(), m_lpszName, 
							::GetLastError()))
						                        
                    } 

                   /*
                    *********************************************************
                    * Call implementation specific pause
                    *********************************************************
                    */
					McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s McbPause (hint %d)\n"),
						::GetCurrentThreadId(), m_lpszName, 
						m_ServiceStatus.dwWaitHint))

					dwStatus = McbPause(); 

                   /*
                    *********************************************************
                    * Handler errors
                    *********************************************************
                    */
					if (dwStatus != NO_ERROR) 
					{ 						
						m_ServiceStatus.dwCurrentState	= SERVICE_RUNNING; 
						m_ServiceStatus.dwCheckPoint    = 0; 
						m_ServiceStatus.dwWaitHint      = 0; 
						m_ServiceStatus.dwWin32ExitCode = dwStatus; 
						m_ServiceStatus.dwServiceSpecificExitCode   
								= m_dwSpecificError; 					

					}
                   /*
                    *********************************************************
                    * If no errors occurred then we can report that the 
					* service is paused...
                    *********************************************************
                    */
                    else 
					{           
						m_ServiceStatus.dwWin32ExitCode = 0; 
						m_ServiceStatus.dwCurrentState  = SERVICE_PAUSED; 
						m_ServiceStatus.dwCheckPoint    = 0; 
						m_ServiceStatus.dwWaitHint      = 0;             
					}                
                    break;

               /*
                *************************************************************
                * Continue the service
                *************************************************************
                */
                case WM_SERVICE_CONTINUE:                

					McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s WM_SERVICE_CONTINUE\n"),
						::GetCurrentThreadId(), m_lpszName))						

                    m_ServiceStatus.dwWin32ExitCode = 0; 
                    m_ServiceStatus.dwCurrentState  = 
						SERVICE_CONTINUE_PENDING; 
                    m_ServiceStatus.dwCheckPoint    = 0; 
                    
                   /*
                    *********************************************************
                    * Obtain wait hint from the object - this an 
					* approximation of how long the service will take to 
					* pause.
                    *********************************************************
                    */
					m_ServiceStatus.dwWaitHint		= 
						McbGetContinueWaitHint();

                    if (!McbSetServiceStatus())
                    {   
                        McbTRACE((McbTRACESERVICE, 
							_T("[%d] Service %s SetServiceStatus error ")
							_T("(while CONTINUE_PENDING) %d\n"), 
							::GetCurrentThreadId(), m_lpszName, 
							::GetLastError()))						
                    } 

                   /*
                    *********************************************************
                    * Call implementation specific continue
                    *********************************************************
                    */
					McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s McbContinue (hint %d)\n"),
						::GetCurrentThreadId(), m_lpszName, 
						m_ServiceStatus.dwWaitHint))

					dwStatus = McbContinue(); 

                   /*
                    *********************************************************
                    * Handler errors
                    *********************************************************
                    */
					if (dwStatus != NO_ERROR) 
					{ 						
						m_ServiceStatus.dwCurrentState	= SERVICE_PAUSED; 
						m_ServiceStatus.dwCheckPoint    = 0; 
						m_ServiceStatus.dwWaitHint      = 0; 
						m_ServiceStatus.dwWin32ExitCode = dwStatus; 
						m_ServiceStatus.dwServiceSpecificExitCode   
								= m_dwSpecificError; 					

					}
                   /*
                    *********************************************************
                    * If no errors occurred then we can report that the 
					* service is running again...
                    *********************************************************
                    */
                    else 
					{           
						m_ServiceStatus.dwWin32ExitCode = 0; 
						m_ServiceStatus.dwCurrentState  = SERVICE_RUNNING; 
						m_ServiceStatus.dwCheckPoint    = 0; 
						m_ServiceStatus.dwWaitHint      = 0;
					}										
                    break;
                
               /*
                *************************************************************
                * Report the status
                *************************************************************
                */
                case WM_SERVICE_INTEROGATE:
					McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s WM_SERVICE_INTEROGATE\n"),
						::GetCurrentThreadId(), m_lpszName));
                }

               /*
                *************************************************************
                * Attempt to set the service status
                *************************************************************
                */
                if (!McbSetServiceStatus())
                {  
					McbTRACE((McbTRACESERVICE, 
						_T("[%d] Service %s SetServiceStatus error %d\n"),
                        ::GetCurrentThreadId(), m_lpszName,
						::GetLastError()))
                } 
            }
           /*
            *****************************************************************
            * If we didn't obtain a handler message then actually call code
			* to implement the service - this should return as quick as it 
			* can so this thread is responsive to calls from the handler.
            *****************************************************************
            */
            else 
			{
				McbRun();			
			}
		}

		McbTRACE((McbTRACESERVICE, 
			_T("[%d] Service %s returning the ServiceMain Thread \n"),
			::GetCurrentThreadId(), m_lpszName))			
	}

   /*
    *************************************************************************
    * Implement the handler function by posting appropriate messages to the
	* ServiceMain thread.  The handler is called on a separate thread from
	* the service main (McbServiceMain) by the system to tell the service when
	* to change its state - ie when to stop/pause/continue/etc.
    *************************************************************************
    */
	virtual void McbServiceHandler(DWORD dwRequest)
	{
		switch(dwRequest) 
		{ 
           /*
            *****************************************************************
            * Pause the service - only post a message to the service main 
			* thread if this service can be paused/continued.
            *****************************************************************
            */
			case SERVICE_CONTROL_PAUSE: 		
				if (m_dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE &&
					m_ServiceStatus.dwCurrentState != SERVICE_PAUSED)
				{
					::PostThreadMessage(m_dwThreadId, WM_SERVICE_PAUSE, 0, 
						0);
				}
				break; 
	 
           /*
            *****************************************************************
            * Continue the service - only post a message to the service main 
			* thread if this service can be paused/continued.
            *****************************************************************
            */
			case SERVICE_CONTROL_CONTINUE:		   
				if (m_dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE &&
					m_ServiceStatus.dwCurrentState == SERVICE_PAUSED)
				{
					::PostThreadMessage(m_dwThreadId, WM_SERVICE_CONTINUE, 0, 
						0);
					
				}
	            break;
	

           /*
            *****************************************************************
            * Stop the service - only post a message to the service main 
			* thread if this service can be stopped
            *****************************************************************
            */
            case SERVICE_CONTROL_STOP:         
                if (m_dwControlsAccepted & SERVICE_ACCEPT_STOP &&
                    m_ServiceStatus.dwCurrentState != SERVICE_STOPPED)
                {
                    ::PostThreadMessage(m_dwThreadId, WM_SERVICE_STOP, 0, 0);            
                }
                break; 
 
           /*
            *****************************************************************
            * Send current status when SCM requests our status
            *****************************************************************
            */
            case SERVICE_CONTROL_INTERROGATE:         
                ::PostThreadMessage(m_dwThreadId, WM_SERVICE_INTEROGATE, 
					0, 0);                
        } 
     
	}

protected:

   /*
    *************************************************************************
    * Allows an override (ie McbPause(), McbStop() to update the service's 
	* status if it requires more time.
    *************************************************************************
    */
	void McbUpdateCurrentWaitHint(DWORD dwWait)
	{
		m_ServiceStatus.dwWaitHint = dwWait;
		m_ServiceStatus.dwCheckPoint++; 

       /*
        *********************************************************************
        * Attempt to set the service status
        *********************************************************************
        */
		if (!McbSetServiceStatus())        
        {  
            McbTRACE((McbTRACESERVICE, 
                _T("[%d] Service %s SetServiceStatus error while requested")
				_T(" more time%d\n"), ::GetCurrentThreadId(), m_lpszName,
                ::GetLastError()))
        } 
	}

   /*
    *************************************************************************
    * Called by McbServiceBase to see if the entry is initialised
    *************************************************************************
    */
	bool McbIsInitialised() const { return m_pfnHandler != NULL; }

   /*
    *************************************************************************
    * Members
    *************************************************************************
    */
	DWORD					m_dwControlsAccepted;
	DWORD					m_dwThreadId;
	SERVICE_STATUS			m_ServiceStatus; 
	SERVICE_STATUS_HANDLE   m_hServiceStatus; 
	LPCTSTR					m_lpszName;
	PFNSERVICEHANDLER		m_pfnHandler;
	DWORD					m_dwSpecificError;
	DWORD					m_dwArgs;
	LPTSTR					*m_lppszArgs;

};

/**
 ****************************************************************************
 * <P> Simple class which is a friend of McbServiceEntry to allow template
 * class McbServiceCtrl to set protected parameters of the McbServiceEntry - 
 * required because friendship is NOT inherited. </P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	5th October   	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */
class McbServiceBase
{
public:
   /*
    *************************************************************************
    * Initialise Service Entry.
    *************************************************************************
    */
	inline static void McbInitialiseEntry(McbServiceEntry &entry, 
		PFNSERVICEHANDLER pfnHandler, 
		int nServiceIndex, DWORD dwControlsAccepted)
	{
       /*
        *********************************************************************
        * Exit quickly if we are already initialised
        *********************************************************************
        */
		if (entry.McbIsInitialised())
		{
			return;
		}

       /*
        *********************************************************************
        * Obtain the service name from the dispatcher table.  This is 
		* necessary cos I cant find a way to use (unimplemented) strings as
		* parameters for template classes.
        *********************************************************************
        */
		LPCTSTR lpszServiceName = 
			(::McbGetDispatchTable())[nServiceIndex].lpServiceName;

		McbTRACE((McbTRACESERVICE, 
			_T("[%d] Preparing service %s\n"),
			::GetCurrentThreadId(), lpszServiceName))

       /*
        *********************************************************************
        * Set the ServiceHandler function, accepted controls and the service
		* name.
        *********************************************************************
        */
		entry.m_pfnHandler = pfnHandler;
		entry.m_dwControlsAccepted = dwControlsAccepted;
		entry.m_lpszName = lpszServiceName;
	}
};

/**
 ****************************************************************************
 * <P> Template class to implement calling the correct service handler and
 * service main for the required object.  The first template parameter
 * should be a class derived from McbServiceEntry.   The index is used to 
 * decipher the name or the service from the dispatcher table.  The third 
 * parameter should be the control codes the service will accept and handle 
 * in its McbServiceHandler() function.  See mk:@MSITStore:E:\MSDNJA~1\MSDN\
 * dllproc.chm::/hh/winbase/services_9s36.htm in the MSDN for details.
 * </P>
 *
 * @version     V1.0
 *
 * @author      Martyn C Brown 
 *
 * @changeHistory  
 *	3rd October   	2000	 - 	(V1.0) Creation (MCB)
 ****************************************************************************
 */
template <class theEntry, int theServiceIndex, DWORD theControlCodes> 
class McbServiceCtrl : public McbServiceBase
{	
public:
   /*
    *************************************************************************
    * Implement the service handler by calling the objects version
    *************************************************************************
    */
	inline static VOID WINAPI McbServiceHandler(DWORD dwRequest)
	{
       /*
        *********************************************************************
        * Call the service handler for the object
        *********************************************************************
        */		
		m_ServiceEntry.McbServiceHandler(dwRequest);
	}	

   /*
    *************************************************************************
    * Implement the service main by calling the object
    *************************************************************************
    */
	inline static VOID WINAPI McbServiceMain(DWORD dwArgc, LPTSTR * lppszArgv)
	{		
       /*
        *********************************************************************
        * Use the base class to initialise the service entry.  This will
		* give the address of the service handler (as per instance of this
		* template class), the name of the service and the acceptable control
		* codes which dictate what the service can do (ie can it be paused 
		* and continued).
        *********************************************************************
        */
		McbServiceBase::McbInitialiseEntry(m_ServiceEntry, 
			McbServiceHandler, theServiceIndex, theControlCodes);
		
       /*
        *********************************************************************
        * Call the service entry for the object
        *********************************************************************
        */
		m_ServiceEntry.McbServiceMain(dwArgc, lppszArgv);
		
	}

protected:
   /*
    *************************************************************************
    * Members
    *************************************************************************
    */
	static theEntry m_ServiceEntry;	
};

/*
 ****************************************************************************
 * Yuck - template static member
 ****************************************************************************
 */
template <class theEntry, int theServiceIndex, DWORD theControlCodes> 
theEntry McbServiceCtrl<theEntry, theServiceIndex, theControlCodes>::
m_ServiceEntry;

/*
 ****************************************************************************
 * Defines to create the dispatcher map which will be passed to the 
 * ::StartServiceCtrlDispatcher() API.
 * The control code is a bit field comprising of:
 *    SERVICE_ACCEPT_STOP	- if the service can be stopped.
 *	  SERVICE_ACCEPT_PAUSE_CONTINUE - if the service can be paused/continued
 * These values should be ORed together.
 ****************************************************************************
 */
#define McbBEGINSERVICEMAP()													\
const SERVICE_TABLE_ENTRY * McbGetDispatchTable()							\
{																			\
	static SERVICE_TABLE_ENTRY dispTable [] =								\
	{


#define McbSERVICEENTRY(clsEntry, lpszName, nIndex, dwControlCodes)			\
		{ (LPTSTR)lpszName,													\
		  McbServiceCtrl<clsEntry, nIndex, dwControlCodes>::McbServiceMain },	

/*
 ****************************************************************************
 * If requested required to run as a server then ignore service specific APIs
 * McbStartServices will only start the first service.
 ****************************************************************************
 */
#ifdef McbRUNASEXE

   /*
    *************************************************************************
    * Macro to end service map also creates dummy function to call the first
	* service for when the binary is run as an executable rather than a 
	* service.  This is useful for example if you want to debug stuff with 
	* boundschecker seems the damn thing wont let ya attach to a process.
    *************************************************************************
    */
	#define McbENDSERVICEMAP()												\
			{ NULL,		NULL	}											\
		};																	\
																			\
		return dispTable;													\
	}																		\
																			\
	BOOL McbDummyStartService(DWORD dwArgc, LPTSTR * lppszArgs)				\
	{																		\
		(McbGetDispatchTable()[0]).lpServiceProc(dwArgc, lppszArgs);			\
		return TRUE;														\
	}	

   /*
    *************************************************************************
    * Macro which results in calling McbServiceMain() for the first service
    *************************************************************************
    */
	#define McbStartServices(dwArgc, lppszArgs)								\
		McbDummyStartService(dwArgc, lppszArgs)


#else //McbRUNASEXE

   /*
    *************************************************************************
    * Macro to end the service map
    *************************************************************************
    */
	#define McbENDSERVICEMAP()												\
			{ NULL,		NULL	}											\
		};																	\
																			\
		return dispTable;													\
	}	

    /*
     ************************************************************************
     * Attempt to register the service with the SCM.  This function will only
     * return when all services controlled by the exe have been stopped and 
     * should be called the the exes main thread promptly after the exe 
	 * starts otherwise the Service Control Manager will think the service 
	 * has died.
     ************************************************************************
     */
    #define McbStartServices(dwArgc, lppszArgs)                              \
        ::StartServiceCtrlDispatcher(::McbGetDispatchTable())
		
															
#endif //McbRUNASEXE


#endif //McbService_Included
