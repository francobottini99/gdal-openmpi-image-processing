#include "processes.h"

#ifdef PARALLEL_PROCESSING
    /**
     * @brief applies the kernel to a given strip.
     * 
     * @param prev_strip The previous strip.
     * @param curr_strip The current strip on aplly kern.
     * @param next_strip The next strip.
     * @param output_strip The output strip to save result.
     * @param lineal_kern The kernel to apply.
     * @param strip_width The width of the strip.
     * 
     * @return void.
    */
    void apply_kern(float* prev_strip, float* curr_strip, float* next_strip, float* output_strip, const float kern[9], int strip_width)
    {
        int prev_col_index;
        int curr_col_index;
        int next_col_index;

        #pragma omp taskloop simd private(curr_col_index, prev_col_index, next_col_index) shared(output_strip, prev_strip, curr_strip, next_strip, kern)
        for (int x = 0; x < strip_width; x++)
        {
            prev_col_index = (x - 1 < 0) ? x : (x - 1);
            curr_col_index = x;
            next_col_index = (x + 1 == strip_width) ? x : (x + 1);

            output_strip[curr_col_index] = kern[0] * prev_strip[prev_col_index] +
                                           kern[1] * curr_strip[prev_col_index] +
                                           kern[2] * next_strip[prev_col_index] +
                                           kern[3] * prev_strip[curr_col_index] +
                                           kern[4] * curr_strip[curr_col_index] +
                                           kern[5] * next_strip[curr_col_index] +
                                           kern[6] * prev_strip[next_col_index] +
                                           kern[7] * curr_strip[next_col_index] +
                                           kern[8] * next_strip[next_col_index];
        }
    }
#else
    /**
     * @brief applies the kernel to a given strip.
     * 
     * @param prev_strip The previous strip.
     * @param curr_strip The current strip on aplly kern.
     * @param next_strip The next strip.
     * @param output_strip The output strip to save result.
     * @param lineal_kern The kernel to apply.
     * @param strip_width The width of the strip.
     * 
     * @return void.
    */
    void apply_kern(float* prev_strip, float* curr_strip, float* next_strip, float* output_strip, const float kern[9], int strip_width)
    {
        int prev_col_index = 0;
        int curr_col_index;
        int next_col_index = 1;

        for (int x = 0; x < strip_width; x++)
        {
            curr_col_index = x;

            output_strip[curr_col_index] = kern[0] * prev_strip[prev_col_index] +
                                           kern[1] * curr_strip[prev_col_index] +
                                           kern[2] * next_strip[prev_col_index] +
                                           kern[3] * prev_strip[curr_col_index] +
                                           kern[4] * curr_strip[curr_col_index] +
                                           kern[5] * next_strip[curr_col_index] +
                                           kern[6] * prev_strip[next_col_index] +
                                           kern[7] * curr_strip[next_col_index] +
                                           kern[8] * next_strip[next_col_index];

            prev_col_index = curr_col_index;
            next_col_index = (curr_col_index + 2 < strip_width) ? curr_col_index + 1 : curr_col_index;
        }
    }
#endif

#ifdef PARALLEL_PROCESSING
    void read_tiff(strip_list* buffer, GDALDatasetH dataset, omp_lock_t* dataset_mutex, int x_size, int y_size, int band_index)
    {
        int count = 0;
        strip input_strip;

        GDALRasterBandH band = GDALGetRasterBand(dataset, band_index);

        if(band == NULL)
        {
            fprintf(stderr, "Failed on get band %d !\n", band_index);
            return;
        }

        #pragma omp taskloop grainsize(1) private(input_strip) shared(buffer, dataset_mutex, band_index, y_size, x_size, count)
        for(int i = 0; i < y_size; i++)
        {
            input_strip = strip_alloc(x_size);

            #pragma omp atomic
            count++;

            omp_set_lock(dataset_mutex);

            if (GDALRasterIO(band, GF_Read, 0, i, x_size, 1, input_strip, x_size, 1, GDT_Float32, 0, 0) != CE_None)
                fprintf(stderr, "Thread %d -> Failed read band %d line %d (count: %d) !\n", omp_get_thread_num(), band_index, i, count);
            #ifdef READ_PRINTS
            else
                fprintf(stdout, "Thread %d -> Read band %d line %d (count: %d) !\n", omp_get_thread_num(), band_index, i, count);
            #endif

            omp_unset_lock(dataset_mutex);

            strip_list_add(buffer, i, input_strip);
        }

        fprintf(stdout, "\nBand %d READ end !\n", band_index);
    }
#else
    void read_tiff(strip_list* buffer, GDALDatasetH dataset, int x_size, int y_size, int band_index)
    {
        int count = 0;
        strip input_strip;
    
        GDALRasterBandH band = GDALGetRasterBand(dataset, band_index);
    
        if(band == NULL)
        {
            fprintf(stderr, "Failed on get band %d !\n", band_index);
            return;
        }
        
        for(int i = 0; i < y_size; i++)
        {
            input_strip = strip_alloc(x_size);
    
            count++;
    
            if (GDALRasterIO(band, GF_Read, 0, i, x_size, 1, input_strip, x_size, 1, GDT_Float32, 0, 0) != CE_None)
                fprintf(stderr, "Failed read band %d line %d (count: %d) !\n", band_index, i, count);
            #ifdef READ_PRINTS
            else
                fprintf(stdout, "Read band %d line %d (count: %d) !\n", band_index, i, count);
            #endif

            strip_list_add(buffer, i, input_strip);
        }
    
        fprintf(stdout, "\nBand %d READ end !\n", band_index);
    }
#endif

#ifdef PARALLEL_PROCESSING
    void filter_tiff(strip_list* read_buffer, strip_list* write_buffer, int x_size, int y_size, int band_index, const int kern[3][3])
    {
        const float lineal_kern[9] =
        {
            (float)kern[0][0], (float)kern[1][0], (float)kern[2][0],
            (float)kern[0][1], (float)kern[1][1], (float)kern[2][1],
            (float)kern[0][2], (float)kern[1][2], (float)kern[2][2]
        };

        int prev_strip_index;
        int curr_strip_index;
        int next_strip_index;

        strip prev_strip = NULL;
        strip curr_strip = NULL;
        strip next_strip = NULL;

        strip output_strip;

        int count = 0;

        #pragma omp taskloop grainsize(1) private(prev_strip_index, curr_strip_index, next_strip_index, prev_strip, curr_strip, next_strip, output_strip) shared(read_buffer, write_buffer, x_size, y_size, lineal_kern, count)
        for(int i = 0; i < y_size; i++)
        {
            prev_strip_index = (i - 1 < 0) ? 0 : (i - 1);
            curr_strip_index = i;
            next_strip_index = (i + 1 == y_size) ? i : (i + 1);

            while (!(curr_strip = strip_list_get(read_buffer, curr_strip_index)));
            while (!(prev_strip = strip_list_get(read_buffer, prev_strip_index)));
            while (!(next_strip = strip_list_get(read_buffer, next_strip_index)));

            output_strip = strip_alloc(x_size);

            apply_kern(prev_strip, curr_strip, next_strip, output_strip, lineal_kern, x_size);

            strip_list_add(write_buffer, curr_strip_index, output_strip);

            if(strip_list_get_access(read_buffer, curr_strip_index) >= 3)
                strip_list_remove_by_index(read_buffer, curr_strip_index);

            if(strip_list_get_access(read_buffer, prev_strip_index) >= 3)
                strip_list_remove_by_index(read_buffer, prev_strip_index);

            if(strip_list_get_access(read_buffer, next_strip_index) >= 3)
                strip_list_remove_by_index(read_buffer, next_strip_index);

            #pragma omp atomic
            count++;

            #ifdef FILTER_PRINTS
            fprintf(stdout, "Thread %d -> Process band %d line %d (count: %d) !\n", omp_get_thread_num(), band_index, curr_strip_index, count);
            #endif
        }

        fprintf(stdout, "\nBand %d FILTER end !\n", band_index);
    }
#else
    void filter_tiff(strip_list* read_buffer, strip_list* write_buffer, int x_size, int y_size, int band_index, const int kern[3][3])
    {   
        const float lineal_kern[9] =
        {
            (float)kern[0][0], (float)kern[1][0], (float)kern[2][0],
            (float)kern[0][1], (float)kern[1][1], (float)kern[2][1],
            (float)kern[0][2], (float)kern[1][2], (float)kern[2][2]
        };

        int prev_strip_index = 0;
        int curr_strip_index;
        int next_strip_index = 1;
    
        strip prev_strip = NULL;
        strip curr_strip = NULL;
        strip next_strip = NULL;
    
        strip output_strip;

        int count = 0;
    
        for(int i = 0; i < y_size; i++)
        {
            curr_strip_index = i;
    
            while (!(curr_strip = strip_list_get(read_buffer, curr_strip_index)));
            while (!(prev_strip = strip_list_get(read_buffer, prev_strip_index)));
            while (!(next_strip = strip_list_get(read_buffer, next_strip_index)));
    
            output_strip = strip_alloc(x_size);
            
            apply_kern(prev_strip, curr_strip, next_strip, output_strip, lineal_kern, x_size);
    
            strip_list_add(write_buffer, curr_strip_index, output_strip);
    
            count++;

            #ifdef FILTER_PRINTS
            fprintf(stdout, "Process band %d line %d (count: %d) !\n", band_index, curr_strip_index, count);
            #endif

            prev_strip_index = curr_strip_index;
            next_strip_index = (curr_strip_index + 2 < y_size) ? curr_strip_index + 1 : curr_strip_index;
        }
    
        fprintf(stdout, "\nBand %d FILTER end !\n", band_index);
    }
#endif

#ifdef PARALLEL_PROCESSING
    void write_tiff(strip_list* buffer, GDALDatasetH dataset, omp_lock_t* dataset_mutex, int x_size, int y_size, int band_index)
    {
        int count = 0;
        strip current = NULL;

        GDALRasterBandH band = GDALGetRasterBand(dataset, band_index);

        if (band == NULL)
        {
            fprintf(stderr, "Failed on get band %d !\n", band_index);
            return;
        }  

        #pragma omp taskloop grainsize(1) private(current) shared(buffer, dataset_mutex, band_index, y_size, x_size, count)
        for(int i = 0; i < y_size; i++) 
        {
            while(!(current = strip_list_get(buffer, i)));

            #pragma omp atomic
            count++;

            omp_set_lock(dataset_mutex);

            if (GDALRasterIO(band, GF_Write, 0, i, x_size, 1, current, x_size, 1, GDT_Float32, 0, 0) != CE_None)
                fprintf(stderr, "Thread %d -> Failed write band %d line %d (count: %d) !\n", omp_get_thread_num(), band_index, i, count);
            #ifdef WRITE_PRINTS
            else
                fprintf(stdout, "Thread %d -> Write band %d line %d (count: %d) !\n", omp_get_thread_num(), band_index, i, count);
            #endif

            omp_unset_lock(dataset_mutex);

            strip_list_remove_by_index(buffer, i);
        }

        fprintf(stdout, "\nBand %d WRITE end !\n", band_index);
    }
#else
    void write_tiff(strip_list* buffer, GDALDatasetH dataset, int x_size, int y_size, int band_index)
    {
        int count = 0;
        strip current = NULL;

        GDALRasterBandH band = GDALGetRasterBand(dataset, band_index);

        if (band == NULL)
        {
            fprintf(stderr, "Failed on get band %d !\n", band_index);
            return;
        }  

        for(int i = 0; i < y_size; i++) 
        {
            while(!(current = strip_list_get(buffer, i)));

            count++;

            if (GDALRasterIO(band, GF_Write, 0, i, x_size, 1, current, x_size, 1, GDT_Float32, 0, 0) != CE_None)
                fprintf(stderr, "Failed write band %d line %d (count: %d) !\n", band_index, i, count);
            #ifdef WRITE_PRINTS
            else
                fprintf(stdout, "Write band %d line %d (count: %d) !\n", band_index, i, count);
            #endif
        }

        fprintf(stdout, "\nBand %d WRITE end !\n", band_index);
    }
#endif