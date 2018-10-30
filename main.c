#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/epoll.h>

#include "uds.h"
#include "uds_network_layer.h"
#include "uds_session_layer.h"

/***************************************************************************************************************************************************
*
*function declear
*
****************************************************************************************************************************************************/

/***************************************************************************************************************************************************
*
*varlue declear
*
****************************************************************************************************************************************************/

unsigned char data_send0[8] = {0x10,0x2B,'B','o','o','m',',','a'};
unsigned char data_send1[8] = {0x21,' ','n','e','w',' ','m','u'};
unsigned char data_send2[8] = {0x22,'l','i','t','-','f','r','a'};
unsigned char data_send3[8] = {0x23,'m','e',' ','m','e','s','s'};
unsigned char data_send4[8] = {0x24,'a','g','e',' ','i','s',' '};
unsigned char data_send5[8] = {0x25,'r','e','c','i','v','e','d'};
unsigned char data_send6[8] = {0x26,'!',0,0,0,0,0,0};

unsigned char data_send7[8] = {0x10,0x38,'W','h','e','n',' ','I'};
unsigned char data_send8[8] = {0x21,' ','f','e','e','l',' ','t'};
unsigned char data_send9[8] = {0x22,'h','e',' ','s','k','y',' '};
unsigned char data_send10[8] = {0x23,'m','i','g','h','t',' ','f'};
unsigned char data_send11[8] = {0x24,'a','l','l',' ','a','n','d'};
unsigned char data_send12[8] = {0x25,' ','n','o',' ','o','n','e'};
unsigned char data_send13[8] = {0x26,' ','h','e','a','r',' ','m'};
unsigned char data_send14[8] = {0x27,'y',' ','c','a','l','l','.'};
unsigned char data_send15[8] = {0x28,0,0,0,0,0,0,0};

unsigned char data_send16[8] = {0x07,'S','F',' ','M','e','s',0};


unsigned char send_flag = 0;

unsigned char *data_list[] = {
	data_send0,
	data_send1,
	data_send2,
	data_send3,
	data_send4,
/*	data_send7,
	data_send8,
	data_send9,
	data_send10,
	data_send11,
	data_send12,
	data_send13,
//	data_send14,
//	data_send15	
*/
	data_send5,
	data_send6

};

unsigned char *data_list1[] = {
	data_send7,
	data_send8,
	data_send9,
	data_send10,
	data_send11,
	data_send12,
	data_send13,
	data_send14,
	data_send15
};

UDS_N_USData_Request_t usdata_request[] = 
{
{
{0x01,0x02,0x03,0x04},
"The handling of the P2CAN_Client and P2 CAN_Server timing is identical to the handling as described in 6.3.5.2.1 and 6.3.5.2.2, the only exception being that the reload values on the client side and the resulting time the server shall send its final response time might differ. This is based on the transition into a session other than the default session where different P2 CAN_Client timing parameters might apply (see DiagnosticSessionControl (10 hex) service in 9.2.1 for details on how the timing parameters are reported to the client).",
600
},


{
{0x01,0x02,0x03,0x04},
"There is a request message.",
28
},

{
{0x01,0x02,0x03,0x04},
"--Part 1: General information",
30
},

{
{0x01,0x02,0x03,0x04},
"--Part 2: Network layer services",
33
},

{
{0x01,0x02,0x03,0x04},
"--Part 3: Implementation of unified diagnostic services (UDS on CAN)",
69
},

{
{0x01,0x02,0x03,0x04},
"--Part 4: Requirements for emissions-related systems",
53
},

{
{0x01,0x02,0x03,0x04},
"Over",
5
},

};

unsigned char *result_char[] = 
{
	"N_OK",		//Execution has completed successfully
	"N_TIMEOUT_A",	//When the timer N_Ar/N_As has passed
	"N_TIMEOUT_Bs",	//When the timer N_Bs has passed
	"N_TIMEOUT_Cr",	//When the timer N_Cr has passed
	"N_WRONG_SN",		//Reception of an unexpected sequence number 
	"N_INVALID_FS",	//Unknown FlowStatus value has been received
	"N_UNEXP_PDU",	//Reception of an unexpected protocol data unit
	"N_WFT_OVRN",		//Flow control WAIT frame that exceeds the maximum counter N_WFTmax. 
	"N_BUFFER_OVFLW", //On reception of a flow control (FC) N_PDU with FlowStatus = OVFLW
	"N_ERROR"			//An error has been detected

};

unsigned char *frame_char[] = 
{
	"SF",	//Single Frame
	"FF",	//First Frame
	"CF",	//Consecutive Frame
	"FC"	//Flow Control Frame

};

/***************************************************************************************************************************************************
*
*pthread
*
****************************************************************************************************************************************************/
void print_time(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	printf("					time = %d\n\r",ts.tv_nsec/1000000);

}

unsigned char *get_result_char(int res)
{
	return result_char[res];
}
unsigned char *get_frame_char(int res)
{
	return frame_char[res];
}

void thread_time_ctl(void *argv)
{
	char getchart[50];
	int i = 0;
	struct timespec ts;
	
	while(1){
		uds_time_handle();
		usleep(1000);
	}
}
void thread_message_send(void *argv)
{
	int i = 0,frame_cnt = 0,ch = 6;
	unsigned char data_send[8] = {0x07,'S','F',' ','M','e','s',0};
	unsigned char **char_d;

	char_d = data_list;
	while(1){
		
		if(frame_cnt >= 7){
			frame_cnt = 0;
		}
		i = UDS_S_service_process_USData_request((void *)&usdata_request[frame_cnt++]);
		if(1 != i){
			printf("USData_request failed!\n\r");
		}
		sleep(3);

		/*
		if(0 == send_flag){
			printf("send FF frame\n\r");
			send_flag = 1;
			i = UDS_N_can_data_put(0x66,8,char_d[frame_cnt++]);
			usleep(15000);
		}
		if(2 == send_flag){
			i = UDS_N_can_data_put(0x66,8,char_d[frame_cnt++]);
			usleep(10000);
		}
		if(frame_cnt > ch){
			if(ch == 6){
				ch = 8;
				char_d = data_list1;
			}else{
				ch = 6;
				char_d = data_list;
			}
			frame_cnt = 0;
			sleep(5);
		}*/
	}
}
void thread_network_proc(void *argv)
{
	int i = 0;
	
	while(1){
		uds_proc_main();
		usleep(500);
	}
}
unsigned char re_buf[4095];

void thread_usd_get_proc(void *argv)
{
	int i = 0,cnt = 0;
	UDS_N_Services_t pservice;
	unsigned char *res = 0;
	
	while(1){
		do{
		i = UDS_S_service_get(&pservice,re_buf);
		if(N_USDATA_INDICATION == i){
			cnt++;
			res = get_result_char(pservice.USData_Indication.N_Result);
			printf("Got the %d USData.indication:\n",cnt);
			printf("                           Mtype = %d\n",pservice.USData_Indication.Info.Mtype);
			printf("                            N_SA = 0x%2x\n",pservice.USData_Indication.Info.N_SA);
			printf("                            N_TA = 0x%2x\n",pservice.USData_Indication.Info.N_TA);
			printf("                        N_TAtype = %d\n",pservice.USData_Indication.Info.N_TAtype);
			printf("                          Length = %d\n",pservice.USData_Indication.Length);
			printf("                        N_Result = %s\n",res);
			printf("                     MessageData = %s\n\n\r",pservice.USData_Indication.MessageData);
		}
		if(N_USDATA_FF_INDICATION == i){
			cnt++;
			printf("Got the %d USData.FF_indication:\n",cnt);
			printf("                           Mtype = %d\n",pservice.USData_FF_indication.Info.Mtype);
			printf("                            N_SA = 0x%2x\n",pservice.USData_FF_indication.Info.N_SA);
			printf("                            N_TA = 0x%2x\n",pservice.USData_FF_indication.Info.N_TA);
			printf("                        N_TAtype = %d\n",pservice.USData_FF_indication.Info.N_TAtype);
			printf("                          Length = %d\n",pservice.USData_FF_indication.Length);
//			printf("                        N_Result = %d\n",pservice.USData_FF_indication.N_Result);
//			printf("                     MessageData = %s\n\n\r",pservice.USData_FF_indication.MessageData);
		}
		if(N_USDATA_CONFIRM == i){
			cnt++;
			res = get_result_char(pservice.USData_confirm.N_Result);
			printf("Got the %d USData.USData_confirm:\n",cnt);
			printf("                           Mtype = %d\n",pservice.USData_confirm.Info.Mtype);
			printf("                            N_SA = 0x%2x\n",pservice.USData_confirm.Info.N_SA);
			printf("                            N_TA = 0x%2x\n",pservice.USData_confirm.Info.N_TA);
			printf("                        N_TAtype = %d\n",pservice.USData_confirm.Info.N_TAtype);
//			printf("                          Length = %d\n",pservice.USData_confirm.Length);
			printf("                        N_Result = %s\n",res);
//			printf("                     MessageData = %s\n\n\r",pservice.USData_confirm.MessageData);
		}
		
	}while(i < N_TYPE_ERROR);
		sleep(1);
	}
	
}

pthread_t id1,id2,id3,id4;
void thread_start(void)
{
	int ret=0;
	
    ret=pthread_create(&id1,NULL,(void*)thread_time_ctl,NULL);
    if(ret)
    {
        printf("create pthread error!\n");
    }
    ret=pthread_create(&id2,NULL,(void*)thread_network_proc,NULL);
    if(ret)
    {
        printf("create pthread error!\n");
    }
	ret=pthread_create(&id3,NULL,(void*)thread_message_send,NULL);
    if(ret)
    {
        printf("create pthread error!\n");
    }
	ret=pthread_create(&id4,NULL,(void*)thread_usd_get_proc,NULL);
    if(ret)
    {
        printf("create pthread error!\n");
    }
   //pthread_join(id1,NULL);
   //pthread_join(id2,NULL);
}


unsigned char m_can_send_hook(unsigned long id,
							  unsigned char length,
							  unsigned char *data)
{
	unsigned char *res;
	res = get_frame_char(data[0]>>4);
	int i;
	struct timespec ts;
/*
	clock_gettime(CLOCK_REALTIME, &ts);
	printf("Send frame information: %s\n",res);
	printf("                      ID = 0x%x\n",id);
	printf("                    data = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
	printf("                  length = %d\n\r",length);
	printf("                    time = %d\n\r",ts.tv_nsec/1000000);
*/
	if(1 == send_flag){
		send_flag = 2;
	}
	
	i = uds_can_data_put(id,length,data);
	
	return 1;
}


gboolean callback_time_ctl(gpointer argv)
{
	uds_time_handle();
}
gboolean callback_thread_message_send(gpointer argv)
{
	int i = 0;

	i = UDS_N_service_process_USData_request((void *)&usdata_request[0]);
	if(1 != i){
		printf("USData_request failed!\n\r");
	}
}
gboolean callback_thread_network_proc(gpointer argv)
{
	uds_proc_main();
}
gboolean callback_thread_usd_get_proc(gpointer argv)
{
	int i = 0,cnt = 0;
	UDS_N_Services_t pservice;
	unsigned char *res = 0;
	
	do{
		i = UDS_N_service_get(&pservice,re_buf);
		if(N_USDATA_INDICATION == i){
			cnt++;
			res = get_result_char(pservice.USData_Indication.N_Result);
			printf("Got the %d USData.indication:\n",cnt);
			printf("                           Mtype = %d\n",pservice.USData_Indication.Info.Mtype);
			printf("                            N_SA = 0x%2x\n",pservice.USData_Indication.Info.N_SA);
			printf("                            N_TA = 0x%2x\n",pservice.USData_Indication.Info.N_TA);
			printf("                        N_TAtype = %d\n",pservice.USData_Indication.Info.N_TAtype);
			printf("                          Length = %d\n",pservice.USData_Indication.Length);
			printf("                        N_Result = %s\n",res);
			printf("                     MessageData = %s\n\n\r",pservice.USData_Indication.MessageData);
		}
		if(N_USDATA_FF_INDICATION == i){
			cnt++;
			printf("Got the %d USData.FF_indication:\n",cnt);
			printf("                           Mtype = %d\n",pservice.USData_FF_indication.Info.Mtype);
			printf("                            N_SA = 0x%2x\n",pservice.USData_FF_indication.Info.N_SA);
			printf("                            N_TA = 0x%2x\n",pservice.USData_FF_indication.Info.N_TA);
			printf("                        N_TAtype = %d\n",pservice.USData_FF_indication.Info.N_TAtype);
			printf("                          Length = %d\n",pservice.USData_FF_indication.Length);
//			printf("                        N_Result = %d\n",pservice.USData_FF_indication.N_Result);
//			printf("                     MessageData = %s\n\n\r",pservice.USData_FF_indication.MessageData);
		}
		if(N_USDATA_CONFIRM == i){
			cnt++;
			res = get_result_char(pservice.USData_confirm.N_Result);
			printf("Got the %d USData.USData_confirm:\n",cnt);
			printf("                           Mtype = %d\n",pservice.USData_confirm.Info.Mtype);
			printf("                            N_SA = 0x%2x\n",pservice.USData_confirm.Info.N_SA);
			printf("                            N_TA = 0x%2x\n",pservice.USData_confirm.Info.N_TA);
			printf("                        N_TAtype = %d\n",pservice.USData_confirm.Info.N_TAtype);
//			printf("                          Length = %d\n",pservice.USData_confirm.Length);
			printf("                        N_Result = %s\n",res);
//			printf("                     MessageData = %s\n\n\r",pservice.USData_confirm.MessageData);
		}
	}while(i < N_TYPE_ERROR);
}

gboolean callback(gpointer arg)
{
  struct timespec ts;
  if(clock_gettime(CLOCK_REALTIME, &ts) == -1) {
	  g_print("realtime error!\n");
  }else{
	  g_print("test time out. ts.tv_nsec = %d; ts.tv_sec = %d.\n",ts.tv_nsec,ts.tv_sec);
  }
}

GMainLoop* loop;

void timer_test(void)
{
  
  if(g_thread_supported() == 0)
  g_threadl_init(NULL);
  g_print("g_main_loop_new\n");
  loop = g_main_loop_new(NULL, FALSE);
  if(loop){
	  printf("main loop creat ok!\n");
  }
  //g_timeout_add(1,callback_time_ctl,NULL);
 // g_timeout_add(5000,callback_thread_message_send,NULL);
  g_timeout_add(1,callback_thread_network_proc,NULL);
  //g_timeout_add(10,callback_thread_usd_get_proc,NULL);
  g_print("g_main_loop_run\n");
  g_main_loop_run(loop);
  g_print("g_main_loop_unref\n");
  g_main_loop_unref(loop);
}

int main(int argc,char *argv[])
{
	uds_init();
	printf("Init finish,start network.\n\r");
	thread_start();
	//timer_test();
	printf("thread start!\n\r");
	while(1){
	}

	return 0;
}
