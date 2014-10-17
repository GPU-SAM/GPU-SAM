#include <CL/opencl.h>
#include <stdio.h>
#include "CLAPI.h"
#include "GPUSparc.hpp"

cl_int waitEventGPUSparc (cl_uint num_events,
						  const cl_event *event_list)
{
	cl_uint i, j;
	cl_int ret = CL_SUCCESS;;

	for(i = 0; i < num_events; i++) {
		map<cl_event, cl_event *>::iterator eiter;
		eiter = eventmap_GPUSparc.find (event_list[0]);
		if (eiter == eventmap_GPUSparc.end())
			continue;
		else {
			cl_event *events = eiter->second;
			for(j = 0; j < nGPU_GPUSparc; j++) {
				ret |= __real_clWaitForEvents (1, &events[j]);
			}
		}
	}
	return ret;
}
				   

#ifdef __cplusplus
extern "C" {
#endif

/*Event Object APIs */
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clWaitForEvents
               (cl_uint           num_events,
                const cl_event *  event_list) CL_API_SUFFIX__VERSION_1_0 
{
	return waitEventGPUSparc (num_events, event_list);
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clGetEventInfo
              (cl_event         event,
               cl_event_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 
{
	return __real_clGetEventInfo (event, param_name, param_value_size, param_value, param_value_size_ret);
}

CL_API_ENTRY cl_event CL_API_CALL
__wrap_clCreateUserEvent
                 (cl_context       context,
                  cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_1 
{
	cl_uint i;
	cl_event *event_GPUSparc = (cl_event *)malloc(sizeof(cl_event) * nGPU_GPUSparc);
	
	for (i = 0; i < nGPU_GPUSparc; i++) {
		event_GPUSparc[i] = __real_clCreateUserEvent (cont_GPUSparc[i], errcode_ret);
	}	
	eventmap_GPUSparc.insert (map<cl_event, cl_event *>::value_type (event_GPUSparc[0], event_GPUSparc));
	return __real_clCreateUserEvent (context, errcode_ret);
}
                            
CL_API_ENTRY cl_int CL_API_CALL
__wrap_clRetainEvent
               (cl_event event) CL_API_SUFFIX__VERSION_1_0 
{
	map<cl_event, cl_event *>::iterator eiter;
	eiter = eventmap_GPUSparc.find (event);
	if (eiter == eventmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find event in clRetainEvent\n");
		return __real_clRetainEvent (event);
	}	

	cl_uint i;
	cl_int ret = CL_SUCCESS;
	cl_event *events = eiter->second;

	for(i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clRetainEvent (events[i]);
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clReleaseEvent
             (cl_event event) CL_API_SUFFIX__VERSION_1_0 
{
	map<cl_event, cl_event *>::iterator eiter;
	eiter = eventmap_GPUSparc.find (event);
	if (eiter == eventmap_GPUSparc.end()) {
		GPUSparcLog ("Cannot find event in clReleaseEvent\n");
		return __real_clReleaseEvent (event);
	}

	cl_uint i;
	cl_int ret = CL_SUCCESS;
	cl_event *events = eiter->second;

	for(i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clReleaseEvent (events[i]);
	}
	
	free (events);
	eventmap_GPUSparc.erase (eiter);
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clSetUserEventStatus
             (cl_event      event,
              cl_int        execution_status) CL_API_SUFFIX__VERSION_1_1 
{
	map<cl_event, cl_event *>::iterator eiter;
	eiter = eventmap_GPUSparc.find (event);
	if (eiter == eventmap_GPUSparc.end()) {
		GPUSparcLog ("Cannof find event in clSetUserEventStatus\n");
		return CL_INVALID_EVENT;
	}
	cl_uint i;
	cl_int ret = CL_SUCCESS;
	cl_event *events = eiter->second;

	for(i = 0; i < nGPU_GPUSparc; i++) {
		ret |= __real_clSetUserEventStatus (events[i], execution_status);
	}
	return ret;
}

CL_API_ENTRY cl_int CL_API_CALL
__wrap_clSetEventCallback
                   (cl_event           event,
                    cl_int             command_exec_callback_type,
                    void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
                    void *             user_data) CL_API_SUFFIX__VERSION_1_1 
{
	return __real_clSetEventCallback (event, command_exec_callback_type, pfn_notify, user_data);
}


#ifdef __cplusplus
}
#endif
