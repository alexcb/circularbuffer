#include "circular_buffer.h"

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


int basic_test() {
	int res;
	char *p;
	size_t n;

	char *q[2];
	size_t size[2];
	CircularBuffer cb;

	init_circular_buffer( &cb, 100 );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 0 );
	assert( size[1] == 0 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 100 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 100 );

	buffer_mark_written( &cb, 22 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p + 22 );
	assert( n == 78 );

	buffer_mark_written( &cb, 75 );

	res = get_buffer_write( &cb, 1, &p, &n );
	assert( !res );
	assert( p == cb.p + 22 + 75 );
	assert( n == 3 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( res );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 97 );
	assert( size[1] == 0 );

	buffer_mark_read_unsafe( &cb, 50 );
	
	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 50 );

	buffer_mark_written( &cb, 50 );

	res = get_buffer_write( &cb, 1, &p, &n );
	assert( res );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 47 );
	assert( size[1] == 50 );

	buffer_mark_read_unsafe( &cb, 20 );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 27 );
	assert( size[1] == 50 );

	buffer_mark_read_unsafe( &cb, 27 );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 50 );
	assert( size[1] == 0 );
	assert( cb.len == 0 );
	assert( cb.read == 0 );

	buffer_mark_read_unsafe( &cb, 35 );
	assert( cb.len == 0 );
	assert( cb.read == 35 );
	assert( cb.write == 50 );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 15 );
	assert( size[1] == 0 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p + 50 );
	assert( n == 50 );

	buffer_mark_written( &cb, 45 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 35 );

	buffer_mark_written( &cb, 15 );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 60 );
	assert( size[1] == 15 );

	buffer_mark_read_unsafe( &cb, 66 );
	assert( cb.len == 0 );
	printf("got %d\n", cb.read);
	assert( cb.read == 6 );

	res = get_buffer_read_unsafe2( &cb, 10000, &q[0], &size[0], &q[1], &size[1] );
	assert( size[0] == 15-6 );
	assert( size[1] == 0 );


//	res = get_buffer_read( &cb, 100, &p, &n );
//	assert( !res );
//	assert( p == cb.p + 50 );
//	assert( n == 22 + 75 - 50 );
//
//	buffer_mark_read( &cb, 47 );
//
//	res = get_buffer_read( &cb, 100, &p, &n );
//	assert( !res );
//	assert( p == cb.p );
//	assert( n == 50 );
//
//	buffer_mark_read( &cb, 50 );
//
//	res = get_buffer_read( &cb, 100, &p, &n );
//	assert( res );

	printf("pass basic test\n");
	return 0;
}


int main() {
	basic_test();
	//thread_test();
	//rewind_test();
}
