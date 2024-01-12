#ifndef __STRIP_H__
#define __STRIP_H__

#include "common.h"

/* One strip is a 1D array of floats. */
typedef float* strip;

/* Define struct to generate strips lists */
typedef struct strip_list
{   
    int size;                   // Number of strips in the list
    int max_size;               // maximum size reached by the list
    struct access_cache* cache; // Cache to store the last accessed strips
    struct node* first_node;    // First node of the list
    struct node* last_node;     // Last node of the list

    #ifdef PARALLEL_PROCESSING
        omp_lock_t mutex;  // Mutex to lock the list
        int readers;       // Number of active readers 
        int writer_active; // Is a writer active ?
    #endif
} strip_list;

/**
 * @brief Allocate memory to strip.
 * 
 * @param size The size of the strip.
 * 
 * @return strip The allocated memory.
*/
strip strip_alloc(int size);

/**
 * @brief Allocate memory to strip list.
 * 
 * @return strip_list The allocated memory.
*/
strip_list* strip_alloc_list(void);

/**
 * @brief Free memory of strip list.
 * 
 * @param list The strip list to free.
 * 
 * @return void.
*/
void strip_free_list(strip_list* list);

/**
 * @brief Add a strip to the end of a strip list.
 * 
 * @param list The strip list to add to.
 * @param index The index of the strip to add.
 * @param content The strip to add.
 * 
 * @return void.
*/
void strip_list_add(strip_list* list, int index, strip content);

/**
 * @brief Remove a strip from a strip list by index.
 * 
 * @param list The strip list to remove from.
 * @param index The index of the strip to remove.
 * 
 * @return void.
*/
void strip_list_remove_by_index(strip_list* list, int index);

/**
 * @brief Get strip from a strip list by index.
 * 
 * @param list The strip list to get from.
 * @param index The index of the strip to get.
 * 
 * @return strip The strip or NULL if not exist the index in the list.
*/
strip strip_list_get(strip_list* list, int index);

/**
 * @brief Get the size of a strip list.
 * 
 * @param list The strip list to get the size of.
 * 
 * @return int The size of the strip list.
*/
int strip_list_get_size(strip_list* list);

/**
 * @brief Get the maximum size reached by a strip list.
 * 
 * @param list The strip list to get the maximum size of.
 * 
 * @return int The maximum size reached by the strip list.
*/
int strip_list_get_max_size(strip_list* list);

/**
 * @brief Get the number of access to strip in a strip list.
 * 
 * @param list The strip list to get the access of.
 * @param index The index of the strip to get the access of.
 * 
 * @return int The number of access to strip in a strip list.
*/
int strip_list_get_access(strip_list* list, int index);

#endif // __STRIP_H__