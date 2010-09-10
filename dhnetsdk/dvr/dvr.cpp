
#include "StdAfx.h"
#include "dvr.h"
#include "devprob.h"
#include <stdlib.h>

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
    switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}
#endif


void * create_prob(void *)
{
	yw_prob_t *prob = (yw_prob_t *)malloc(sizeof(yw_prob_t));
	memset(prob, 0, sizeof(yw_prob_t));
	prob->base.try_connect = try_connect_dvr;
	prob->base.start_listen = start_listen_dvr;
	prob->base.stop_listen = stop_listen_dvr;
	prob->base.close_client = close_client_dvr;
	prob->base.response_reg = response_reg_dvr;
	prob->base.net_startup = network_startup;
	prob->base.net_cleanup = network_cleanup;
	prob->base.start_alarm_server = _start_alarm_server;
	prob->base.stop_alarm_server = _stop_alarm_server;
	prob->base.server_heart_beat = _heart_beat;
	prob->base.start_search_device = _start_search_device;
	prob->base.stop_search_device = _stop_search_device;
	prob->base.new_tcp_socket = _new_tcp_socket;
	prob->base.delete_tcp_socket = _delete_tcp_socket;
	prob->base.write_data = _write_data;
	return prob;
}

DVR_API afk_plugin_info_t plugin_info = {
	AFK_PLUGIN_DEVICEPROBE,
	0x00000001,
	5,
	"dahua dvr",
	0,
    0,
	create_prob		
};

