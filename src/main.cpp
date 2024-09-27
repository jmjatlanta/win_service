#include <windows.h>
#include <tchar.h>

/***
 * This example was built based on 
 * https://www.codeproject.com/Articles/499465/Simple-Windows-Service-in-Cplusplus
*/

// globals

SERVICE_STATUS serviceStatus = {0};
SERVICE_STATUS_HANDLE statusHandle = nullptr;
HANDLE serviceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME _T("My Sample Service")

/***
 * Main entry point
*/
int _tmain(int argc, TCHAR *argv[])
{
    SERVICE_TABLE_ENTRY serviceTable[] = {
            { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
            { nullptr, nullptr }
        };

    if (StartServiceCtrlDispatcher(serviceTable) == FALSE)
        return GetLastError();
    return 0;
}

/**
 * When the service control dispatcher wants to start the service, this is the method it calls...
 * This is a blocking function. It will not return until the service is shut down
 */
VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;
 
    // Register our service control handler with the SCM
    statusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);
 
    if (statusHandle == NULL) 
    {
        goto EXIT;
    }
 
    // Tell the service controller we are starting
    ZeroMemory (&serviceStatus, sizeof (serviceStatus));
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwControlsAccepted = 0;
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    serviceStatus.dwWin32ExitCode = 0;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwCheckPoint = 0;
 
    if (SetServiceStatus (statusHandle , &serviceStatus) == FALSE)
    {
        OutputDebugString(_T(
          "My Sample Service: ServiceMain: SetServiceStatus returned error"));
    }
 
    /*
     * Perform tasks necessary to start the service here
     */
 
    // Create a service stop event to wait on later
    serviceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (serviceStopEvent == NULL) 
    {   
        // Error creating event
        // Tell service controller we are stopped and exit
        serviceStatus.dwControlsAccepted = 0;
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        serviceStatus.dwWin32ExitCode = GetLastError();
        serviceStatus.dwCheckPoint = 1;
 
        if (SetServiceStatus (statusHandle, &serviceStatus) == FALSE)
        {
            OutputDebugString(_T(
              "My Sample Service: ServiceMain: SetServiceStatus returned error"));
        }
        goto EXIT; 
    }    
    
    // Tell the service controller we are started
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    serviceStatus.dwWin32ExitCode = 0;
    serviceStatus.dwCheckPoint = 0;
 
    if (SetServiceStatus (statusHandle, &serviceStatus) == FALSE)
    {
        OutputDebugString(_T(
          "My Sample Service: ServiceMain: SetServiceStatus returned error"));
    }
 
    // Start a thread that will perform the main task of the service
    HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
   
    // Wait until our worker thread exits signaling that the service needs to stop
    WaitForSingleObject (hThread, INFINITE);
   
    
    /*
     * Perform any cleanup tasks 
     */
 
    CloseHandle (serviceStopEvent);
 
    // Tell the service controller we are stopped
    serviceStatus.dwControlsAccepted = 0;
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    serviceStatus.dwWin32ExitCode = 0;
    serviceStatus.dwCheckPoint = 3;
 
    if (SetServiceStatus (statusHandle, &serviceStatus) == FALSE)
    {
        OutputDebugString(_T(
          "My Sample Service: ServiceMain: SetServiceStatus returned error"));
    }
    
EXIT:
    return;
}

/***
 * The Service Control Manager calls this method when it wants to control the service
 */
VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    switch (CtrlCode) 
    {
        case SERVICE_CONTROL_STOP :
            if (serviceStatus.dwCurrentState != SERVICE_RUNNING)
                break;

            /* 
             * Perform tasks necessary to stop the service here 
            */

            serviceStatus.dwControlsAccepted = 0;
            serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            serviceStatus.dwWin32ExitCode = 0;
            serviceStatus.dwCheckPoint = 4;

            if (SetServiceStatus (statusHandle, &serviceStatus) == FALSE)
            {
                OutputDebugString(_T(
                    "My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error"));
            }

            // This will signal the worker thread to start shutting down
            SetEvent (serviceStopEvent);

            break;

        default:
            break;
    }
}

/***
 * This is where the real work is done
 */
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(serviceStopEvent, 0) != WAIT_OBJECT_0)
    {        
        /* 
         * Perform main service function here
         */
 
        //  Simulate some work by sleeping
        Sleep(3000);
    }
 
    return ERROR_SUCCESS;
}
