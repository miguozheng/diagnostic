#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>



#include "uds.h"
#include "uds_network_layer.h"
#include "uds_session_layer.h"
#include "uds_application_layer.h"


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
pthread_t id1,id2,id3,id4,id5,id_accpet;

unsigned char server_mode = 0;
int server_sockfd = -1;
int client_sockfd = -1;
int s_client_sockfd = -1;
int accpet_r = -1;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

typedef struct {
	unsigned long id;
	unsigned char length;
	unsigned char data[8];
}CAN_frame_t;

typedef union {
	CAN_frame_t frame;
	unsigned char data[sizeof(CAN_frame_t)];
}CAN_frame;


unsigned char sdefault_session[] = {0x10,0x01,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char sd_session[] = {0x11,0x01,0x00,0x00,0x00,0x00,0x00};


UDS_N_USData_Request_t usdata_request[] = 
{

{
	{0x01,0x02,0x03,0x04},
	sdefault_session,
	2
},

{
	{0x01,0x02,0x03,0x04},
	sd_session,
	2
},

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
unsigned char *ch_char[] = 
{
	"N_PARA_CH_STmin",//Send time min
	"N_PARA_CH_BS",		//Block size
	"N_PARA_CH_ERROR"		//type end

};
unsigned char *ch_result_char[] = 
{
	"N_PARA_CHANGE_OK",	//Execution has completed successfully
	"N_PARA_RX_ON",			//The service did not execute since a reception of the message identified by <AI> was taking place
	"N_PARA_WRONG_PARAMETER",	//Undefined <Parameter>
	"N_PARA_WRONG_VALUE"		//Out of range <Parameter_Value>
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

unsigned char *get_ch_char(int res)
{
	return ch_char[res];
}

unsigned char *get_ch_result_char(int res)
{
	return ch_result_char[res];
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
UDS_N_Change_Parameters_Request_t parach = 
{
	{0x01,0x02,0x03,0x04},
	0,
	0xFB
};

void thread_message_send(void *argv)
{
	unsigned int i = 0,frame_cnt = 0,ch = 6,time_cnt = 0;
	
	while(1){
#if 1
		sleep(1);
		//UDS_S_service_process_ParaChange_request((void *)&parach);

		if(frame_cnt >= 2){
			frame_cnt = 0;
		}
		if(frame_cnt < 2){
			i = UDS_S_service_process_USData_request((void *)&usdata_request[frame_cnt++]);
			if(1 != i){
				printf("USData_request failed!\n\r");
			}
		}
		sleep(2);
#endif
	}
}
void thread_network_proc(void *argv)
{
	int i = 0;
	
	while(1){
		//uds_proc_main();
		uds_network_all();
		if(server_mode){
			uds_session_all();
			uds_application_all();
		}
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
			if(0 == server_mode){
				i = UDS_S_service_get(&pservice,re_buf);
			}
//			if(N_USDATA_INDICATION == i){
//				cnt++;
//				res = get_result_char(pservice.USData_Indication.N_Result);
//				printf("Got the %d USData.indication:\n",cnt);
//				printf("                           Mtype = %d\n",pservice.USData_Indication.Info.Mtype);
//				printf("                            N_SA = 0x%2x\n",pservice.USData_Indication.Info.N_SA);
//				printf("                            N_TA = 0x%2x\n",pservice.USData_Indication.Info.N_TA);
//				printf("                        N_TAtype = %d\n",pservice.USData_Indication.Info.N_TAtype);
//				printf("                          Length = %d\n",pservice.USData_Indication.Length);
//				printf("                        N_Result = %s\n",res);
//				printf("                     MessageData = %s\n\n\r",pservice.USData_Indication.MessageData);
//			}
//			if(N_USDATA_FF_INDICATION == i){
//				cnt++;
//				printf("Got the %d USData.FF_indication:\n",cnt);
//				printf("                           Mtype = %d\n",pservice.USData_FF_indication.Info.Mtype);
//				printf("                            N_SA = 0x%2x\n",pservice.USData_FF_indication.Info.N_SA);
//				printf("                            N_TA = 0x%2x\n",pservice.USData_FF_indication.Info.N_TA);
//				printf("                        N_TAtype = %d\n",pservice.USData_FF_indication.Info.N_TAtype);
//				printf("                          Length = %d\n",pservice.USData_FF_indication.Length);
//			}
//			if(N_USDATA_CONFIRM == i){
//				cnt++;
//				res = get_result_char(pservice.USData_confirm.N_Result);
//				printf("Got the %d USData.USData_confirm:\n",cnt);
//				printf("                           Mtype = %d\n",pservice.USData_confirm.Info.Mtype);
//				printf("                            N_SA = 0x%2x\n",pservice.USData_confirm.Info.N_SA);
//				printf("                            N_TA = 0x%2x\n",pservice.USData_confirm.Info.N_TA);
//				printf("                        N_TAtype = %d\n",pservice.USData_confirm.Info.N_TAtype);
//				printf("                        N_Result = %s\n",res);
//			}
//			if(N_CHANGE_PARAMETER_CONFIRM == i){
//				cnt++;
//				res = get_ch_result_char(pservice.Change_parameters_confirm.Result_ChangeParameter);
//				printf("Got the %d N_ChangeParameter.confirm:\n",cnt);
//				printf("                           Mtype = %d\n",pservice.Change_parameters_confirm.Info.Mtype);
//				printf("                            N_SA = 0x%2x\n",pservice.Change_parameters_confirm.Info.N_SA);
//				printf("                            N_TA = 0x%2x\n",pservice.Change_parameters_confirm.Info.N_TA);
//				printf("                        N_TAtype = %d\n",pservice.Change_parameters_confirm.Info.N_TAtype);
//				res = get_ch_char(pservice.Change_parameters_confirm.Parameter);
//				printf("                       Parameter = %s\n",res);
//				res = get_ch_result_char(pservice.Change_parameters_confirm.Result_ChangeParameter);
//				printf("          Result_ChangeParameter = %s\n",res);
//				
//			}
		}while(i < N_TYPE_ERROR);
		sleep(1);
	}
	
}

void thread_server_accept(void *argv)
{
	socklen_t client_len = sizeof(client_addr);
	
	while(1){
		accpet_r = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
		usleep(1000);
	}
}

CAN_frame server_buf;
CAN_frame client_buf;

void thread_server_proc(void *argv)
{
	int i = 0;
	 // 接收连接，创建新的套接字
	while(1){
		if(-1 != accpet_r){
			s_client_sockfd = accpet_r;
			accpet_r = -1;
		}
		if(-1 != s_client_sockfd){
			i = read(s_client_sockfd,server_buf.data,sizeof(CAN_frame));
			if(0 < i){
				printf("\nECU get a message:\n\r");
				printf("                      ID = 0x%x\n",server_buf.frame.id);
				printf("                    data = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",server_buf.frame.data[0],server_buf.frame.data[1],server_buf.frame.data[2],server_buf.frame.data[3],server_buf.frame.data[4],server_buf.frame.data[5],server_buf.frame.data[6],server_buf.frame.data[7]);
				printf("                  length = %d\n\r",server_buf.frame.length);
				uds_can_data_put(server_buf.frame.id,server_buf.frame.length,server_buf.frame.data);
			}
		}
		usleep(2000);
	}
}

void thread_client_proc(void *argv)
{
	int i = 0;
	
	while(1){
		i = read(client_sockfd,client_buf.data,sizeof(CAN_frame));
		if(0 < i){
			printf("\nTester get a message:\n\r");
			printf("                      ID = 0x%x\n",client_buf.frame.id);
			printf("                    data = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",client_buf.frame.data[0],client_buf.frame.data[1],client_buf.frame.data[2],client_buf.frame.data[3],client_buf.frame.data[4],client_buf.frame.data[5],client_buf.frame.data[6],client_buf.frame.data[7]);
			printf("                  length = %d\n\r",client_buf.frame.length);
			uds_can_data_put(client_buf.frame.id,client_buf.frame.length,client_buf.frame.data);
		}
		usleep(2000);
	}
}



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
	ret=pthread_create(&id4,NULL,(void*)thread_usd_get_proc,NULL);
    if(ret)
    {
        printf("create pthread error!\n");
    }
	if(server_mode){
		ret=pthread_create(&id5,NULL,(void*)thread_server_proc,NULL);
		if(ret)
		{
			printf("create pthread error!\n");
		}
		ret=pthread_create(&id_accpet,NULL,(void*)thread_server_accept,NULL);
		if(ret)
		{
			printf("create pthread error!\n");
		}
	}else{
		ret=pthread_create(&id3,NULL,(void*)thread_message_send,NULL);
		if(ret)
		{
			printf("create pthread error!\n");
		}
		ret=pthread_create(&id5,NULL,(void*)thread_client_proc,NULL);
		if(ret)
		{
			printf("create pthread error!\n");
		}
	}
}




unsigned char m_can_send_hook(unsigned long id,
							  unsigned char length,
							  unsigned char *data)
{
	unsigned char *res;
	res = get_frame_char(data[0]>>4);
	int i;
	struct timespec ts;

//	if(server_mode){
//		printf("\n\nECU:\n\r");
//	}else{
//		printf("\n\nTester:\n\r");
//	}

//	clock_gettime(CLOCK_REALTIME, &ts);
//	printf("Send frame information: %s\n",res);
//	printf("                      ID = 0x%x\n",id);
//	printf("                    data = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
//	printf("                  length = %d\n\r",length);
//	printf("                    time = %d\n\r",ts.tv_nsec/1000000);

	if(server_mode){
		server_buf.frame.id = id;
		server_buf.frame.length = length;
		for(i = 0;i < 8;i++){
			server_buf.frame.data[i] = data[i];
		}
		write(s_client_sockfd,server_buf.data,sizeof(CAN_frame));
	}else{
		client_buf.frame.id = id;
		client_buf.frame.length = length;
		for(i = 0;i < 8;i++){
			client_buf.frame.data[i] = data[i];
		}
		write(client_sockfd,&client_buf,sizeof(CAN_frame));
	}

	return 1;
}

int socket_server_init(void)
{
	int ret = -1;
	
	// 创建流套接字
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	ret = server_sockfd;
	if(-1 == server_sockfd){
		printf("Server socket fd creat failed!\n\r");
	}else{
		// 设置服务器接收的连接地址和监听的端口
		server_addr.sin_family = AF_INET;					// 指定网络套接字
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// 接受所有IP地址的连接
		server_addr.sin_port = htons(9755); 				// 绑定到 9736 端口
		
		// 绑定（命名）套接字
		ret = bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if(-1 == ret){
			printf("Server bind failed!\n\r");
		}else{
			// 创建套接字队列，监听套接字
			ret = listen(server_sockfd, 20);
			if(-1 == ret){
				printf("Server listen failed!\n\r");
			}
		}
	}

	return ret;
}

int socket_client_init(char *ip)
{
	int len = 0;
	int ret = -1;

    // 创建流套接字
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
 	if(-1 == client_sockfd){
		printf("Client socket fd creat failed!\n\r");
	}else{
		// 设置要连接的服务器的信息
		client_addr.sin_family = AF_INET;			// 使用网络套接字
		client_addr.sin_addr.s_addr = inet_addr(ip);// 服务器地址
		client_addr.sin_port = htons(9755); 		// 服务器所监听的端口
		len = sizeof(client_addr);
		// 连接到服务器
		ret = connect(client_sockfd, (struct sockaddr *)&client_addr, (socklen_t)len);
		if(-1 == ret){
			printf("Client connect to server failed!\n\r");
		}
	}

	return ret;
}

int main(int argc,char *argv[])
{
	if(1 == argc){
		server_mode = 1;
	}else if(2 == argc){
		server_mode = 0;
	}else{
		return 0;
	}

	if(server_mode){
		if(-1 == socket_server_init()){
			return 0;
		}
	}else{
		if(-1 == socket_client_init(argv[1])){
			return 0;
		}
	}
	uds_init();
	printf("Init finish,start network.\n\r");
	thread_start();
	printf("uds start!\n\r");
	while(1){
	}

	return 0;
}
