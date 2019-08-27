/**
 * @file math/ring_buffer.c
 *
 * @brief      Ring buffer implementation for double-precision doubles
 *
 *             Ring buffers are FIFO (first in first out) buffers of fixed
 *             length which efficiently boot out the oldest value when full.
 *             They are particularly well suited for storing the last n values
 *             in a discrete time filter.
 *
 *             The user creates their own instance of a buffer and passes a
 *             pointer to the these ring_buf functions to perform normal
 *             operations.
 *
 * @author     James Strawson
 * @date       2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define FIFOBUF_TYPE int
#include "fifo_buf.h"


#define SIZE 3

static void print_buffer_contents(rc_fifobuf_t* buf_ptr)
{
	FIFOBUF_TYPE val;
	rc_fifobuf_pop(buf_ptr, &val);
	printf("%d ", val);
	return;
}

static void print_buffer_contents_ptr(rc_fifobuf_t* buf_ptr)
{
	FIFOBUF_TYPE * val_ptr;
	rc_fifobuf_pop_ptr(buf_ptr, &val_ptr);
	printf("%d ", *val_ptr);
	return;
}

int main()
{
	int i;
	FIFOBUF_TYPE val;
	rc_fifobuf_t buf = RC_FIFOBUF_INITIALIZER;

	printf("Allocating fifobuffer of size: %d\n", SIZE);
	rc_fifobuf_alloc(&buf, SIZE);

	printf("testing read of empty buffer, pop should return -1\n");
	printf("pop returned: %d\n", rc_fifobuf_pop(&buf, &val));
	printf("testing available values in empty buffer, available should return 0\n");
	printf("available returned: %d\n", rc_fifobuf_available(&buf));


	printf("adding 1 to the buffer\n");
	rc_fifobuf_push(&buf,1);
	printf("available returned: %d\n", rc_fifobuf_available(&buf));
	printf("poping 1 from buffer\n");
	printf("pop returned: %d\n", rc_fifobuf_pop(&buf, &val));
	printf("pop read out: %d\n", val);
	printf("available returned: %d\n", rc_fifobuf_available(&buf));

	printf("popping on empty buffer, should return -1\n");
	printf("pop returned: %d\n", rc_fifobuf_pop(&buf, &val));


	printf("adding 2,3,4 to the buffer\n");
	for(i=2;i<=4;i++) rc_fifobuf_push(&buf,i);

	printf("try pushing 5, should return 0 since it's full\n");
	printf("push returned: %d\n", rc_fifobuf_push(&buf, 5));

	printf("available returned: %d\n", rc_fifobuf_available(&buf));

	printf("popping all 3 from buffer\n");
	for(i=0;i<SIZE;i++) print_buffer_contents(&buf);
	printf("\n");
	printf("available returned: %d\n", rc_fifobuf_available(&buf));


	printf("adding 1 to the buffer\n");
	rc_fifobuf_push(&buf,1);
	printf("available returned: %d\n", rc_fifobuf_available(&buf));
	printf("poping 1 from buffer\n");
	printf("pop returned: %d\n", rc_fifobuf_pop(&buf, &val));
	printf("pop read out: %d\n", val);
	printf("available returned: %d\n", rc_fifobuf_available(&buf));

	printf("adding 5,6 to the buffer\n");
	rc_fifobuf_push(&buf,5);
	rc_fifobuf_push(&buf,6);
	printf("available returned: %d\n", rc_fifobuf_available(&buf));
	print_buffer_contents_ptr(&buf);
	printf("\n");
	printf("available returned: %d\n", rc_fifobuf_available(&buf));
	print_buffer_contents_ptr(&buf);
	printf("\n");
	printf("available returned: %d\n", rc_fifobuf_available(&buf));

	rc_fifobuf_free(&buf);

	printf("DONE\n");
	return 0;

}