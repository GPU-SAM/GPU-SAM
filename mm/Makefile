LINKTAIL  := -Wl,--wrap,clGetPlatformIDs,--wrap,clGetPlatformInfo,--wrap,clGetDeviceIDs,--wrap,clGetDeviceInfo,--wrap,clCreateSubDevices
LINKTAIL  += -Wl,--wrap,clRetainDevice,--wrap,clReleaseDevice,--wrap,clCreateContext,--wrap,clCreateContextFromType,--wrap,clRetainContext
LINKTAIL  += -Wl,--wrap,clReleaseContext,--wrap,clGetContextInfo,--wrap,clCreateCommandQueue,--wrap,clRetainCommandQueue,--wrap,clReleaseCommandQueue
LINKTAIL  += -Wl,--wrap,clGetCommandQueueInfo,--wrap,clCreateBuffer,--wrap,clCreateSubBuffer,--wrap,clCreateImage,--wrap,clRetainMemObject
LINKTAIL  += -Wl,--wrap,clReleaseMemObject,--wrap,clGetSupportedImageFormats,--wrap,clGetMemObjectInfo,--wrap,clGetImageInfo,--wrap,clSetMemObjectDestructorCallback
LINKTAIL  += -Wl,--wrap,clCreateSampler,--wrap,clRetainSampler,--wrap,clReleaseSampler,--wrap,clGetSamplerInfo,--wrap,clCreateProgramWithSource,--wrap,clCreateProgramWithBinary,--wrap,clCreateProgramWithBuiltInKernels,--wrap,clRetainProgram,--wrap,clReleaseProgram
LINKTAIL  += -Wl,--wrap,clBuildProgram,--wrap,clCompileProgram,--wrap,clLinkProgram,--wrap,clUnloadPlatformCompiler,--wrap,clGetProgramInfo
LINKTAIL  += -Wl,--wrap,clGetProgramBuildInfo,--wrap,clCreateKernel,--wrap,clCreateKernelsInProgram,--wrap,clRetainKernel,--wrap,clReleaseKernel
LINKTAIL  += -Wl,--wrap,clSetKernelArg,--wrap,clGetKernelInfo,--wrap,clGetKernelArgInfo,--wrap,clGetKernelWorkGroupInfo,--wrap,clWaitForEvents
LINKTAIL  += -Wl,--wrap,clGetEventInfo,--wrap,clCreateUserEvent,--wrap,clRetainEvent,--wrap,clReleaseEvent,--wrap,clSetUserEventStatus,--wrap,clSetEventCallback,--wrap,clGetEventProfilingInfo,--wrap,clFlush,--wrap,clFinish
LINKTAIL  += -Wl,--wrap,clEnqueueReadBuffer,--wrap,clEnqueueReadBufferRect,--wrap,clEnqueueWriteBuffer,--wrap,clEnqueueWriteBufferRect,--wrap,clEnqueueFillBuffer
LINKTAIL  += -Wl,--wrap,clEnqueueCopyBuffer,--wrap,clEnqueueCopyBufferRect,--wrap,clEnqueueReadImage,--wrap,clEnqueueWriteImage,--wrap,clEnqueueFillImage
LINKTAIL  += -Wl,--wrap,clEnqueueCopyImage,--wrap,clEnqueueCopyImageToBuffer,--wrap,clEnqueueCopyBufferToImage,--wrap,clEnqueueMapBuffer,--wrap,clEnqueueMapImage
LINKTAIL  += -Wl,--wrap,clEnqueueUnmapMemObject,--wrap,clEnqueueMigrateMemObjects,--wrap,clEnqueueNDRangeKernel,--wrap,clEnqueueTask,--wrap,clEnqueueNativeKernel
LINKTAIL  += -Wl,--wrap,clEnqueueMarkerWithWaitList,--wrap,clEnqueueBarrierWithWaitList,--wrap,clSetCommandQueueProperty
LINKTAIL  += -Wl,--wrap,clCreateImage2D,--wrap,clCreateImage3D,--wrap,clEnqueueMarker,--wrap,clEnqueueWaitForEvents,--wrap,clEnqueueBarrier
LINKTAIL  += -Wl,--wrap,clUnloadCompiler

all: mm mm-dual

mm: mm.c
	gcc -O3 -o mm mm.c -lOpenCL
mm-dual: mm.c
	gcc -O3 -o mm-dual mm.c -L/home/cpslab/GPUSAM/GPU-SAM/SAMCL/ -lGPUSAM $(LINKTAIL)

clean: 
	rm mm mm-dual
