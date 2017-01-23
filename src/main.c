#include "circular_buffer.h"

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


int basic_test() {
	int res;
	char *p;
	size_t n;
	CircularBuffer cb;

	init_circular_buffer( &cb, 100 );

	res = get_buffer_read( &cb, 100, &p, &n );
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

	res = get_buffer_read( &cb, 100, &p, &n );
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

	res = get_buffer_read( &cb, 100, &p, &n );
	assert( !res );
	assert( p == cb.p + 50 );
	assert( n == 22 + 75 - 50 );

	buffer_mark_read( &cb, 47 );

	res = get_buffer_read( &cb, 100, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 50 );

	buffer_mark_read( &cb, 50 );

	res = get_buffer_read( &cb, 100, &p, &n );
	assert( res );

	printf("pass basic test\n");
	return 0;
}

void producer( void *data )
{
	size_t min_buffer_size;
	size_t buffer_free;
	size_t *payload_size;
	size_t bytes_written;
	char *p;
	int res;
	size_t start;
	int i;

	CircularBuffer *cb = (CircularBuffer*) data;
	printf("starting producer\n");

	for( i = 0; i < 1000; ) {
		min_buffer_size = 53 + i % 23;
		res = get_buffer_write( cb, min_buffer_size, &p, &buffer_free );
		if( res ) {
			continue;
		}
		start = p - cb->p;
		printf("producer: writing at %lu available %lu\n", start, buffer_free);

		// magic num
		*((unsigned char*)p) = 55;
		p++;
		buffer_free--;
		bytes_written = 1;

		// len
		payload_size = (size_t*) p;
		p += sizeof(size_t);
		buffer_free -= sizeof(size_t);
		bytes_written += sizeof(size_t);

		buffer_free = buffer_free / 2 - i % 19;
		*payload_size = buffer_free;
		bytes_written += buffer_free;

		*((int*) p) = i++;

		buffer_mark_written( cb, bytes_written );
		printf("producer: wrote %lu\n", bytes_written);
	}
	printf("producer done\n");
}

void consumer( void *data )
{
	size_t buffer_avail;
	size_t payload_size;
	size_t bytes_written;
	char *p;
	unsigned char payload_id;
	int res;
	int i;
	size_t start;

	int payload;

	CircularBuffer *cb = (CircularBuffer*) data;
	printf("starting consumer\n");

	for( i = 0; i < 1000; ) {
		res = get_buffer_read( cb, 10000, &p, &buffer_avail );
		if( res ) {
			continue;
		}
		start = p - cb->p;
		printf("consumer: reading at %lu available %lu\n", start, buffer_avail);

		// magic num
		payload_id = *(unsigned char*) p;
		p++;
		buffer_avail--;
		buffer_mark_read( cb, 1 );

		printf("consumer: payload_id %d\n", payload_id);
		assert( payload_id == 55);

		// len
		payload_size = *(size_t*) p;
		p += sizeof(size_t);
		buffer_avail -= sizeof(size_t);
		buffer_mark_read( cb, sizeof(size_t) );

		assert(payload_size <= buffer_avail);

		payload = *((int*)p);
		assert( payload == i++ );

		buffer_mark_read( cb, payload_size );
	}
	printf("consumer received all messages\n");
}

int thread_test() {
	pthread_t producer_thread;
	pthread_t consumer_thread;

	CircularBuffer cb;
	init_circular_buffer( &cb, 1024 );

	pthread_create( &producer_thread, NULL, (void *) &producer, (void *) &cb);
	pthread_create( &consumer_thread, NULL, (void *) &consumer, (void *) &cb);

	pthread_join( producer_thread, NULL );
	pthread_join( consumer_thread, NULL );
}

int rewind_test() {
	int res;
	char *p;
	size_t n;
	CircularBuffer cb;

	init_circular_buffer( &cb, 100 );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 100 );

	buffer_rewind_lock( &cb );
	res = buffer_rewind_and_unlock( &cb, p );
	assert( !res );

	res = get_buffer_write( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 100 );

	buffer_mark_written( &cb, 25 );

	res = get_buffer_read( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 10 );

	buffer_mark_read( &cb, 10 );

	buffer_rewind_lock( &cb );
	res = buffer_rewind_and_unlock( &cb, p+15 );
	assert( !res );
	assert( cb.write = 15 );

	res = get_buffer_read( &cb, 10, &p, &n );
	assert( !res );
	assert( p == cb.p + 10 );
	assert( n == 5 );
	
	buffer_mark_read( &cb, 5 );

	res = get_buffer_read( &cb, 10, &p, &n );
	assert( res );

	// special case write causes snap back to 0
	res = get_buffer_write( &cb, 50, &p, &n );
	assert( !res );
	assert( n == 100 );
	assert( p == cb.p );

	buffer_mark_written( &cb, 85 );

	res = get_buffer_read( &cb, 80, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 80 );
	
	buffer_mark_read( &cb, 80 );

	res = get_buffer_write( &cb, 50, &p, &n );
	assert( !res );
	assert( n == 80 );
	assert( p == cb.p );

	buffer_mark_written( &cb, 50 );

	buffer_rewind_lock( &cb );
	res = buffer_rewind_and_unlock( &cb, p+30 );
	assert( !res );
	assert( cb.write = 30 );

	res = get_buffer_read( &cb, 80, &p, &n );
	assert( !res );
	assert( p == cb.p + 80 );
	assert( n == 5 );

	buffer_mark_read( &cb, 5 );

	res = get_buffer_read( &cb, 80, &p, &n );
	assert( !res );
	assert( p == cb.p );
	assert( n == 30 );

	return 0;
}

int main() {
	basic_test();
	thread_test();
	rewind_test();
}
