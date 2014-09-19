/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   Set a time limit on a process
   =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int iPid;
char *caProgName;

void OnAlarm( int sig ) {
	kill( iPid, SIGKILL );
}

int main( int argc, char** argv ) {

int 	iSec =  10,
	iStat;
void    error( int, char*, char* );

	caProgName = argv[0];
	if( argc > 1 && argv[1][0] == '-') {
		iSec = atoi( &argv[1][1] );
		argc--;
		argv++;
	}
	if( argc < 2 ) { 
		error( 0, "\nUsage : %s [-10] command\n", caProgName );
		exit(-1);
	}

	if( (iPid = fork()) == 0 ) {
		execvp( argv[1], &argv[1] );
		error( 0,"couldn't exec %s", argv[1] );
		exit(-1);
	}
	signal( SIGALRM, OnAlarm );
	alarm( iSec );
	if( wait(&iStat) == -1 || (iStat & 0177) != 0 ) {
		error( iSec, "%s killed, timed out after %d sec", argv[1] );
		exit( -1 );
	}

	exit( (iStat >> 8) & 0377);
}


void error( int iMesg, char* cpFmt, char* cpStr ) {

char caBuffer[256];


	if( iMesg == 0 )
		sprintf( caBuffer, cpFmt, cpStr );
	else
		sprintf( caBuffer, cpFmt, cpStr, iMesg );

	perror( caBuffer );
	return;
}
