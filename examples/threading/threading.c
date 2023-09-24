#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

	#define MS_TO_NS	(1e6)
	
	int ret;
	bool thread_success = false;
	
	struct thread_data *td = (struct thread_data *)thread_param;
	struct timespec req = { .tv_sec = 0, .tv_nsec = 0 };

	// wait before acquiring mutex	
	req.tv_nsec = td->wait_to_obtain_ms * MS_TO_NS;
	ret = nanosleep( &req, NULL );
	if( ret )
		perror( "nanosleep" );	
	else
	{
		// obtain mutex	
		ret = pthread_mutex_lock( td->mutex );
		if( ret )
			perror( "mutex lock");
		else
		{
			// wait before releasing mutex	
			req.tv_nsec = td->wait_to_release_ms * MS_TO_NS;
			ret = nanosleep( &req, NULL );
			if( ret )
				perror( "nanosleep" );
			else
			{
				// release mutex
				ret = pthread_mutex_unlock( td->mutex );
				if( ret )
					perror( "mutex unlock" );
				else
					thread_success = true;
			}
		}
	}
	
	td->thread_complete_success = thread_success;   
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
     
     // Validate arguments
    if( mutex == NULL )
	{
		// Invalid mutex
		ERROR_LOG( "Invalid mutex." );
		return false;
	}     
    
    // Create and initialize thread data     
	struct thread_data *td = (struct thread_data *)malloc( sizeof( struct thread_data ) );
	if( td == NULL )
	{
		// Couldn't allocate the memory
		ERROR_LOG( "Couldn't allocate thread data." );
		return false;
	}     
		    
	td->mutex = mutex;
	td->wait_to_obtain_ms = wait_to_obtain_ms;
	td->wait_to_release_ms = wait_to_release_ms;
	td->thread_complete_success = false;
	
	// Create and start the thread, setting the 'thread' argument to its PID, passing the function and data object to it
	int rc = pthread_create( thread, NULL, threadfunc, td );
	if( rc != 0 )
	{
		ERROR_LOG( "Couldn't create thread, $s" );
		// Since no thread was created, the caller will not get a pointer to the malloc'd data to allow the caller to free it.
		free( td );	
		return false;
	}
	
    return true;
}

