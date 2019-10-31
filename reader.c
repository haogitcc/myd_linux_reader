#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#include "app_sys_setting.h"
#include "mid_timer.h"
#include "mid_task.h"
#include "mid_telnet.h"
#include "mid_net.h"
#include "serial_m6e.h"
#include "server_m6e.h"
#include "m6e_init.h"
#include "gpio_init.h"

#define MYD
#ifdef MYD
//MYD demo board
#define DEVICE "/dev/ttymxc1"
#define DEVICE_NAME "tmr:///dev/ttymxc1"
#else
//M28x demo board
#define DEVICE "/dev/ttySP0"
#define DEVICE_NAME "tmr:///dev/ttySP0"
#endif

/*
处理SIGPIPE信号
1、client发送了消息，没等server返回就close了
2、这时候server可以给client发送消息，但是不知道client已断开
3、server就会收到SIG_PIPE这个回复
*/
void handle_pipe(int sig)
{
  //do nothing
  printf("### handle_pipe %d\n", sig);
}

/*
检查或修改与指定信号相关联的处理动作
*/
void *interrupt()
{
  	struct sigaction action;
  	action.sa_handler = handle_pipe;
  	sigemptyset(&action.sa_mask);
  	action.sa_flags = 0;
  	sigaction(SIGPIPE, &action, NULL);
	return NULL;
}

int main(int argc, char **argv)
{
    printf("\n\n MYD ver 1.0 \n\n");
	printf("device=%s\n", DEVICE);
	int ret = -1;
	interrupt();
  	sys_config_init();  
  	sys_config_load(0);
	
	mid_task_init();	
	mid_timer_init();

	mid_connect();

	telnetd_init(1);
	shell_play_init();

	gpio_init();

	ret = m6e_init(DEVICE_NAME);
	if(ret != 0)
	{
		int times = 0;
		while(times <= 3 && ret != 0)
		{
            printf("    re init times= %d, %d\n", ++times, ret);
		    ret = m6e_init(DEVICE_NAME);
			printf("    re init %d\n", ret);
		}
		if(ret != 0 && times == 4)
		{
			printf("m6e_init and restart failed\n");
			return -1;
		}
	}
	else {
	    printf("m6e_init success\n");
		m6e_configuration_init();
		m6e_destory();
	}
	
	if(serial_open(DEVICE) < 0)
    {
        printf("serial_open failed \n");
        return -1;
	}	
	else
	{
		m6e_baudrate(460800);
		serial_flush();
		printf("serial_open success\n");
	}
	pthread_tag_init();
	m6e_start();
	return 0;
}
