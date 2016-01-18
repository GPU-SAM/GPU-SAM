#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define widthA 2048
#define heightA 2048

#define widthB widthA
#define heightB 2048

#define widthC widthA
#define heightC heightB

#include <CL/opencl.h>
#define MAX_SOURCE_SIZE (0x100000)

int main()
{
  float * A = (float *)malloc(sizeof(float)*widthA*heightA);
  float * B = (float *)malloc(sizeof(float)*widthB*heightB);
  float * C = (float *)malloc(sizeof(float)*widthC*heightC);
  float * Res = (float *)malloc(sizeof(float)*widthC*heightC);
  float * D= (float *)malloc(sizeof(float)*widthC*heightC);

  FILE * fp1 = fopen("matAdata.txt", "w");
  if (!fp1) {
    fprintf(stderr, "Failed to open matAdata.\n");
    exit(1);
   }
	int i,j,k;
  for(i = 0;i < widthA; i++)
  {
		for(j=0;j	< heightA; j++)	{
			float p=(rand()%100)/7.0;
			*(A+i*heightA+j)=rand()%100 + p;
			fprintf(fp1, "%f ",*(A+i*heightA+j));
		}
		fprintf(fp1, "\n");
  }
  fclose(fp1);

  fp1 = fopen("matBdata.txt", "w");
  if (!fp1) {
   	fprintf(stderr, "Failed to open matAdata.\n");
  	exit(1);
  }

	for(i = 0;i < widthB; i++)
	{
		for(j=0; j < heightB; j++){
			float p=(rand()%100)/7.0;
			*((B+i*heightB+j))=rand()%100 + p;
			fprintf(fp1, "%f ",*(B+i*heightA+j));
		}
		fprintf(fp1, "\n");
	}
	fclose(fp1);

	struct timeval time1;
	struct timeval time2;
	struct timeval time3;
	struct timeval time4;
	struct timeval time5;
	struct timeval time6;
	struct timeval time7;
      cl_device_id device_id[2];
  cl_context context[2];
  cl_command_queue command_queue[2];
  cl_mem memobjA[2];
  cl_mem memobjB[2];
  cl_mem memobjC[2];
  cl_mem rowA[2];
  cl_mem colC[2];
  cl_program program[2];
  cl_kernel kernel[2];
  cl_platform_id platform_id = NULL;
  cl_uint ret_num_devices;
  cl_uint ret_num_platforms;
  cl_int ret;

  //char string[MEM_SIZE];

  FILE *fp;
  char fileName[] = "./hello.cl";
  char *source_str;
  size_t source_size;
  int row = widthA;
  int col = heightC;
  /* Load the source code containing the kernel*/
  fp = fopen(fileName, "r");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel.\n");
    exit(1);
  }
  source_str = (char*)malloc(MAX_SOURCE_SIZE);
  source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
  fclose( fp );
  /* Get Platform and Device Info */
  ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
  ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_GPU, 2, device_id, &ret_num_devices);

  /* Create OpenCL context */
  context[0] = clCreateContext(0, 1, &device_id[0], NULL, NULL, &ret);
  context[1] = clCreateContext(0, 1, &device_id[1], NULL, NULL, &ret);

  /* Create Command Queue */
  command_queue[0] = clCreateCommandQueue(context[0], device_id[0], 0, &ret);
  command_queue[1] = clCreateCommandQueue(context[1], device_id[1], 0, &ret);


  /* Create Memory Buffer */
  memobjA[0] = clCreateBuffer(context[0], CL_MEM_READ_WRITE, widthA * heightA * sizeof(float), NULL, &ret);
  memobjA[1] = clCreateBuffer(context[1], CL_MEM_READ_WRITE, widthA * heightA * sizeof(float), NULL, &ret);

  memobjB[0] = clCreateBuffer(context[0], CL_MEM_READ_WRITE, widthB * heightB * sizeof(float), NULL, &ret);
  memobjB[1] = clCreateBuffer(context[1], CL_MEM_READ_WRITE, widthB * heightB * sizeof(float), NULL, &ret);
  
	
  memobjC[0] = clCreateBuffer(context[0], CL_MEM_READ_WRITE, widthC * heightC * sizeof(float), NULL, &ret);
  memobjC[1] = clCreateBuffer(context[1], CL_MEM_READ_WRITE, widthC * heightC * sizeof(float), NULL, &ret);

  rowA[0] = clCreateBuffer(context[0], CL_MEM_READ_WRITE,  sizeof(int), NULL, &ret);
  rowA[1] = clCreateBuffer(context[1], CL_MEM_READ_WRITE,  sizeof(int), NULL, &ret);

  colC[0] = clCreateBuffer(context[0], CL_MEM_READ_WRITE,  sizeof(int), NULL, &ret);
  colC[1] = clCreateBuffer(context[1], CL_MEM_READ_WRITE,  sizeof(int), NULL, &ret);


  program[0] = clCreateProgramWithSource(context[0], 1, (const char **)&source_str,(const size_t *)&source_size, &ret);
  program[1] = clCreateProgramWithSource(context[1], 1, (const char **)&source_str,(const size_t *)&source_size, &ret);

  /* Build Kernel Program */
  ret = clBuildProgram(program[0], 1, &device_id[0], NULL, NULL, NULL);
  ret = clBuildProgram(program[1], 1, &device_id[1], NULL, NULL, NULL);
  /* Create OpenCL Kernel */
  kernel[0] = clCreateKernel(program[0], "matrixMultiplication", &ret);
  kernel[1] = clCreateKernel(program[1], "matrixMultiplication", &ret);

  /* Set OpenCL Kernel Arguments */
  ret = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), (void *)&memobjA[0]);
  ret = clSetKernelArg(kernel[0], 1, sizeof(cl_mem), (void *)&memobjB[0]);
  ret = clSetKernelArg(kernel[0], 2, sizeof(cl_mem), (void *)&memobjC[0]);
  ret = clSetKernelArg(kernel[0], 3, sizeof(cl_int), (void *)&row);
  ret = clSetKernelArg(kernel[0], 4, sizeof(cl_int), (void *)&col);
  ret = clSetKernelArg(kernel[0], 5, sizeof(cl_int), (void *)&col);

  ret = clSetKernelArg(kernel[1], 0, sizeof(cl_mem), (void *)&memobjA[1]);
  ret = clSetKernelArg(kernel[1], 1, sizeof(cl_mem), (void *)&memobjB[1]);
  ret = clSetKernelArg(kernel[1], 2, sizeof(cl_mem), (void *)&memobjC[1]);
  ret = clSetKernelArg(kernel[1], 3, sizeof(cl_int), (void *)&row);
  ret = clSetKernelArg(kernel[1], 4, sizeof(cl_int), (void *)&col);
  ret = clSetKernelArg(kernel[1], 5, sizeof(cl_int), (void *)&col);


  //ret = clEnqueueTask(command_queue, kernel, 0, NULL,NULL);
  size_t globalThreads[2][2] = {{widthA, widthA/2},{widthA,widthA/2}};
  size_t offset[2][2] = {{0,0},{0,widthA/2}};
  //size_t localThreads[2] = {16, 16};
	gettimeofday (&time1, NULL);

  // Copy the lists A and B to their respective memory buffers
  ret = clEnqueueWriteBuffer(command_queue[0], memobjA[0], CL_TRUE, 0, widthA * heightA * sizeof(float), A, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue[0], memobjB[0], CL_TRUE, 0, widthB * heightB * sizeof(float), B, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue[0], rowA[0], CL_TRUE, 0, sizeof(int), &row, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue[0], colC[0], CL_TRUE, 0, sizeof(int), &col, 0, NULL, NULL);

  ret = clEnqueueWriteBuffer(command_queue[1], memobjA[1], CL_TRUE, 0, widthA * heightA * sizeof(float), A, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue[1], memobjB[1], CL_TRUE, 0, widthB * heightB * sizeof(float), B, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue[1], rowA[1], CL_TRUE, 0, sizeof(int), &row, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue[1], colC[1], CL_TRUE, 0, sizeof(int), &col, 0, NULL, NULL);

	clFinish(command_queue[0]);
	clFinish(command_queue[1]);
	gettimeofday (&time2, NULL);
  /* Execute OpenCL Kernel */

	gettimeofday (&time3, NULL);
        ret = clEnqueueNDRangeKernel (command_queue[0], kernel[0], 2, offset[0], globalThreads[0],NULL, 0, NULL, NULL);
        ret = clEnqueueNDRangeKernel (command_queue[1], kernel[1], 2, offset[1], globalThreads[1], NULL, 0, NULL, NULL);
        //printf("%d\n", ret);
	clFinish(command_queue[0]);
	clFinish(command_queue[1]);
	gettimeofday (&time4, NULL);
  /* Copy results from the memory buffer */
        ret = clEnqueueReadBuffer(command_queue[0], memobjC[0], CL_TRUE, 0, widthA * heightC * sizeof(float),Res, 0, NULL, NULL);
        ret = clEnqueueReadBuffer(command_queue[1], memobjC[1], CL_TRUE, 0, widthA * heightC * sizeof(float),Res, 0, NULL, NULL);

        clFinish (command_queue[0]);
        clFinish (command_queue[1]);
	gettimeofday (&time5, NULL);

        printf("upload  :%f\n", ((time2.tv_sec - time1.tv_sec)*1000000+ (time2.tv_usec - time1.tv_usec))/1000000.0);
	printf("%f\n", ((time4.tv_sec - time3.tv_sec)*1000000+ (time4.tv_usec - time3.tv_usec))/1000000.0);
	printf("download:%f\n", ((time5.tv_sec - time4.tv_sec)*1000000+ (time5.tv_usec - time4.tv_usec))/1000000.0);
	//printf("total   :%f\n", ((time5.tv_sec - time1.tv_sec)*1000000+ (time5.tv_usec - time1.tv_usec))/1000000.0);
  fp1 = fopen("matGPURes.txt", "w");
  if (!fp1) {
    fprintf(stderr, "Failed to open matAdata.\n");
    exit(1);
  }

 // printf("\nResult\n");
	for(i = 0;i < widthC; i++)
	{
		for(j=0;j < heightC; j++)
		{

			fprintf(fp1, "%f ",*(Res+i*heightC+j));

		}
		fprintf(fp1, "\n");
	}
	fclose(fp1);

  ret = clFlush(command_queue[0]);
  ret = clFinish(command_queue[0]);
  ret = clReleaseKernel(kernel[0]);
  ret = clReleaseProgram(program[0]);
  ret = clReleaseMemObject(memobjA[0]);
  ret = clReleaseMemObject(memobjB[0]);
  ret = clReleaseMemObject(memobjC[0]);
  ret = clReleaseCommandQueue(command_queue[0]);
  ret = clReleaseContext(context[0]);

  ret = clFlush(command_queue[1]);
  ret = clFinish(command_queue[1]);
  ret = clReleaseKernel(kernel[1]);
  ret = clReleaseProgram(program[1]);
  ret = clReleaseMemObject(memobjA[1]);
  ret = clReleaseMemObject(memobjB[1]);
  ret = clReleaseMemObject(memobjC[1]);
  ret = clReleaseCommandQueue(command_queue[1]);
  ret = clReleaseContext(context[1]);
  free(source_str);

 /* float sum=0.0;

  for(i = 0;i < widthA; i++)
	{
		for(j = 0; j < heightC; j++)
		{
			sum = 0;
			for(k = 0; k < widthB; k++)
			{
				sum += A[i*col+k] * B[k*row+j];
			}
		D[i*heightC+j] = sum;
		}

	}

    fp1 = fopen("matNormalMultiplicationRes.txt", "w");
  if (!fp1) {
    fprintf(stderr, "Failed to open matAdata.\n");
    exit(1);
  }

  printf("\nResult\n");
	for(i = 0;i < widthA; i++)
	{
		for(j=0;j < heightC; j++)
		{
			fprintf(fp1, "%f ",*(D+i*heightC+j));

		}
		fprintf(fp1, "\n");
	}*/
  return 0;
}

