#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "async.h"

/*#define ASYNC_DEBUG*/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_FDS 128

typedef struct {
	int fd;                         /* file descriptor */
	AsyncCallback callback;         /* callback per endpoint */
	int mask;                       /* event mask for fd */
} assoc_t;

typedef struct {
	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;
	/* consider changing to map<fd,assoc_t> */
	assoc_t endpoints[MAX_FDS];
	int num_endpoints;
	int high_fd;
} thread_data_t;


static pthread_mutex_t data_mutex;

static pthread_t thread;

static int thread_running;
static thread_data_t thread_data;


static assoc_t *search_endpoint( int fd )
{
	int i;
	assoc_t* ret = NULL;

	pthread_mutex_lock( &data_mutex );

	for( i = 0; i < thread_data.num_endpoints; i++ ) {

		if( thread_data.endpoints[i].fd == fd ) {
			ret = &thread_data.endpoints[i];
			break;
		}

	}

	pthread_mutex_unlock( &data_mutex );

	return ret;
}

static void do_callback( assoc_t* assoc, int fd, int event, int status )
{

	if( assoc->callback ) {
		assoc->callback( fd, event, status );
	}

}

static void* select_thread( void* data )
{
	fd_set read_fds, write_fds, except_fds;
	int high, fd;

	/* shut up compiler */
	data = data;

#ifdef ASYNC_DEBUG
	fprintf( stderr, "entering select_thread\n" );
#endif

	while( thread_running ) {
		struct timeval tv;
		int active;

		pthread_mutex_lock( &data_mutex );
		/* copy? */
		read_fds = thread_data.read_fds;
		write_fds = thread_data.write_fds;
		except_fds = thread_data.except_fds;

		high = thread_data.high_fd + 1;
		/* unlock here => guarded copy? */
		pthread_mutex_unlock( &data_mutex );

		/* FIXME: 10 ms timeout? */
		tv.tv_sec = 0;
		tv.tv_usec = 10000;

		active = select( high, &read_fds, &write_fds, &except_fds, &tv );

		if( active == -1 ) {
			/* Don't bother printing a message here...
			perror( "select_thread(): select()" );
			*/
			continue;
		}

		if( active == 0 ) {
			continue;
		}

		/* FIXME:
		 * starting at 0? is this *completely* braindead? why don't
		 * we just iterate over the values in endpoints[i].fd?
		 * /me wants the STL map :(
		 */
		for( fd = 0; fd < high; fd++ ) {
			assoc_t* assoc;

			assoc = search_endpoint( fd );

			if( !assoc ) {
				continue;
			}

			if( FD_ISSET( fd, &read_fds ) ) {

				if( assoc->mask & FD_ACCEPT ) {
#ifdef ASYNC_DEBUG
					fprintf( stderr, "fd %d is set for accepting\n", fd );
#endif
					do_callback( assoc, fd, FD_ACCEPT, 1 );
				} else {

					if( assoc->mask & FD_CONNECT ) {
#ifdef ASYNC_DEBUG
						fprintf( stderr, "fd %d is set for reading, but is waiting\n", fd );
#endif
					} else {
						int amount;

#ifdef ASYNC_DEBUG
						fprintf( stderr, "fd %d is set for reading\n", fd );
#endif

						if( ioctl( fd, FIONREAD, &amount ) == -1 ) {
							perror( "select_thread(): ioctl()" );
							continue;
						}

						if( !amount ) {

							if( assoc->mask & FD_CLOSE ) {
								/* Ie, do we want to hear about FD_CLOSE? */
								/* ECONNRESET? */
								do_callback( assoc, fd, FD_CLOSE, 1 );
							}

						} else {
							do_callback( assoc, fd, FD_READ, 1 );
						}

					}

				}

			}

			if( FD_ISSET( fd, &write_fds ) ) {
				/* data is ready to be written to the socket */

				if( assoc->mask & FD_CONNECT ) {
					int error = 0;
					socklen_t error_size = sizeof( error );
					int status = 1;
			
					if( getsockopt( fd, SOL_SOCKET, SO_ERROR, &error, &error_size ) == -1 ) {
						perror( "select_thread(): getsockopt()" );
					}

					if( error ) {
						status = 0;
					}

#ifdef ASYNC_DEBUG
					fprintf( stderr, "fd %d is set for connect with status %d\n", fd, status );
#endif
					assoc->mask &= ~(FD_CONNECT);
					do_callback( assoc, fd, FD_CONNECT, status );
				} else {
#ifdef ASYNC_DEBUG
					fprintf( stderr, "fd %d is set for writing\n", fd );
#endif
					do_callback( assoc, fd, FD_WRITE, 1 );
					/* to prevent checking for it repeatedly */
					pthread_mutex_lock( &data_mutex );
					FD_CLR( fd, &( thread_data.write_fds ) );
					pthread_mutex_unlock( &data_mutex );
				}

			}

			if( FD_ISSET( fd, &except_fds ) ) {
				/* exception on the socket, OOB data */
				do_callback( assoc, fd, FD_OOB, 1 );
			}

		} /* for all our fds */

	} /* while thread_running */

#ifdef ASYNC_DEBUG
	fprintf( stderr, "leaving select_thread\n" );
#endif

	return NULL;
}

int AsyncCancel( int fd )
{
	int i = 0;
	
	pthread_mutex_lock( &data_mutex );

	if( thread_data.num_endpoints <= 0 ) {
		pthread_mutex_unlock( &data_mutex );
		return -1;
	}

	FD_CLR( fd, &thread_data.read_fds );
	FD_CLR( fd, &thread_data.write_fds );
	FD_CLR( fd, &thread_data.except_fds );

	for( i = 0; i < thread_data.num_endpoints && thread_data.endpoints[i].fd != fd; i++ ) {
		/* spin */
	}

	if( i == thread_data.num_endpoints ) {
		/* we didn't find it */
		pthread_mutex_unlock( &data_mutex );
		return -1;
	}
	
	thread_data.num_endpoints--;

	for( ; i < thread_data.num_endpoints; i++ ) {
		thread_data.endpoints[i] = thread_data.endpoints[ i + 1 ];
	}

	assert( thread_data.num_endpoints >= 0 );
	pthread_mutex_unlock( &data_mutex );

	return 0;
}

int AsyncSelect( int fd, AsyncCallback callback, int mask )
{
	int waiting, listening;

	pthread_mutex_lock( &data_mutex );

	if( thread_data.num_endpoints >= MAX_FDS ) {
		pthread_mutex_unlock( &data_mutex );
		return -1;
	}

	waiting = ( mask & FD_CONNECT ) ? 1 : 0;
	listening = ( mask & FD_ACCEPT ) ? 1 : 0;

	thread_data.endpoints[thread_data.num_endpoints].fd = fd;
	thread_data.endpoints[thread_data.num_endpoints].callback = callback;
	/* this will copy over FD_CLOSE, etc. */
	thread_data.endpoints[thread_data.num_endpoints].mask = mask;
	thread_data.num_endpoints++;

#ifdef ASYNC_DEBUG
	fprintf( stderr, "added fd %d (%d/%d), %d total\n", fd, waiting,
		 listening, thread_data.num_endpoints );
#endif

	if( fcntl( fd, F_SETFL, fcntl( fd, F_GETFL ) | O_NONBLOCK ) < 0 ) {
		perror( "AsyncSelect(): fcntl()" );
		return -1;
	}

	if( fd > thread_data.high_fd ) {
		thread_data.high_fd = fd;
	}

	if( mask & FD_READ || mask & FD_ACCEPT ) {
		FD_SET( fd, &thread_data.read_fds );
	}

	if( mask & FD_WRITE || mask & FD_CONNECT ) {
		FD_SET( fd, &thread_data.write_fds );
	}

	if( mask & FD_OOB ) {
		FD_SET( fd, &thread_data.except_fds );
	}

	pthread_mutex_unlock( &data_mutex );

	return 0;
}

int AsyncShutdown( void )
{
	thread_running = FALSE;

	pthread_join( thread, 0 );

	pthread_mutex_destroy( &data_mutex );

	return 0;
}

int AsyncInit( void )
{
	pthread_mutex_init( &data_mutex, 0 );

	thread_running = TRUE;

	FD_ZERO( &thread_data.read_fds );
	FD_ZERO( &thread_data.write_fds );
	FD_ZERO( &thread_data.except_fds );

	thread_data.high_fd = 0;
	thread_data.num_endpoints = 0;

	if( pthread_create( &thread, 0, select_thread, 0 ) < 0 ) {
		perror( "AsyncInit(): pthread_create()" );
		return -1;
	}

	return 0;
}
