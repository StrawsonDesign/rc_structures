/**
 * "fifo_buffer.h"
 *
 * @brief      fifo buffer implementation
 *
 *
 * The user creates their own instance of a buffer and passes a pointer to the
 * these functions to perform normal operations.
 *
 * @author     James Strawson
 * @date       2019
 *
 */


#ifndef FIFOBUF_TYPE
#error "ERROR user must #define FIFOBUF_TYPE before including fifo_buf.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect (!!(x), 0)
#endif

#ifndef likely
#define likely(x)   __builtin_expect (!!(x), 1)
#endif


/**
 * @brief      Struct containing state of a fifobuffer and pointer to
 * dynamically allocated memory.
 */
typedef struct rc_fifobuf_t {
    FIFOBUF_TYPE* d;    ///< pointer to dynamically allocated data
    int size;           ///< number of elements the buffer can hold
    int tail;           ///< index of the next value to be read
    int available;      ///< number of entried waiting to be read
    int initialized;    ///< flag indicating if memory has been allocated for the buffer
} rc_fifobuf_t;


#define RC_FIFOBUF_INITIALIZER {\
    .d = NULL,\
    .size = 0,\
    .tail = 0,\
    .available = 0,\
    .initialized = 0}

/**
 * @brief      Returns an rc_fifobuf_t struct which is completely zero'd out
 * with no memory allocated for it.
 *
 * This is essential for declaring new fifo buffers since structs declared
 * inside of functions are not necessarily zero'd out which can cause the struct
 * to contain problematic contents leading to segfaults. New fifo buffers should
 * be initialized with this before calling rc_fifobuf_alloc.
 *
 * @return     empty and ready-to-allocate rc_fifobuf_t
 */
rc_fifobuf_t rc_fifobuf_empty(void)
{
    rc_fifobuf_t out = RC_FIFOBUF_INITIALIZER;
    return out;
}

/**
 * @brief      Allocates memory for a fifo buffer and initializes an
 * rc_fifobuf_t struct.
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
int rc_fifobuf_alloc(rc_fifobuf_t* buf, int size)
{
    // sanity checks
    if(unlikely(buf==NULL)){
        fprintf(stderr,"ERROR in rc_fifobuf_alloc, received NULL pointer\n");
        return -1;
    }
    if(unlikely(size<2)){
        fprintf(stderr,"ERROR in rc_fifobuf_alloc, size must be >=2\n");
        return -1;
    }
    // if it's already allocated, nothing to do
    if(buf->initialized && buf->size==size && buf->d!=NULL) return 0;
    // make sure it's zero'd out
    buf->size = 0;
    buf->tail = 0;
    buf->available = 0;
    buf->initialized = 0;
    // free memory and allocate fresh
    free(buf->d);
    buf->d = (FIFOBUF_TYPE*)calloc(size,sizeof(FIFOBUF_TYPE));
    if(buf->d==NULL){
        fprintf(stderr,"ERROR in rc_fifobuf_alloc, failed to allocate memory\n");
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
int rc_fifobuf_free(rc_fifobuf_t* buf)
{
    rc_fifobuf_t new = RC_FIFOBUF_INITIALIZER;
    if(unlikely(buf==NULL)){
        fprintf(stderr, "ERROR in rc_fifobuf_free, received NULL pointer\n");
        return -1;
    }
    if(buf->initialized) free(buf->d);
    *buf = new;
    return 0;
}

/**
 * @brief      memsets the buffer to 0 and sets the buffer index
 * back to 0.
 *
 * @param      buf   Pointer to user's buffer
 *
 * @return     Returns 0 on success or -1 on failure.
 */
int rc_fifobuf_reset(rc_fifobuf_t* buf)
{
    // sanity checks
    if(unlikely(buf==NULL)){
        fprintf(stderr, "ERROR in rc_fifobuf_reset, received NULL pointer\n");
        return -1;
    }
    if(unlikely(!buf->initialized)){
        fprintf(stderr,"ERROR rc_fifobuf_reset, fifobuf uninitialized\n");
        return -1;
    }
    // wipe the data and index
    memset(buf->d,0,buf->size*sizeof(FIFOBUF_TYPE));
    buf->tail=0;
    buf->available=0;
    return 0;
}

int rc_fifobuf_available(rc_fifobuf_t* buf)
{
    // sanity checks
    if(unlikely(buf==NULL)){
        fprintf(stderr, "ERROR in rc_fifobuf_available, received NULL pointer\n");
        return -1;
    }
    if(unlikely(!buf->initialized)){
        fprintf(stderr,"ERROR rc_fifobuf_available, fifobuf uninitialized\n");
        return -1;
    }
    return buf->available;
}



/**
 * @brief      Puts a new entry into the fifo buffer and updates the index
 * accordingly.
 *
 *
 * @param      buf   Pointer to user's buffer
 * @param[in]  val   The value to be inserted
 *
 * @return     Returns 0 on success or -1 on failure.
 */
int rc_fifobuf_push(rc_fifobuf_t* buf, FIFOBUF_TYPE val)
{
    int new_index;
    // sanity checks
    if(unlikely(buf==NULL)){
        fprintf(stderr,"ERROR in rc_fifobuf_insert, received NULL pointer\n");
        return -1;
    }
    if(unlikely(!buf->initialized)){
        fprintf(stderr,"ERROR in rc_fifobuf_insert, fifobuf uninitialized\n");
        return -1;
    }

    // check for full. fail silently as the user may run into this as an
    // intentional check for the buffer being full
    if(buf->available == buf->size) return -1;

    // calculate index to put the new value into
    new_index = (buf->tail + buf->available) % buf->size;
    buf->d[new_index]=val;
    // increment available count
    buf->available++;
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
int rc_fifobuf_pop(rc_fifobuf_t* buf, FIFOBUF_TYPE * value)
{
    int return_index;
    // sanity checks
    if(unlikely(buf==NULL)){
        fprintf(stderr,"ERROR in rc_fifobuf_get_value, received NULL pointer\n");
        return -1.0f;
    }
    if(unlikely(!buf->initialized)){
        fprintf(stderr,"ERROR in rc_fifobuf_get_value, fifobuf uninitialized\n");
        return -1.0f;
    }

    // check for empty. fail silently as the user may run into this as an
    // intentional check for the buffer being empty
    if(buf->available == 0) return -1;

    // write out value
    *value = buf->d[buf->tail];

    // update counters
    buf->available--;
    buf->tail = (buf->tail + 1) % buf->size;
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
int rc_fifobuf_pop_ptr(rc_fifobuf_t* buf, FIFOBUF_TYPE ** value_ptr)
{
    int return_index;
    // sanity checks
    if(unlikely(buf==NULL)){
        fprintf(stderr,"ERROR in rc_fifobuf_get_value_ptr, received NULL pointer\n");
        return -1.0f;
    }
    if(unlikely(!buf->initialized)){
        fprintf(stderr,"ERROR in rc_fifobuf_get_value_ptr, fifobuf uninitialized\n");
        return -1.0f;
    }

    // check for empty. fail silently as the user may run into this as an
    // intentional check for the buffer being empty
    if(buf->available == 0) return -1;

    // write out value
    *value_ptr = &buf->d[buf->tail];

    // update counters
    buf->available--;
    buf->tail = (buf->tail + 1) % buf->size;
    return 0;
}




#ifdef __cplusplus
}
#endif

