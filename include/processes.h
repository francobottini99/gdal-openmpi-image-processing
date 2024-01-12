#ifndef __PROCESSES_H__
#define __PROCESSES_H__

#include "common.h"
#include "strips.h"

#ifdef PARALLEL_PROCESSING
    /**
     * @brief Write a strip list on a band of TIFF file.
     * 
     * @param buffer The strip list to write.
     * @param dataset The dataset to write to.
     * @param dataset_mutex The mutex to lock the dataset with.
     * @param x_size The width of the strips.
     * @param y_size The number of strips.
     * @param band_index The band index to write to.
     * 
     * @return void.
    */
    void write_tiff(strip_list* buffer, GDALDatasetH dataset, omp_lock_t* dataset_mutex, int x_size, int y_size, int band_index);

    /**
     * @brief Read a strip list from a band of TIFF file.
     * 
     * @param buffer The strip list to read to.
     * @param dataset The dataset to read from.
     * @param dataset_mutex The mutex to lock the dataset with.
     * @param x_size The width of the strips.
     * @param y_size The number of strips.
     * @param band_index The band index to read from.
     * 
     * @return void.
    */
    void read_tiff(strip_list* buffer, GDALDatasetH dataset, omp_lock_t* dataset_mutex, int x_size, int y_size, int band_index);
#else
    /**
     * @brief Write a strip list on a band of TIFF file.
     * 
     * @param buffer The strip list to write.
     * @param dataset The dataset to write to.
     * @param x_size The width of the strips.
     * @param y_size The number of strips.
     * @param band_index The band index to write to.
     * 
     * @return void.
    */
    void write_tiff(strip_list* buffer, GDALDatasetH dataset, int x_size, int y_size, int band_index);

    /**
     * @brief Read a strip list from a band of TIFF file.
     * 
     * @param buffer The strip list to read to.
     * @param dataset The dataset to read from.
     * @param x_size The width of the strips.
     * @param y_size The number of strips.
     * @param band_index The band index to read from.
     * 
     * @return void.
    */
    void read_tiff(strip_list* buffer, GDALDatasetH dataset, int x_size, int y_size, int band_index);
#endif

/**
 * @brief Applies the given kernel to the input strip list and saves it to the output strip list.
 * 
 * @param read_buffer The input strip list.
 * @param write_buffer The output strip list.
 * @param x_size The width of the strips.
 * @param y_size The number of strips.
 * @param band_index The band index to apply the kernel to.
 * @param kern The kernel to be applied.
 * 
 * @return void.
*/
void filter_tiff(strip_list* read_buffer, strip_list* write_buffer, int x_size, int y_size, int band_index, const int kern[3][3]);

#endif // __PROCESSES_H__