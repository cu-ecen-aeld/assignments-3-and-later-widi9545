/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif
#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    size_t entry_offset = 0;
    size_t offset_holder = 0;
    size_t entry = buffer ->out_offs;
    struct aesd_buffer_entry* entry_holder;

    if (buffer == NULL) {
        return NULL;
    }

    //check each entry for the offset that matches
    // i < aesd
    for(int i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++){
        //printf("this is the iteration: %d \n", i);
        // check entry
        //set entry back to 0 to start iterating through again / not break circular device
        if(entry == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED){
            entry = 0;
        }
        // address needed otherwise we are assiging wrong type
        entry_holder = &buffer->entry[entry];
        entry_offset = entry_holder -> size;

        if(char_offset < entry_offset + offset_holder){

            *entry_offset_byte_rtn = char_offset - offset_holder;
            return entry_holder;

        }


        offset_holder += entry_offset;
        entry += 1;
        if(entry == buffer -> out_offs){
            break;
        }

    }



    return NULL;
    

}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
    //size_t size_of_buffer = add_entry -> size;


    // grab offset, increment by 1, set check
    uint8_t offset = buffer -> in_offs;
    uint8_t new_location = offset + 1;
    bool status_check = buffer -> full;

    buffer -> entry[offset] = *add_entry;
    // if not full, set new out location
    // bool initializes to 0 in struct
    if(status_check){
        buffer -> out_offs = (new_location); 
    }
    // AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED defined in aesd-circular-buffer
    if(new_location == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED){
        new_location = 0;
    } 
    buffer -> in_offs = (new_location);



    if(buffer -> in_offs == buffer -> out_offs){
        buffer->full = true;
    }



    

}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
