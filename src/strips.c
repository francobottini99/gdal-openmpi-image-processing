#include "strips.h"

/* Define the size of last nodes access cache */
#define ACCESS_CACHE_SIZE 32

/* Define struct to generate nodes in the list */
typedef struct node
{
    int index;          // Index of the strip
    int access;         // Number of access counter
    strip* content;     // The strip
    struct node* next;  // Next node in the list

    #ifdef PARALLEL_PROCESSING
        omp_lock_t mutex;   // Mutex to lock the node
    #endif
} node;

/* Define access cache to nodes of lists */
typedef struct access_cache
{
    node* nodes[ACCESS_CACHE_SIZE]; // Array to store the last accessed nodes
    int index;                      // Index of the next node to be replaced
    unsigned long total_access;     // Total number of cache access
    unsigned long misses;           // Number of cache misses

    #ifdef PARALLEL_PROCESSING
        omp_lock_t mutex;           // Mutex to lock the cache
    #endif
} access_cache;

#ifdef PARALLEL_PROCESSING
    /**
     * @brief Acquire a reader lock for the list.
     * 
     * @param list The list to acquire the lock.
     * 
     * @return void.
    */
    void acquire_reader_lock(strip_list* list)
    {
        while (1) 
        {
            omp_set_lock(&list->mutex);

            if (!list->writer_active) 
            {
                list->readers++;

                omp_unset_lock(&list->mutex);

                break;
            }

            omp_unset_lock(&list->mutex);
            #pragma omp flush
        }
    }

    /**
     * @brief Release a reader lock for the list.
     * 
     * @param list The list to release the lock.
     * 
     * @return void.
    */
    void release_reader_lock(strip_list* list) 
    {
        omp_set_lock(&list->mutex);
        
        list->readers--;

        omp_unset_lock(&list->mutex);
    }

    /**
     * @brief Acquire a writer lock for the list.
     * 
     * @param list The list to acquire the lock.
     * 
     * @return void.
    */
    void acquire_writer_lock(strip_list* list) 
    {
        while (1) 
        {
            omp_set_lock(&list->mutex);

            if (list->readers == 0 && !list->writer_active) 
            {
                list->writer_active = 1;

                omp_unset_lock(&list->mutex);
                
                break;
            }

            omp_unset_lock(&list->mutex);
            #pragma omp flush
        }
    }

    /**
     * @brief Release a writer lock for the list.
     * 
     * @param list The list to release the lock.
     * 
     * @return void.
    */
    void release_writer_lock(strip_list* list) 
    {
        omp_set_lock(&list->mutex);

        list->writer_active = 0;

        omp_unset_lock(&list->mutex);
    }
#endif

node* get_node_cache(access_cache* cache, int index)
{
    node* n = NULL;

    cache->total_access++;

    for(int i = 0; i < ACCESS_CACHE_SIZE; i++)
    {
        if (cache->nodes[i] && cache->nodes[i]->index == index) 
        {
            n = cache->nodes[i];
            break;
        }
    }

    if(!n)
        cache->misses++;

    return n;
}

/**
 * @brief Find the parent node of a node in the cache.
 * 
 * @param cache The cache to search.
 * @param n The node to find the parent.
 * 
 * @return The parent node of the node or NULL if not found.
*/
node* get_node_cache_parent(access_cache* cache, node* n)
{
    node* parent = NULL;

    cache->total_access++;

    for(int i = 0; i < ACCESS_CACHE_SIZE; i++)
    {
        if (cache->nodes[i] && cache->nodes[i]->next == n) 
        {
            parent = cache->nodes[i];
            break;
        }
    }

    if(!parent)
        cache->misses++;

    return parent;
}

/**
 * @brief Add a node to the cache.
 * 
 * @param cache The cache to add the node.
 * @param n The node to add.
 * 
 * @return void.
*/
void add_node_cache(access_cache* cache, node* n)
{
    int index = cache->index;

    for(int i = 0; i < ACCESS_CACHE_SIZE; i++)
    {
        if (!cache->nodes[i]) 
        {
            index = i;
            break;
        }
        else if(cache->nodes[i] == n)
            return;
    }

    cache->nodes[index] = n;
    
    if(index == cache->index)
        cache->index = (cache->index + 1) % ACCESS_CACHE_SIZE;
}

/**
 * @brief Remove a node from the cache.
 * 
 * @param cache The cache to remove the node.
 * @param n The node to remove.
 * 
 * @return void.
*/
void remove_node_cache(access_cache* cache, node* n)
{
    for(int i = 0; i < ACCESS_CACHE_SIZE; i++)
    {
        if (cache->nodes[i] == n) 
        {
            cache->nodes[i] = NULL;
            break;
        }
    }
}

/**
 * @brief Find a node in the list.
 * 
 * @param list The list to search.
 * @param index The index of the node to find.
 * 
 * @return The node or NULL if not found.
*/
node* get_node(strip_list* list, int index)
{
    #ifdef PARALLEL_PROCESSING
        omp_set_lock(&list->cache->mutex);
    #endif

    node* it = get_node_cache(list->cache, index);

    #ifdef PARALLEL_PROCESSING
        omp_unset_lock(&list->cache->mutex);
    #endif
    
    if(it)
        return it;

    for (it = list->first_node; it; it = it->next)
        if (it->index == index) 
            break;

    if(it)
    {
        #ifdef PARALLEL_PROCESSING
            omp_set_lock(&list->cache->mutex);
        #endif

        add_node_cache(list->cache, it);

        #ifdef PARALLEL_PROCESSING
            omp_unset_lock(&list->cache->mutex);
        #endif

    }

    return it;
}

/**
 * @brief Find the parent node of a node in the list.
 * 
 * @param list The list to search.
 * @param n The node to find the parent.
 * 
 * @return The parent node of the node or NULL if not found.
*/
node* get_node_parent(strip_list* list, node* n)
{
    node *parent = list->first_node;

    if(!parent || n == parent)
        return NULL;

    #ifdef PARALLEL_PROCESSING
        omp_set_lock(&list->cache->mutex);
    #endif

    parent = get_node_cache_parent(list->cache, n);

    #ifdef PARALLEL_PROCESSING
        omp_unset_lock(&list->cache->mutex);
    #endif

    if(parent)
        return parent;

    while (parent->next != n)
        parent = parent->next;

    #ifdef PARALLEL_PROCESSING
        omp_set_lock(&list->cache->mutex);
    #endif

    add_node_cache(list->cache, parent);

    #ifdef PARALLEL_PROCESSING
        omp_unset_lock(&list->cache->mutex);
    #endif

    return parent;
}

/**
 * @brief Remove a node from the list.
 * 
 * @param list The list to remove the node.
 * @param n The node to remove.
 * 
 * @return void.
*/
void remove_node(strip_list* list, node* n)
{
    node *parent = get_node_parent(list, n);

    if(!parent)
        list->first_node = n->next;
    else if (!n->next)
        parent->next = NULL;  
    else 
        parent->next = n->next;

    if (n == list->last_node)
    {
        if(parent)
            list->last_node = parent;
        else
            list->last_node = list->first_node;
    }

    #ifdef PARALLEL_PROCESSING
        omp_set_lock(&list->cache->mutex);
    #endif

    remove_node_cache(list->cache, n);

    #ifdef PARALLEL_PROCESSING
        omp_unset_lock(&list->cache->mutex);
    #endif
    
    list->size--;

    #ifdef PARALLEL_PROCESSING
        omp_destroy_lock(&n->mutex);
    #endif

    CPLFree(*n->content);

    free(n->content);
    free(n);
}

/**
 * @brief Remove all nodes from the list.
 * 
 * @param list The list to remove all nodes.
 * 
 * @return void.
*/
void remove_all_node(strip_list* list)
{
    node* it = list->first_node;

    while (it)
    {
        node* aux = it;

        it = it->next;

        #ifdef PARALLEL_PROCESSING
            omp_destroy_lock(&aux->mutex);
        #endif

        CPLFree(*aux->content);

        free(aux->content);
        free(aux);
    }
}

strip strip_alloc(int size)
{
    return (strip) CPLMalloc((size_t)((int)sizeof(float) * size));
}

strip_list* strip_alloc_list(void)
{
    strip_list* list = (strip_list*) malloc(sizeof(strip_list));

    list->first_node = NULL;
    list->size = 0;
    list->max_size = 0;

    list-> cache = (access_cache*) malloc(sizeof(access_cache));

    list->cache->index = 0;
    list->cache->misses = 0;
    list->cache->total_access = 0;

    for (int i = 0; i < ACCESS_CACHE_SIZE; i++)
        list->cache->nodes[i] = NULL;

    #ifdef PARALLEL_PROCESSING
        omp_init_lock(&list->cache->mutex);

        list->readers = 0;
        list->writer_active = 0;
        omp_init_lock(&list->mutex);
    #endif

    return list;
}

void strip_free_list(strip_list* list)
{
    #ifdef PARALLEL_PROCESSING
        acquire_writer_lock(list);
    #endif

    remove_all_node(list);

    #ifdef PARALLEL_PROCESSING
        omp_destroy_lock(&list->mutex);
    #endif

    free(list->cache);
    free(list);
}

void strip_list_add(strip_list* list, int index, strip content)
{
    node* new_node = (node*) malloc(sizeof(node));

    new_node->index = index;
    new_node->access = 0;

    new_node->content = (strip*) malloc(sizeof(strip));
    *new_node->content = content;

    new_node->next = NULL;
    
    #ifdef PARALLEL_PROCESSING
        omp_init_lock(&new_node->mutex);

        acquire_writer_lock(list);
    #endif

    if(!list->first_node)
    {
        list->first_node = new_node;
        list->last_node = new_node;
    }
    else
    {
        list->last_node->next = new_node;
        list->last_node = new_node;
    }

    list->size++;

    if(list->size > list->max_size)
        list->max_size = list->size;
    
    #ifdef PARALLEL_PROCESSING
        release_writer_lock(list);
    #endif
}

void strip_list_remove_by_index(strip_list* list, int index)
{
    #ifdef PARALLEL_PROCESSING
        acquire_writer_lock(list);
    #endif

    if (list->first_node)
    {
        node *n = get_node(list, index);

        if (n)
            remove_node(list, n);
    }

    #ifdef PARALLEL_PROCESSING
        release_writer_lock(list);
    #endif
}

strip strip_list_get(strip_list* list, int index)
{
    #ifdef PARALLEL_PROCESSING
        acquire_reader_lock(list);
    #endif

    strip content = NULL;

    node* n = get_node(list, index);

    if(n)
    {
        #ifdef PARALLEL_PROCESSING
            omp_set_lock(&n->mutex);
        #endif

        content = *n->content;
        n->access++;

        #ifdef PARALLEL_PROCESSING
            omp_unset_lock(&n->mutex);
        #endif
    }

    #ifdef PARALLEL_PROCESSING
        release_reader_lock(list);
    #endif

    return content;
}

int strip_list_get_size(strip_list* list)
{
    #ifdef PARALLEL_PROCESSING
        acquire_reader_lock(list);
    #endif

    int size = list->size;

    #ifdef PARALLEL_PROCESSING
        release_reader_lock(list);
    #endif

    return size;
}

int strip_list_get_max_size(strip_list* list)
{
    #ifdef PARALLEL_PROCESSING
        acquire_reader_lock(list);
    #endif

    int max_size = list->max_size;

    #ifdef PARALLEL_PROCESSING
        release_reader_lock(list);
    #endif

    return max_size;
}

int strip_list_get_access(strip_list* list, int index)
{
    #ifdef PARALLEL_PROCESSING
        acquire_reader_lock(list);
    #endif

    int access = -1;

    node *n = get_node(list, index);
    
    if (n)
    {
        #ifdef PARALLEL_PROCESSING
            omp_set_lock(&n->mutex);
        #endif
        
        access = n->access;
        
        #ifdef PARALLEL_PROCESSING
            omp_unset_lock(&n->mutex);
        #endif
    }

    #ifdef PARALLEL_PROCESSING
        release_reader_lock(list);
    #endif
    
    return access;
}