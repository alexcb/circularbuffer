#include "circular_buffer.h"

#include <assert.h>
#include <stdio.h>

int main() {
	int res;
	char *p;
	size_t n;
	CircularBuffer cb;

	init_circular_buffer( &cb, 100 );

	res = get_buffer_read( &cb, &p, &n );
	assert( res );

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

	res = get_buffer_read( &cb, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 22 + 75 );

	buffer_mark_read( &cb, 50 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 50 );

	buffer_mark_written( &cb, 50 );

	res = get_buffer_write( &cb, 1, &p, &n );
	assert( res );

	res = get_buffer_read( &cb, &p, &n );
	assert( !res );
	assert( p == cb.p + 50 );
	assert( n == 22 + 75 - 50 );

	buffer_mark_read( &cb, 47 );

	res = get_buffer_read( &cb, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 50 );

	buffer_mark_read( &cb, 50 );

	res = get_buffer_read( &cb, &p, &n );
	assert( res );

	// by this point everything has been read and buffer is now empty

	res = get_buffer_write( &cb, 101, &p, &n );
	assert( res );

	for( int i = 0; i < 10; i++ ) {
		res = get_buffer_write( &cb, 100, &p, &n );
		assert( !res );
		assert( p == cb.p );
		assert( n == 100 );

		buffer_mark_written( &cb, 100 );

		for( int j = 0; j < 100; j++ ) {
			res = get_buffer_read( &cb, &p, &n );
			assert( !res );
			assert( p == cb.p + j );
			assert( n == 100 - j );

			buffer_mark_read( &cb, 1 );
		}

	}

	printf("pass\n");
	return 0;
}
