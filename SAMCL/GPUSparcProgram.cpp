#include <CL/opencl.h>
#include <stdio.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"

#define MAX_SOURCE_SIZE 30000

#ifdef __cplusplus
extern "C" {
#endif

/* Program Object APIs */
CL_API_ENTRY cl_program CL_API_CALL
__wrap_clCreateProgramWithSource
                         (cl_context        context,
                          cl_uint           count,
                          const char **     strings,
                          const size_t *    lengths,
                          cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_program *programs_GPUSparc = (cl_program *)malloc(sizeof (cl_program) * nGPU_GPUSparc);
	cl_uint i;

	FILE *fp = kernelParser_GPUSparc (*strings);
	char *buf = (char *)malloc(MAX_SOURCE_SIZE);
	size_t len;
	len = fread (buf, 1, MAX_SOURCE_SIZE, fp);
	fclose (fp);
	GPUSparcLog ("%s\n", buf);

	for (i = 0; i < nGPU_GPUSparc; i++)
		programs_GPUSparc[i] = __real_clCreateProgramWithSource (cont_GPUSparc[i], count, (const char **)&buf, &len, errcode_ret);
	
	programmap_GPUSparc.insert (map<cl_program, cl_program *>::value_type(programs_GPUSparc[0], programs_GPUSparc));
	return programs_GPUSparc[0];
}

CL_API_ENTRY cl_program CL_API_CALL
__wrap_clCreateProgramWithBinary
                         (cl_context                     context,
                          cl_uint                        num_devices,
                          const cl_device_id *           device_list,
                          const size_t *                 lengths,
                          const unsigned char **         binaries,
                          cl_int *                       binary_status,
                          cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
	cl_program *programs_GPUSparc = (cl_program *)malloc(sizeof (cl_program) * nGPU_GPUSparc);
	cl_uint i;
	for (i = 0; i < nGPU_GPUSparc; i++)
		programs_GPUSparc[i] = __real_clCreateProgramWithBinary (cont_GPUSparc[i], 1, &dev_GPUSparc[i], lengths, binaries, binary_status, errcode_ret);
	programmap_GPUSparc.insert (map<cl_program, cl_program *>::value_type(programs_GPUSparc[0], programs_GPUSparc));
	return programs_GPUSparc[0];
}


CL_API_ENTRY cl_int CL_API_CALL
__wrap_clRetainProgram
                 (cl_program program) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_program, cl_program *>::iterator piter_GPUSparc;
	piter_GPUSparc = programmap_GPUSparc.find (program);
	if (piter_GPUSparc == programmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find program in clRetainProgram\n");
		ret =  __real_clRetainProgram (program);
	}
	else {
		cl_program *programs_GPUSparc = piter_GPUSparc->second;
		for (i = 0; i < nGPU_GPUSparc; i++)
			ret |= __real_clRetainProgram (programs_GPUSparc[i]);
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clReleaseProgram
                 (cl_program program) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_program, cl_program *>::iterator piter_GPUSparc;
	piter_GPUSparc = programmap_GPUSparc.find (program);
	if (piter_GPUSparc == programmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find program in clReleaseProgram\n");
		ret =  __real_clReleaseProgram (program);
	}
	else {
		cl_program *programs_GPUSparc = piter_GPUSparc->second;
		for (i = 0; i < nGPU_GPUSparc; i++)
			ret |= __real_clReleaseProgram (programs_GPUSparc[i]);
		free (programs_GPUSparc);
		programmap_GPUSparc.erase (piter_GPUSparc);
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clBuildProgram
              (cl_program           program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options, 
               void (CL_CALLBACK * pfn_notify)(cl_program program, void * user_data),
               void *               user_data) CL_API_SUFFIX__VERSION_1_0 
{
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	map<cl_program, cl_program *>::iterator piter_GPUSparc;
	piter_GPUSparc = programmap_GPUSparc.find (program);
	if (piter_GPUSparc == programmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find program in clBuildProgram\n");
		ret = __real_clBuildProgram (program, num_devices, device_list, options, pfn_notify, user_data);
	}
	else {
		cl_program *programs_GPUSparc = piter_GPUSparc->second;
		for (i = 0; i < nGPU_GPUSparc; i++)
			ret |= __real_clBuildProgram (programs_GPUSparc[i], 1, &dev_GPUSparc[i], options, pfn_notify, user_data);
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetProgramInfo
                (cl_program         program,
                 cl_program_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
  return __real_clGetProgramInfo (program, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetProgramBuildInfo
                     (cl_program            program,
                      cl_device_id          device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetProgramBuildInfo (program, device, param_name, param_value_size, param_value, param_value_size_ret);
}


#ifdef __cplusplus
}
#endif
