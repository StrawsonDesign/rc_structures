/**
 * "ring_buf.h"
 *
 * @brief      ring buffer implementation for arbitrary datatypes
 *
 * Ring buffers are FIFO (first in first out) buffers of fixed length which
 * efficiently boot out the oldest value when full. They are particularly well
 * suited for storing the last n values in a discrete time filter.
 *
 * The user creates their own instance of a buffer and passes a pointer to the
 * these functions to perform normal operations.
 *
 * @author     James Strawson
 * @date       2019
 *
 */


#ifndef RINGBUF_TYPE
#error "ERROR user must #define RINGBUF_TYPE before including ring_buf.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef unlikely
#define unlikely(x)	__builtin_expect (!!(x), 0)
#endif

#ifndef likely
#define likely(x)	__builtin_expect (!!(x), 1)
#endif


/**
 * @brief      Struct containing state of a ringbuffer and pointer to
 * dynamically allocated memory.
 */
typedef struct rc_ringbuf_t {
	RINGBUF_TYPE* d;	///< pointer to dynamically allocated data
	int size;	///< number of elements the buffer can hold
	int index;	///< index of the most recently added value
	int initialized;///< flag indicating if memory has been allocated for the buffer
} rc_ringbuf_t;


#define RC_RINGBUF_INITIALIZER {\
	.d = NULL,\
	.size = 0,\
	.index = 0,\
	.initialized = 0}

/**
 * @brief      Returns an rc_ringbuf_t struct which is completely zero'd out
 * with no memory allocated for it.
 *
 * This is essential for declaring new ring buffers since structs declared
 * inside of functions are not necessarily zero'd out which can cause the struct
 * to contain problematic contents leading to segfaults. New ring buffers should
 * be initialized with this before calling rc_ringbuf_alloc.
 *
 * @return     empty and ready-to-allocate rc_ringbuf_t
 */
rc_ringbuf_t rc_ringbuf_empty(void)
{
	rc_ringbuf_t out = RC_RINGBUF_INITIALIZER;
	return out;
}

/**
 * @brief      Allocates memory for a ring buffer and initializes an
 * rc_ringbuf_t struct.
 *
 * If buf is already the right size then it is left untouched. Otherwise any
 * existing memory allocated for buf is freed to avoid memory leaks and new
 * memory is allocated.
 *
 * @param      buf   Pointer to user's buffer
 * @param[in]  size  Number of elements to allocate space for
 *
 * @return     Returns 0 on success or -1 on failure.
 */
int rc_ringbuf_alloc(rc_ringbuf_t* buf, int size)
{
	// sanity checks
	if(unlikely(buf==NULL)){
		fprintf(stderr,"ERROR in rc_ringbuf_alloc, received NULL pointer\n");
		return -1;
	}
	if(unlikely(size<2)){
		fprintf(stderr,"ERROR in rc_ringbuf_alloc, size must be >=2\n");
		return -1;
	}
	// if it's already allocated, nothing to do
	if(buf->initialized && buf->size==size && buf->d!=NULL) return 0;
	// make sure it's zero'd out
	buf->size = 0;
	buf->index = 0;
	buf->initialized = 0;
	// free memory and allocate fresh
	free(buf->d);
	buf->d = (RINGBUF_TYPE*)calloc(size,sizeof(RINGBUF_TYPE));
	if(buf->d==NULL){
		fprintf(stderr,"ERROR in rc_ringbuf_alloc, failed to allocate memory\n");
		return -1;
	}
	// write out other details
	buf->size = size;
	buf->initialized = 1;
	return 0;
}

/**
 * @brief      Frees the memory allocated for buffer buf.
 *
 * Also set the initialized flag to 0 so other functions don't try to access
 * unallocated memory.
 *
 * @param      buf   Pointer to user's buffer
 *
 * @return     Returns 0 on success or -1 on failure.
 */
int rc_ringbuf_free(rc_ringbuf_t* buf)
{
	rc_ringbuf_t new = RC_RINGBUF_INITIALIZER;
	if(unlikely(buf==NULL)){
		fprintf(stderr, "ERROR in rc_ringbuf_free, received NULL pointer\n");
		return -1;
	}
	if(buf->initialized) free(buf->d);
	*buf = new;
	return 0;
}

/**
 * @brief      Sets all values in the buffer to 0.0f and sets the buffer index
 * back to 0.
 *
 * @param      buf   Pointer to user's buffer
 *
 * @return     Returns 0 on success or -1 on failure.
 */
int rc_ringbuf_reset(rc_ringbuf_t* buf)
{
	// sanity checks
	if(unlikely(buf==NULL)){
		fprintf(stderr, "ERROR in rc_ringbuf_reset, received NULL pointer\n");
		return -1;
	}
	if(unlikely(!buf->initialized)){
		fprintf(stderr,"ERROR rc_ringbuf_reset, ringbuf uninitialized\n");
		return -1;
	}
	// wipe the data and index
	memset(buf->d,0,buf->size*sizeof(RINGBUF_TYPE));
	buf->index=0;
	return 0;
}

/**
 * @brief      Puts a new float into the ring buffer and updates the index
 * accordingly.
 *
 * If the buffer was full then the oldest value in the buffer is automatically
 * removed.
 *
 * @param      buf   Pointer to user's buffer
 * @param[in]  val   The value to be inserted
 *
 * @return     Returns 0 on success or -1 on failure.
 */
int rc_ringbuf_insert(rc_ringbuf_t* buf, RINGBUF_TYPE val)
{
	int new_index;
	// sanity checks
	if(unlikely(buf==NULL)){
		fprintf(stderr,"ERROR in rc_ringbuf_insert, received NULL pointer\n");
		return -1;
	}
	if(unlikely(!buf->initialized)){
		fprintf(stderr,"ERROR in rc_ringbuf_insert, ringbuf uninitialized\n");
		return -1;
	}
	// increment index and check for loop-around
	new_index=buf->index+1;
	if(new_index>=buf->size) new_index=0;
	// write out new value
	buf->d[new_index]=val;
	buf->index=new_index;
	return 0;
}

/**
 * @brief      Fetches the float which is 'position' steps behind the last value
 * added to the buffer.
 *
 * If 'position' is given as 0 then the most recent value is returned. The
 * position obviously can't be larger than (buffer size - 1).
 *
 * @param      buf       Pointer to user's buffer
 * @param[in]  position  steps back in the buffer to fetch the value from
 *
 * @return     Returns the requested float. Prints an error message and returns
 * -1 on error.
 */
int rc_ringbuf_get_value(rc_ringbuf_t* buf, int position, RINGBUF_TYPE * value)
{
	int return_index;
	// sanity checks
	if(unlikely(buf==NULL)){
		fprintf(stderr,"ERROR in rc_ringbuf_get_value, received NULL pointer\n");
		return -1.0f;
	}
	if(unlikely(position<0 || position>buf->size-1)){
		fprintf(stderr,"ERROR in rc_ringbuf_get_value, position out of bounds\n");
		return -1.0f;
	}
	if(unlikely(!buf->initialized)){
		fprintf(stderr,"ERROR in rc_ringbuf_get_value, ringbuf uninitialized\n");
		return -1.0f;
	}
	// check for looparound
	return_index=buf->index-position;
	if(return_index<0) return_index+=buf->size;
	*value = buf->d[return_index];
	return 0;
}

/**
 * @brief      Fetches the float which is 'position' steps behind the last value
 * added to the buffer.
 *
 * If 'position' is given as 0 then the most recent value is returned. The
 * position obviously can't be larger than (buffer size - 1).
 *
 * @param      buf       Pointer to user's buffer
 * @param[in]  position  steps back in the buffer to fetch the value from
 *
 * @return     Returns the requested float. Prints an error message and returns
 * -1 on error.
 */
int rc_ringbuf_get_value_ptr(rc_ringbuf_t* buf, int position, RINGBUF_TYPE ** value_ptr)
{
	int return_index;
	// sanity checks
	if(unlikely(buf==NULL)){
		fprintf(stderr,"ERROR in rc_ringbuf_get_value_ptr, received NULL pointer\n");
		return -1.0f;
	}
	if(unlikely(position<0 || position>buf->size-1)){
		fprintf(stderr,"ERROR in rc_ringbuf_get_value_ptr, position out of bounds\n");
		return -1.0f;
	}
	if(unlikely(!buf->initialized)){
		fprintf(stderr,"ERROR in rc_ringbuf_get_value_ptr, ringbuf uninitialized\n");
		return -1.0f;
	}
	// check for looparound
	return_index=buf->index-position;
	if(return_index<0) return_index+=buf->size;
	*value_ptr = &buf->d[return_index];
	return 0;
}




#ifdef __cplusplus
}
#endif

