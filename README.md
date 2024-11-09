# Parallelization with OpenMPI

This program processes a `GeoTIFF` image file and applies a filter to each band of the image. The filter used is a linear kernel that highlights the edges of the image. The **GDAL** library is used for reading and writing raster image files, and the **OpenMP** library is used to parallelize the processing.

> [!IMPORTANT]
> You can find the `GeoTIFF` images used for testing at the following [link](https://drive.google.com/drive/folders/1em4_plY-dYmwc4ENqZqVOczFuFjKcWNJ?usp=drive_link).

### Authors:
- **Bottini, Franco Nicolas**

### How to compile?

```bash
$ git clone https://github.com/francobottini99/PARALLELPROGRAM-2023.git
$ cd PARALLELPROGRAM-2023
$ cmake .
$ make
```

The program is designed to support both serial and parallel processing, and it can be compiled in either mode using a conditional compilation directive. This directive is **PARALLEL_PROCESSING**, and it can be set in the `commun.h` file. There are also other directives in this file that allow modifying other aspects of the program’s compilation.

The output will be an executable located in the `/bin` folder: `lab4`.

> [!NOTE]
> To compile the project, it is necessary to have the **GDAL** library installed on the system.

### How to run?

Once the program is compiled, it can be run as follows:

```bash
$ ./bin/lab4 <input_file> <output_file>
```

The main function takes two command-line arguments: the input file path for the GeoTIFF image and the output file path for the processed image.

### How it works?

As mentioned at the beginning, the program is an image processor that applies a convolutional filter to a TIFF image file. The filter used is called the *edge filter*, and it highlights the edges of an image. The program takes as arguments the path to the input file (original TIFF image) and the path where the output file (filtered TIFF image) will be generated. From this, two *datasets* are created, one for the input file and one for the output file. With this data, depending on the compilation mode, the processing is either serial or parallel. The processing is divided into three main tasks: reading the image, filtering the image, and writing the image. Each of these tasks is executed for each of the image’s bands (red, green, and blue). In serial processing, the tasks are executed sequentially, while in parallel processing, they are executed concurrently. Once the processing is completed, memory is freed, and the datasets are closed. This results in the output file with the filtered image, and the program execution finishes.

### Performance Testing

For performance testing, a machine with the following specifications was used:

- Processor: 12th Gen Intel(R) Core(TM) i7-12700H
- Memory: 16 GB
- GPU: NVIDIA GeForce RTX 3050
- Operating System: Pop!_OS 22.04 LTS
- Compiler: gcc 11.3.0
- GDAL: 3.7.0

A 10980x10980 pixel image, with a size of 345 MB, was used. Thirty executions of each processing mode were performed on the same image, and the average execution time was calculated. The results were as follows:

| N°  | Serial (s) | Parallel (s) | N°  | Serial (s) | Parallel (s) |
| --- | ---------- | ------------ | --- | ---------- | ------------ |
| 1   | 9.56       | 2.10         | 16  | 15.77      | 1.46         |
| 2   | 15.31      | 1.87         | 17  | 15.55      | 1.61         |
| 3   | 15.32      | 2.14         | 18  | 9.67       | 1.86         |
| 4   | 9.04       | 1.56         | 19  | 9.16       | 1.39         |
| 5   | 8.68       | 1.61         | 20  | 9.40       | 1.91         |
| 6   | 9.13       | 1.90         | 21  | 15.21      | 1.38         |
| 7   | 9.35       | 1.59         | 22  | 8.93       | 2.24         |
| 8   | 15.37      | 1.66         | 23  | 15.60      | 1.85         |
| 9   | 15.22      | 1.87         | 24  | 9.13       | 1.83         |
| 10  | 15.52      | 1.68         | 25  | 15.35      | 1.71         |
| 11  | 15.61      | 1.42         | 26  | 15.63      | 1.78         |
| 12  | 15.58      | 1.94         | 27  | 9.33       | 1.91         |
| 13  | 15.75      | 1.79         | 28  | 15.78      | 1.85         |
| 14  | 15.42      | 1.51         | 29  | 15.54      | 1.92         |
| 15  | 15.69      | 2.22         | 30  | 9.82       | 1.65         |

|     | Serial (s) | Parallel (s) |     | Serial (s) | Parallel (s) |
| --- | ---------- | ------------ | --- | ---------- | ------------ |
| Average | 13.02      | 1.77         | Total | 390.61     | 53.21        |

### Conclusions

From the data obtained, we can conclude that on the hardware used for the tests, the parallel execution algorithm is, on average, **634%** faster than the serial version.

<p align="center">
  <img src="imgs/Grafico_1.png" alt="Grafico_1">
</p>

<p align="center">
  <img src="imgs/Grafico_2.png" alt="Grafico_2">
</p>
