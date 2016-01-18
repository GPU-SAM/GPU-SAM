#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define widthA 2048
#define heightA 2048

#define widthB 2048
#define heightB widthA

#define widthC widthB
#define heightC heightA

#include <CL/opencl.h>
#define MAX_SOURCE_SIZE (0x100000)

int main(int argc, char **argv)
{
  int w = atoi(argv[1]);
  int h = atoi(argv[2]);
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

  for(i = 0;i < heightA; i++)
  {
		for(j=0;j	< widthA; j++)	{
			float p=(rand()%100)/7.0;
			*(A+i*widthA+j)=rand()%100 + p;
			fprintf(fp1, "%f ",*(A+i*widthA+j));
		}
		fprintf(fp1, "\n");
  }
  fclose(fp1);

  fp1 = fopen("matBdata.txt", "w");
  if (!fp1) {
   	fprintf(stderr, "Failed to open matAdata.\n");
  	exit(1);
  }

	for(i = 0;i < heightB; i++)
	{
		for(j=0; j < widthB; j++){
			float p=(rand()%100)/7.0;
			*((B+i*widthB+j))=rand()%100 + p;
			fprintf(fp1, "%f ",*(B+i*widthB+j));
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
  cl_context context = NULL;
  cl_command_queue command_queue = NULL;
  cl_mem memobjA = NULL;
  cl_mem memobjB = NULL;
  cl_mem memobjC = NULL;
  cl_mem rowA = NULL;
  cl_mem colC = NULL;
  cl_program program = NULL;
  cl_kernel kernel = NULL;
  cl_platform_id platform_id = NULL;
  cl_uint ret_num_devices;
  cl_uint ret_num_platforms;
  cl_int ret;

  //char string[MEM_SIZE];

  FILE *fp;
  char fileName[] = "./hello.cl";
  char *source_str;
  size_t source_size;
  int row = widthC;
  int col = heightC;
  int process = widthA;
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
  context = clCreateContext(0, 1, &device_id[0], NULL, NULL, &ret);

  /* Create Command Queue */
  command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);

  /* Create Memory Buffer */
  memobjA = clCreateBuffer(context, CL_MEM_READ_WRITE, widthA * heightA * sizeof(float), NULL, &ret);

  memobjB = clCreateBuffer(context, CL_MEM_READ_WRITE, widthB * heightB * sizeof(float), NULL, &ret);
	
  memobjC = clCreateBuffer(context, CL_MEM_READ_WRITE, widthC * heightC * sizeof(float), NULL, &ret);

  rowA = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(int), NULL, &ret);
  colC = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(int), NULL, &ret);


  program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
				                      (const size_t *)&source_size, &ret);

  /* Build Kernel Program */
  ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);

  /* Create OpenCL Kernel */
  kernel = clCreateKernel(program, "matrixMultiplication", &ret);

  /* Set OpenCL Kernel Arguments */
  ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobjA);
  ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memobjB);
  ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&memobjC);
  ret = clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&process);
  ret = clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&col);
  ret = clSetKernelArg(kernel, 5, sizeof(cl_int), (void *)&row);

  //ret = clEnqueueTask(command_queue, kernel, 0, NULL,NULL);
  size_t globalThreads[2] = {widthC/w, heightC/h};
  //size_t localThreads[2] = {16, 16};
	gettimeofday (&time1, NULL);

  // Copy the lists A and B to their respective memory buffers
  ret = clEnqueueWriteBuffer(command_queue,memobjA, CL_TRUE, 0,
           widthA * heightA * sizeof(int), A, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue, memobjB, CL_TRUE, 0,
            widthB * heightB * sizeof(int), B, 0, NULL, NULL);
	//ret = clEnqueueWriteBuffer(command_queue, rowA, CL_TRUE, 0, sizeof(int), &row, 0, NULL, NULL);
//	ret = clEnqueueWriteBuffer(command_queue, colC, CL_TRUE, 0, sizeof(int), &col, 0, NULL, NULL);
	gettimeofday (&time2, NULL);
  /* Execute OpenCL Kernel */
  //ret = clEnqueueTask(command_queue, kernel, 0, NULL,NULL);
	gettimeofday (&time3, NULL);
	clEnqueueNDRangeKernel (command_queue, kernel, 2, NULL, globalThreads,NULL, 0, NULL, NULL);
	clFinish(command_queue);
	gettimeofday (&time4, NULL);
  /* Copy results from the memory buffer */
  ret = clEnqueueReadBuffer(command_queue, memobjC, CL_TRUE, 0,
			                widthC * heightC * sizeof(float),Res, 0, NULL, NULL);

	clFinish (command_queue);
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

  /*printf("\nResult\n");
	for(i = 0;i < heightC; i++)
	{
		for(j=0;j < widthC; j++)
		{

			fprintf(fp1, "%f ",*(Res+i*widthC+j));

		}
		fprintf(fp1, "\n");
	}
	fclose(fp1);
*/
  ret = clFlush(command_queue);
  ret = clFinish(command_queue);
  ret = clReleaseKernel(kernel);
  ret = clReleaseProgram(program);
  ret = clReleaseMemObject(memobjA);
  ret = clReleaseMemObject(memobjB);
  ret = clReleaseMemObject(memobjC);
  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);

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

