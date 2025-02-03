#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

// Функция для проверки ошибок OpenCL
void checkError(cl_int err, const char *operation) {
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error during operation '%s': %d\n", operation, err);
        exit(1);
    }
}

int main() {
    // Инициализация данных
    const int arraySize = 1024;
    float A[arraySize], B[arraySize], C[arraySize];
    for (int i = 0; i < arraySize; i++) {
        A[i] = i;
        B[i] = i * 2;
    }

    // Получение платформы
    cl_platform_id platform;
    cl_uint numPlatforms;
    cl_int err = clGetPlatformIDs(1, &platform, &numPlatforms);
    checkError(err, "Getting platform IDs");

    // Получение устройства
    cl_device_id device;
    cl_uint numDevices;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &numDevices);
    checkError(err, "Getting device IDs");

    // Создание контекста
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    checkError(err, "Creating context");

    // Создание очереди команд
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    checkError(err, "Creating command queue");

    // Создание буферов
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY, arraySize * sizeof(float), NULL, &err);
    checkError(err, "Creating buffer A");
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY, arraySize * sizeof(float), NULL, &err);
    checkError(err, "Creating buffer B");
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, arraySize * sizeof(float), NULL, &err);
    checkError(err, "Creating buffer C");

    // Копирование данных на устройство
    err = clEnqueueWriteBuffer(queue, bufferA, CL_TRUE, 0, arraySize * sizeof(float), A, 0, NULL, NULL);
    checkError(err, "Copying A to device");
    err = clEnqueueWriteBuffer(queue, bufferB, CL_TRUE, 0, arraySize * sizeof(float), B, 0, NULL, NULL);
    checkError(err, "Copying B to device");

    // Чтение ядра из файла
    FILE *file = fopen("vector_add.cl", "r");
    if (!file) {
        fprintf(stderr, "Failed to open kernel file\n");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *source = (char *)malloc(size + 1);
    fread(source, 1, size, file);
    fclose(file);
    source[size] = '\0';

    // Создание программы
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source, NULL, &err);
    checkError(err, "Creating program");

    // Компиляция программы
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        fprintf(stderr, "Build log:\n%s\n", buffer);
        exit(1);
    }

    // Создание ядра
    cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
    checkError(err, "Creating kernel");

    // Установка аргументов ядра
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    checkError(err, "Setting kernel argument 0");
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    checkError(err, "Setting kernel argument 1");
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
    checkError(err, "Setting kernel argument 2");

    // Запуск ядра
    size_t globalWorkSize = arraySize;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);
    checkError(err, "Enqueueing kernel");

    // Чтение результата с устройства
    err = clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, arraySize * sizeof(float), C, 0, NULL, NULL);
    checkError(err, "Reading C from device");

    // Проверка результата
    for (int i = 0; i < arraySize; i++) {
        if (C[i] != A[i] + B[i]) {
            fprintf(stderr, "Error: C[%d] = %f, expected %f\n", i, C[i], A[i] + B[i]);
            exit(1);
        }
    }

    printf("Success!\n");

    // Освобождение ресурсов
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
