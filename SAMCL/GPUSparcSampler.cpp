#include <CL/opencl.h>
#include <stdio.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"


#ifdef __cplusplus
extern "C" {
#endif

/* Sampler APIs */
CL_API_ENTRY cl_sampler CL_API_CALL
__wrap_clCreateSampler
               (cl_context          context,
                cl_bool             normalized_coords, 
                cl_addressing_mode  addressing_mode, 
                cl_filter_mode      filter_mode,
                cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_sampler *sampler_GPUSparc = (cl_sampler *)malloc(sizeof(cl_sampler) * nGPU_GPUSparc);
	for (i = 0; i < nGPU_GPUSparc; i++)
		sampler_GPUSparc[i] = __real_clCreateSampler (cont_GPUSparc[i], normalized_coords, addressing_mode, filter_mode, errcode_ret);
	samplermap_GPUSparc.insert (map<cl_sampler, cl_sampler *>::value_type (sampler_GPUSparc[0], sampler_GPUSparc));
	return sampler_GPUSparc[0];
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clRetainSampler
               (cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_sampler, cl_sampler *>::iterator siter_GPUSparc;
	siter_GPUSparc = samplermap_GPUSparc.find (sampler);
	if (siter_GPUSparc == samplermap_GPUSparc.end()) {
		GPUSparcLog("Cannot find sampler in clRetainSampler\n");
		return __real_clRetainSampler (sampler);
	}
	cl_sampler *sampler_GPUSparc = siter_GPUSparc->second;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clRetainSampler (sampler_GPUSparc[i]);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clReleaseSampler
               (cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_sampler, cl_sampler *>::iterator siter_GPUSparc;
	siter_GPUSparc = samplermap_GPUSparc.find (sampler);
	if (siter_GPUSparc == samplermap_GPUSparc.end()) {
		GPUSparcLog("Cannot find sampler in clReleaseSampler\n");
		return __real_clReleaseSampler (sampler);
	}
	cl_sampler *sampler_GPUSparc = siter_GPUSparc->second;
	for (i = 0; i < nGPU_GPUSparc; i++)
		ret |= __real_clReleaseSampler (sampler_GPUSparc[i]);
	free (sampler_GPUSparc);
	samplermap_GPUSparc.erase (siter_GPUSparc);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetSamplerInfo
                (cl_sampler         sampler,
                 cl_sampler_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetSamplerInfo (sampler, param_name, param_value_size, param_value, param_value_size_ret);
}

#ifdef __cplusplus
}
#endif
