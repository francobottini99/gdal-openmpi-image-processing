#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <unistd.h>
#include <gdal.h>
#include <cpl_conv.h>

/* Defining this macro the program is compiled with the parallel filtering algorithm.
   Otherwise, the program is compiled with the sequential filtering algorithm. */
#define PARALLEL_PROCESSING

/* Defining this macro the program is compiled with the test mode and define number of test to execute.
   Otherwise, the program is compiled with the normal mode. */
//#define TEST 3

/* Definig this macro the program is compiled with debug mensagges on write file. */
//#define WRITE_PRINTS

/* Definig this macro the program is compiled with debug mensagges on read file. */
//#define READ_PRINTS

/* Definig this macro the program is compiled with debug mensagges on filter. */
//#define FILTER_PRINTS

#ifdef PARALLEL_PROCESSING
    #include <omp.h>
#else
    #include <time.h>
#endif

#endif // __COMMON_H__