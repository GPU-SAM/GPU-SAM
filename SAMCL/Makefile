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

CC=g++
SOURCES=GPUSparcCommand.cpp GPUSparcCommandQueue.cpp GPUSparcContext.cpp GPUSparcDevice.cpp GPUSparcEvent.cpp GPUSparcFlushFinish.cpp GPUSparcKernel.cpp GPUSparcMem.cpp GPUSparcPlatform.cpp GPUSparcProfiling.cpp GPUSparcProgram.cpp GPUSparcSampler.cpp GPUSparc.cpp GPUSparcBufferManager.cpp GPUSparcDecomposer.cpp GPUSparcKernelParser.cpp
C_INCLUDE_PATH:=./
LDFLAGS= -lOpenCL $(LINKTAIL)
OBJECTS=$(SOURCES:.cpp=.o)

all: libGPUSparc.so

libGPUSparc.so: $(OBJECTS)
	$(CC) -O3 -shared -o libGPUSparc.so $(OBJECTS) $(LDFLAGS) -lpthread -lfl -lrt -lc

.cpp.o: $(SOURCES)
	flex -o GPUSparcKernelParser.cpp GPUSparcKernelParser.l
	$(CC) -O3 -c -Wall -fPIC $(SOURCES) -I$(C_INCLUDE_PATH)

clean:
	rm $(OBJECTS) libGPUSparc.so GPUSparcKernelParser.cpp
