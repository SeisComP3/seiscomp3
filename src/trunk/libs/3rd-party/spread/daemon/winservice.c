/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2014 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Amy Babay            babay@cs.jhu.edu - accelerated ring protocol.
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */

#include <stdlib.h>

#include "spu_alarm.h"
#include "sp_events.h"

#include <Windows.h>
#include <Winsvc.h>

// The name used by Windows to refer to this service
#define SERVICE_NAME "spread"
#define DISPLAY_NAME "Spread Service"
#define SERVICE_DESCRIPTION_STR "Provides distributed message based communications services."

/* ************************************************************************* */

static void ErrorMessageBox(const char *faultMsg, DWORD err)
{
	LPVOID lpMsgBuf;
	char szBuf[1024];
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL);
	sprintf_s(szBuf, sizeof(szBuf), "%s Error %d: %s", faultMsg, err, lpMsgBuf);
	MessageBox(NULL, szBuf, DISPLAY_NAME, MB_OK);
	LocalFree(lpMsgBuf);
}

/* ************************************************************************* */

static BOOL InstallService()
{
    SERVICE_DESCRIPTION sdBuf;
    SC_HANDLE hSCManager;
    SC_HANDLE hService;

    char lpszBinaryPathName[_MAX_PATH + 10];

	if (!GetModuleFileName(NULL, lpszBinaryPathName, _MAX_PATH)) {
        ErrorMessageBox(TEXT("Unable to get the executable's path."), GetLastError());
        return FALSE;
    }

	strcat(lpszBinaryPathName, TEXT(" --service"));

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    
    if (hSCManager == NULL) {
        ErrorMessageBox(TEXT("Unable to open the service manager."), GetLastError());
        return FALSE;
    }

    hService = CreateService(hSCManager, 
        SERVICE_NAME, // service name
        DISPLAY_NAME, // service name to display
        SERVICE_ALL_ACCESS, // desired access
        SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, // service type
        SERVICE_DEMAND_START, // start type 
        SERVICE_ERROR_NORMAL, // error control type 
        lpszBinaryPathName, // service's binary 
        NULL, // no load ordering group 
        NULL, // no tag identifier 
        TEXT("\0"), // no dependencies
        NULL, // LocalSystem account
        NULL); // no password
    
    if (hService == NULL) {
        ErrorMessageBox(TEXT("Unable to create the service."), GetLastError());
        CloseServiceHandle(hService);
        return FALSE;
    }

    sdBuf.lpDescription = (LPTSTR) SERVICE_DESCRIPTION_STR;
    if (!ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sdBuf)) {
        ErrorMessageBox(TEXT("Unable to change the service description."), GetLastError());
        return FALSE;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return TRUE;
}

/* ************************************************************************* */

static BOOL UninstallService()
{
    SC_HANDLE hSCManager;
    SC_HANDLE hService;
    SERVICE_STATUS status;
    BOOL bDelete;

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCManager == NULL) {
        ErrorMessageBox(TEXT("Couldn't open service manager."), GetLastError());
        return FALSE;
    }

    hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);

    if (hService == NULL) {
        ErrorMessageBox(TEXT("Couldn't open service."), GetLastError());
        CloseServiceHandle(hSCManager);
        return FALSE; 
    }

    memset(&status, 0, sizeof(status));
    ControlService(hService, SERVICE_CONTROL_STOP, &status);

    bDelete = DeleteService(hService);

    CloseServiceHandle( hService );
    CloseServiceHandle( hSCManager );

    if (!bDelete) {
        ErrorMessageBox(TEXT("Service could not be uninstalled."), GetLastError());
        return FALSE;
    }
    return TRUE;
}

/* ************************************************************************* */

SERVICE_STATUS m_ServiceStatus;
SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
extern int SpreadMain(int argc, char *argv[]);

static void WINAPI ServiceCtrlHandler(DWORD Opcode)
{
    switch(Opcode)
    {
    case SERVICE_CONTROL_PAUSE: 
        m_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    case SERVICE_CONTROL_CONTINUE:
        m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        break;

    case SERVICE_CONTROL_STOP:
		E_exit_events();
		m_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        m_ServiceStatus.dwWin32ExitCode = NO_ERROR;
        m_ServiceStatus.dwCheckPoint = 0;
        m_ServiceStatus.dwWaitHint = 0;
        SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        break;
    }
    return;
}

/* ************************************************************************* */

static void spread_atexit(void)
{
    m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
}

/* ************************************************************************* */

//static int    My_Argc;
//static char **My_Argv;

static void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
    DWORD status;

    m_ServiceStatus.dwServiceType = SERVICE_WIN32;
    m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    m_ServiceStatus.dwServiceSpecificExitCode = 0;
    m_ServiceStatus.dwCheckPoint = 0;
    m_ServiceStatus.dwWaitHint = 0;

    m_ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler); 
    if (m_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    {
        return;
    }
    m_ServiceStatus.dwWaitHint = 0;
    m_ServiceStatus.dwCheckPoint = 0;
    m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);
    
    atexit( spread_atexit );

    status = SpreadMain(argc, argv);

	if (status != 0) {
        m_ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        m_ServiceStatus.dwServiceSpecificExitCode = status;
    }
    m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

	return;
}

/* ************************************************************************* */

int main(int argc, char *argv[])
{
	//My_Argc = argc;
	//My_Argv = argv;

    if (argc > 1)
    {
        if (_stricmp(argv[1], "--service") == 0)
        {
            SERVICE_TABLE_ENTRY DispatchTable[] = {
                {SERVICE_NAME, ServiceMain},
                {NULL, NULL}
            };
			StartServiceCtrlDispatcher(DispatchTable);
            return 0;    
        }

        if (_stricmp(argv[1], "--install-service") == 0)
        {
            return InstallService() ? 0 : 1;
        }

        if (_stricmp(argv[1], "--uninstall-service") == 0)        
        {
            return UninstallService() ? 0 : 1;
        }
    } 
    
    return SpreadMain(argc, argv);
}

/* ************************************************************************* */
