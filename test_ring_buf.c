/**
 * @file math/ring_buffer.c
 *
 * @brief      test of ring_buf.h
 *
 *
 * @author     James Strawson
 * @date       2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RINGBUF_TYPE int
#include "ring_buf.h"


#define SIZE 3

static void print_buffer_contents(rc_ringbuf_t* buf_ptr)
{
	int i;
	RINGBUF_TYPE val;
	printf("contents: ");
	for(i=0;i<SIZE;i++){
		rc_ringbuf_get_value(buf_ptr, i, &val);
		printf("%d ", val);
	}
	printf("\n");
	return;
}

static void print_buffer_contents_ptr(rc_ringbuf_t* buf_ptr)
{
	int i;
	RINGBUF_TYPE * val_ptr;
	printf("contents: ");
	for(i=0;i<SIZE;i++){
		rc_ringbuf_get_value_ptr(buf_ptr, i, &val_ptr);
		printf("%d ", *val_ptr);
	}
	printf("\n");
	return;
}

int main()
{
	int i;
	rc_ringbuf_t buf = RC_RINGBUF_INITIALIZER;

	printf("Allocating ringbuffer of size: %d\n", SIZE);
	rc_ringbuf_alloc(&buf, SIZE);

	// print contents of empty buffer
	printf("Printing empty buffer contents, should contain: 0 0 0\n");
	print_buffer_contents(&buf);
	

	// fill the ring buffer and print contents
	for(i=0;i<SIZE; i++) rc_ringbuf_insert(&buf, (RINGBUF_TYPE)i);
	printf("put 1,2,3 into buffer, should contain: 2 1 0\n");
	print_buffer_contents(&buf);

	// fill the ring buffer again and print contents
	for(i=0;i<SIZE+1; i++) rc_ringbuf_insert( &buf, (RINGBUF_TYPE)i );
	printf("put 1,2,3,4 into buffer, should contain: 3 2 1\n");
	print_buffer_contents(&buf);
	
	printf("Reading back same contents but straight from memory, should contain: 3 2 1\n");
	print_buffer_contents_ptr(&buf);
	
	rc_ringbuf_free(&buf);

	printf("DONE\n");
	return 0;

}