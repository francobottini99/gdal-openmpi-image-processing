#ifndef __MAIN_H__
#define __MAIN_H__

#include "common.h"
#include "processes.h"
#include "strips.h"

/**
 * @brief applies the given kernel to the input dataset and saves it to the output dataset.
 * 
 * @param input_dataset the input dataset.
 * @param output_dataset the output dataset.
 * @param kern the kernel to be applied.
 * @param x_size the width of the dataset.
 * @param y_size the height of the dataset.
 * 
 * @return the time taken to process the dataset.
*/
double process_dataset(GDALDatasetH input_dataset, GDALDatasetH output_dataset, const int kern[3][3], int x_size, int y_size);

/**
 * @brief applies the given kernel to the input file and saves it to the output file.
 * 
 * @param input_path the input file path.
 * @param output_path the output file path.
 * @param kern the kernel to be applied.
 * 
 * @return the time taken to process the file.
*/
double process_file(const char* input_path, const char* output_path, const int kern[3][3]);

#ifdef TEST
    /**
     * @brief tests the performance of the filtering algorithm by applying the given kernel N (define by macro TEST) times on the given input file and saving it to the given output file.
     * 
     * @param input_path the input file path.
     * @param output_path the output file path.
     * @param kern the kernel to be applied.
     * 
     * @return void.
    */
    void testing(const char* input_path, const char* output_path, const int kern[3][3]);
#endif

#endif // __MAIN_H__