/*	The following program is a 'c' alternative to the writer.sh test script in assignment 1
	Performs the following:
	Accepts the following arguments: the first argument is a full path to a file (including filename) on the
	filesystem, referred to below as writefile; the second argument is a text string which will be written within
	this file, referred to below as writestr.
	
	If any of the arguments above were not specified, logs the error with LOG_ERR level and exits with value 1.
	
	Creates a new file with name and path writefile with content writestr, overwriting any existing file, and
	logging this activity with LOG_DEBUG level. The directory is assumed to exist.
	
	If the file could not be created, written to, or closed properly, logs the error with LOG_ERR level and exits with value 1.

	Example:
	writer /tmp/aesd/assignment1/sample.txt ios

	Creates file:
	/tmp/aesd/assignment1/sample.txt
	
	With content:
	ios
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>

static void LogFileErr( const char* file );

int main( int argc, char *argv[] )
{

	// Open the log for use - docs don't mention this sets errno, so I'm not checking it
	openlog( NULL, 0, LOG_USER );
	
	// Check # of args
	if( argc < 3)
	{
		syslog( LOG_ERR, "Invalid number of arguments: %d", argc );
		closelog();
		return 1;
	}

	// Just to keep things straight
	const char* writefile = argv[1];
	const char* writestr = argv[2];
	
	int fd;
	
	// Create the file
	fd = creat( writefile, 0664 );
	if( fd == -1 )
	{
		LogFileErr( writefile );
		return 1;
	}
	
	// Write the 2nd arg to the file
	syslog( LOG_DEBUG, "Writing %s to %s", writestr, writefile );
	
	ssize_t nr = write( fd, writestr, strlen( writestr ) + 1 );
	if( nr == -1 )
	{
		LogFileErr( writefile );
		if( close( fd ) == -1 )
			LogFileErr( writefile );		
		return 1;
	}

	// Close the file
	if( close( fd ) == -1 )
	{
		LogFileErr( writefile );	
		return 1;
	}
}

/* Logs errno and ensures the log is closed in preparation for exit */
static void LogFileErr( const char* file )
{
	syslog( LOG_ERR, "%s: %s", file, strerror( errno ) );
	closelog();	
}

