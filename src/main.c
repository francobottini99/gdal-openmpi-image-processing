#include "main.h"

#ifdef PARALLEL_PROCESSING
    double process_dataset(GDALDatasetH input_dataset, GDALDatasetH output_dataset, const int kern[3][3], int x_size, int y_size)
    {
        double start_time, end_time, elapsed_time;

        start_time = omp_get_wtime();

        strip_list** read_buffer = malloc(sizeof(strip_list*) * 3);
        strip_list** write_buffer = malloc(sizeof(strip_list*) * 3);

        for (int i = 0; i < 3; i++)
        {
            read_buffer[i] = strip_alloc_list();
            write_buffer[i] = strip_alloc_list();
        }

        omp_lock_t dataset_input_mutex;
        omp_lock_t dataset_output_mutex;

        omp_init_lock(&dataset_input_mutex);
        omp_init_lock(&dataset_output_mutex);

        fprintf(stdout, "\nStarting process bands !\n\n");

        #pragma omp parallel
        {
            #pragma omp single
            {
                for (int band_index = 1; band_index < 4; band_index++)
                {
                    #pragma omp task
                    {
                        fprintf(stdout, "\nBand %d READ start !\n", band_index);
                        read_tiff(read_buffer[band_index - 1], input_dataset, &dataset_input_mutex, x_size, y_size, band_index);   
                    }

                    #pragma omp task
                    {
                        fprintf(stdout, "\nBand %d FILTER start !\n", band_index);
                        filter_tiff(read_buffer[band_index - 1], write_buffer[band_index - 1], x_size, y_size, band_index, kern);    
                    }

                    #pragma omp task
                    {
                        fprintf(stdout, "\nBand %d WRITE start !\n", band_index);
                        write_tiff(write_buffer[band_index - 1], output_dataset, &dataset_output_mutex, x_size, y_size, band_index);    
                    }
                }
            }
        }

        omp_destroy_lock(&dataset_input_mutex);
        omp_destroy_lock(&dataset_output_mutex);

        for (int i = 0; i < 3; i++)
        {
            strip_free_list(read_buffer[i]);
            strip_free_list(write_buffer[i]);
        }

        free(read_buffer);
        free(write_buffer);

        end_time = omp_get_wtime();

        elapsed_time = end_time - start_time;

        fprintf(stderr, "\nAll bands process (Execution time: %f seconds) !\n", elapsed_time);
        
        return elapsed_time;
    }
#else
    double process_dataset(GDALDatasetH input_dataset, GDALDatasetH output_dataset, const int kern[3][3], int x_size, int y_size)
    {
        clock_t start_time, end_time;
        double cpu_time_used;

        fprintf(stdout, "\nStarting process bands !\n\n");

        start_time = clock();

        for (int band_index = 1; band_index < 4; band_index++)
        {
            strip_list* read_buffer = strip_alloc_list();
            strip_list* write_buffer = strip_alloc_list();

            fprintf(stdout, "\nBand READ %d start !\n", band_index);
            read_tiff(read_buffer, input_dataset, x_size, y_size, band_index);   
        
            fprintf(stdout, "\nBand FILTER %d start !\n", band_index);
            filter_tiff(read_buffer, write_buffer, x_size, y_size, band_index, kern);    

            strip_free_list(read_buffer);

            fprintf(stdout, "\nBand WRITE %d start !\n", band_index);
            write_tiff(write_buffer, output_dataset, x_size, y_size, band_index);    
        
            strip_free_list(write_buffer);
        }

        end_time = clock();

        cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

        fprintf(stderr, "\nAll bands process (Execution time: %f seconds) !\n", cpu_time_used);

        return cpu_time_used;

    }
#endif

double process_file(const char* input_path, const char* output_path, const int kern[3][3])
{
    GDALDatasetH input_dataset = GDALOpen(input_path, GA_ReadOnly);

    if (input_dataset == NULL) 
    {
        fprintf(stderr, "Failed on open file %s !\n", input_path);
        exit(EXIT_FAILURE);
    }

    int x_size = GDALGetRasterXSize(input_dataset);
    int y_size = GDALGetRasterYSize(input_dataset); 

    GDALDatasetH output_dataset = GDALCreate(GDALGetDriverByName("GTiff"), output_path, x_size, y_size, 3, GDT_Byte, NULL);

    if (output_dataset == NULL)
    {
        fprintf(stderr, "Failed on create output dataset !\n");
        exit(EXIT_FAILURE);
    }

    double time = process_dataset(input_dataset, output_dataset, kern, x_size, y_size);

    GDALClose(input_dataset);
    GDALClose(output_dataset);

    return time;
}

#ifdef TEST
    void testing(const char* input_path, const char* output_path, const int kern[3][3])
    {
        double time[TEST];

        for (int i = 0; i < TEST; i++)
        {
            fprintf(stdout, "\nStarting process %d / %d !\n", i + 1, TEST);

            time[i] = process_file(input_path, output_path, kern);

            fprintf(stdout, "\nEnding process %d / %d !\n", i + 1, TEST);
        }

        fprintf(stdout, "\n\n");

        double sum = 0;

        for (int i = 0; i < TEST; i++)
        {
            fprintf(stdout, "Time %d: %f\n", i + 1, time[i]);
            sum += time[i];
        }

        fprintf(stdout, "\nTotal time: %f\n", sum);
        fprintf(stdout, "\nAverage time: %f\n", sum / TEST);
    }
#endif

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Invalid number of arguments: [input_path] [output_path] !\n");
        return EXIT_FAILURE; 
    }

    GDALAllRegister();

    const int kern[3][3] = 
    {
        { -1, -1, -1 },
        { -1,  8, -1 },
        { -1, -1, -1 }
    };
    
    #ifndef TEST
        fprintf(stdout, "\nStarting process !\n");

        double time = process_file(argv[1], argv[2], kern);

        fprintf(stdout, "\nEnding process !\n");
        fprintf(stdout, "\nTotal time: %f\n", time);
    #else
        fprintf(stdout, "\nStarting test !\n");

        testing(argv[1], argv[2], kern);

        fprintf(stdout, "\nEnding test !\n");
    #endif
    
    return EXIT_SUCCESS;
}

