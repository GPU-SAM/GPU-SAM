#include <CL/opencl.h>
#include <stdio.h>
#include <string.h>

#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif

/* Memory Object APIs */
CL_API_ENTRY cl_mem CL_API_CALL
__wrap_clCreateBuffer
              (cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_mem *mem_GPUSparc = (cl_mem *)malloc(sizeof(cl_mem) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++)
		mem_GPUSparc[i] = __real_clCreateBuffer (cont_GPUSparc[i], flags, size, host_ptr, errcode_ret);
	
	struct meminfo_GPUSparc minfo_GPUSparc;
	minfo_GPUSparc.mem 	         = mem_GPUSparc;
	minfo_GPUSparc.flags         = flags;
	minfo_GPUSparc.size	 		 = size;
	minfo_GPUSparc.shadow_buffer = malloc(size);
	memset (minfo_GPUSparc.shadow_buffer, 0, size);
	minfo_GPUSparc.element_size = 1;

	minfo_GPUSparc.region[0] = size;
	minfo_GPUSparc.region[1] = 1;
	minfo_GPUSparc.region[2] = 1;
	minfo_GPUSparc.merged = true;
	minfo_GPUSparc.synched = false;

	if (flags & CL_MEM_USE_HOST_PTR) {
		free (minfo_GPUSparc.shadow_buffer);
		minfo_GPUSparc.shadow_buffer = host_ptr;
		minfo_GPUSparc.synched = true;
	}
	if (flags & CL_MEM_COPY_HOST_PTR) {
		if (!(flags & CL_MEM_USE_HOST_PTR))
			memcpy (minfo_GPUSparc.shadow_buffer, host_ptr, size);
		minfo_GPUSparc.synched = true;
	}
	memmap_GPUSparc.insert (map<cl_mem, struct meminfo_GPUSparc>::value_type (mem_GPUSparc[0], minfo_GPUSparc));
	return mem_GPUSparc[0];
}

CL_API_ENTRY cl_mem CL_API_CALL
__wrap_clCreateSubBuffer
              (cl_mem                  buffer,
              cl_mem_flags             flags,
              cl_buffer_create_type    types,
              const void *             buf_create_info,
              cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_1_1 
{
	cl_uint i = 0;
	cl_mem *mem_GPUSparc = (cl_mem *)malloc(sizeof(cl_mem) * nGPU_GPUSparc);
	
	map <cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (buffer);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find mem in clCreateSubBuffer\n");
		return __real_clCreateSubBuffer (buffer, flags, types, buf_create_info, errcode_ret);
	}

	cl_mem *buf = miter_GPUSparc->second.mem;

	struct _cl_buffer_region *br = (struct _cl_buffer_region *)buf_create_info;

	size_t size = br->size;
	void *shadow_buffer = NULL;
	if (miter_GPUSparc->second.shadow_buffer != NULL)
		shadow_buffer = (void *)((size_t)miter_GPUSparc->second.shadow_buffer + br->origin);
	
	struct meminfo_GPUSparc minfo_GPUSparc;
	minfo_GPUSparc.mem = mem_GPUSparc;
	minfo_GPUSparc.flags = flags;
	minfo_GPUSparc.shadow_buffer = shadow_buffer;
	minfo_GPUSparc.size = size;
	minfo_GPUSparc.element_size = 1;
	minfo_GPUSparc.region[0] = size;
	minfo_GPUSparc.region[1] = 1;
	minfo_GPUSparc.region[2] = 1;
	minfo_GPUSparc.merged = miter_GPUSparc->second.merged;
	minfo_GPUSparc.synched = miter_GPUSparc->second.synched;

	for (i = 0; i < nGPU_GPUSparc; i++)
		mem_GPUSparc[i] = __real_clCreateSubBuffer (buf[i], flags, types, buf_create_info, errcode_ret);

	memmap_GPUSparc.insert (map<cl_mem, struct meminfo_GPUSparc>::value_type (mem_GPUSparc[0], minfo_GPUSparc));
	return mem_GPUSparc[0];
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clRetainMemObject
               (cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i = 0;
	cl_int ret = CL_SUCCESS;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (memobj);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannnot find mem in clRetainMemObject\n");
		return __real_clRetainMemObject (memobj);
	}

	cl_mem *mem_GPUSparc = miter_GPUSparc->second.mem;
	for (i = 0; i < nGPU_GPUSparc; i++) 
		ret |= __real_clRetainMemObject (mem_GPUSparc[i]);

	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clReleaseMemObject
               (cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i = 0;
	cl_int ret = CL_SUCCESS;
	map<cl_mem, struct meminfo_GPUSparc>::iterator miter_GPUSparc;
	miter_GPUSparc = memmap_GPUSparc.find (memobj);
	if (miter_GPUSparc == memmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find mem in clReleaseMemObject\n");
		return __real_clReleaseMemObject (memobj);
	}

	cl_mem *mem_GPUSparc = miter_GPUSparc->second.mem;
	for (i = 0; i < nGPU_GPUSparc; i++) 
		ret |= __real_clReleaseMemObject (mem_GPUSparc[i]);
	
	if (!(miter_GPUSparc->second.flags & CL_MEM_USE_HOST_PTR) && miter_GPUSparc->second.shadow_buffer != NULL)
		free (miter_GPUSparc->second.shadow_buffer);
	
	free (mem_GPUSparc);
	memmap_GPUSparc.erase (miter_GPUSparc);
	return ret;
}


CL_API_ENTRY cl_mem CL_API_CALL
__wrap_clCreateImage2D
               (cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_row_pitch, 
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	size_t element_size_GPUSparc;
	size_t channel_GPUSparc;
	analyzeImageFormat_GPUSparc (image_format, &element_size_GPUSparc, &channel_GPUSparc);

	size_t size = image_width * image_height * element_size_GPUSparc;
	
	cl_mem *mem_GPUSparc = (cl_mem *)malloc(sizeof(cl_mem) * nGPU_GPUSparc);
	struct meminfo_GPUSparc minfo_GPUSparc;
	minfo_GPUSparc.mem = mem_GPUSparc;
	minfo_GPUSparc.flags = flags;
	minfo_GPUSparc.size = size;
	minfo_GPUSparc.shadow_buffer = malloc(size);
	memset (minfo_GPUSparc.shadow_buffer, 0, size);

	minfo_GPUSparc.element_size = element_size_GPUSparc;
	minfo_GPUSparc.region[0] = image_width;
	minfo_GPUSparc.region[1] = image_height;
	minfo_GPUSparc.region[2] = 1;
	minfo_GPUSparc.merged = true;
	minfo_GPUSparc.synched = false;
		
	if (flags & CL_MEM_USE_HOST_PTR) {
		free (minfo_GPUSparc.shadow_buffer);
		minfo_GPUSparc.shadow_buffer = host_ptr;
		minfo_GPUSparc.synched = true;
	}
	if (flags & CL_MEM_COPY_HOST_PTR) {
		if (!(flags & CL_MEM_USE_HOST_PTR))
			memcpy (minfo_GPUSparc.shadow_buffer, host_ptr, size);
		minfo_GPUSparc.synched = true;
	}	
	
	for (i = 0; i < nGPU_GPUSparc; i++)
		mem_GPUSparc[i] = __real_clCreateImage2D (cont_GPUSparc[i], flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret);

	memmap_GPUSparc.insert (map<cl_mem, struct meminfo_GPUSparc>::value_type (mem_GPUSparc[0], minfo_GPUSparc));
	return mem_GPUSparc[0];
}
                        
CL_API_ENTRY cl_mem CL_API_CALL
__wrap_clCreateImage3D
               (cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width, 
                size_t                  image_height,
                size_t                  image_depth, 
                size_t                  image_row_pitch, 
                size_t                  image_slice_pitch, 
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	size_t element_size_GPUSparc;
	size_t channel_GPUSparc;
	analyzeImageFormat_GPUSparc (image_format, &element_size_GPUSparc, &channel_GPUSparc);

	size_t size = image_width * image_height * image_depth * element_size_GPUSparc;

	cl_mem *mem_GPUSparc = (cl_mem *)malloc(sizeof(cl_mem) * nGPU_GPUSparc);
	struct meminfo_GPUSparc minfo_GPUSparc;
	minfo_GPUSparc.mem = mem_GPUSparc;
	minfo_GPUSparc.flags = flags;
	minfo_GPUSparc.shadow_buffer = malloc(size);
	memset (minfo_GPUSparc.shadow_buffer, 0, size);

	minfo_GPUSparc.element_size = element_size_GPUSparc;

	minfo_GPUSparc.region[0] = image_width;
	minfo_GPUSparc.region[1] = image_height;
	minfo_GPUSparc.region[2] = image_depth;
	minfo_GPUSparc.merged = true;
	minfo_GPUSparc.synched = false;

	if (flags & CL_MEM_USE_HOST_PTR) {
		free (minfo_GPUSparc.shadow_buffer);
		minfo_GPUSparc.shadow_buffer = host_ptr;
		minfo_GPUSparc.synched = true;
	}
	if (flags & CL_MEM_COPY_HOST_PTR) {
		if (!(flags & CL_MEM_USE_HOST_PTR))
			memcpy (minfo_GPUSparc.shadow_buffer, host_ptr, size);
		minfo_GPUSparc.synched = true;
	}

	for (i = 0; i < nGPU_GPUSparc; i++) 
		mem_GPUSparc[i] = __real_clCreateImage3D (cont_GPUSparc[i], flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
	
	memmap_GPUSparc.insert (map<cl_mem, struct meminfo_GPUSparc>::value_type (mem_GPUSparc[0], minfo_GPUSparc));
	return mem_GPUSparc[0];
}


CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetSupportedImageFormats
                          (cl_context           context,
                           cl_mem_flags         flags,
                           cl_mem_object_type   image_type,
                           cl_uint              num_entries,
                           cl_image_format *    image_formats,
                           cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetSupportedImageFormats (context, flags, image_type, num_entries, image_formats, num_image_formats);
}
                                    
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetMemObjectInfo
                  (cl_mem           memobj,
                   cl_mem_info      param_name, 
                   size_t           param_value_size,
                   void *           param_value,
                   size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
  return __real_clGetMemObjectInfo (memobj, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetImageInfo
              (cl_mem           image,
               cl_image_info    param_name, 
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
  return __real_clGetImageInfo (image, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clSetMemObjectDestructorCallback
					(cl_mem memobj, 
					 void (CL_CALLBACK * pfn_notify)( cl_mem memobj, void* user_data), 
					 void * user_data) CL_API_SUFFIX__VERSION_1_1 
{
	return __real_clSetMemObjectDestructorCallback (memobj, pfn_notify, user_data);
}

#ifdef __cplusplus
}
#endif
