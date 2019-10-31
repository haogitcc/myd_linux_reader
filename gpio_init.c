#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include "gpio_init.h"
#include "app_sys_setting.h"

#define MSG(args...) printf(args);
#define MYD
#ifdef MYD
#define GPO_28 44
#define GPO_29 43
#define GPI_30 42
#define GPI_31 41
#else
#define GPO_28 60
#define GPO_29 61
#define GPI_30 62
#define GPI_31 63
#endif

void gpio_init()
{
//28 4
	gpio_export(GPO_28);
	gpio_direction(GPO_28, 1);
	gpio_write(GPO_28, 0);
//29 3
	gpio_export(GPO_29);
	gpio_direction(GPO_29, 1);
	gpio_write(GPO_29, 1);
    MSG("GPO=[%d, %d]\n", GPO_28, GPO_29);
//30 2
	gpio_export(GPI_30);
	gpio_direction(GPI_30, 0);
	//gpio_edge(GPI_30, 1);
//31 1
	gpio_export(GPI_31);
	gpio_direction(GPI_31, 0);
	//gpio_edge(GPI_31, 0);
	MSG("GPI=[%d, %d]\n", GPI_30, GPI_31);
}

/*
暴露GPIO操作接口
*/
int gpio_export(int pin)
{
	char buffer[64];
	int len;
	int fd;

	fd = open("/sys/class/gpio/export" ,O_WRONLY);
	if(fd < 0) {
		MSG("%s Failed to open export for writing!\n", __func__);
		return -1;
	}

	len = snprintf(buffer, sizeof(buffer), "%d", pin);
	if(write(fd, buffer, len) < 0) {
		MSG("%s Failed to export gpio!\n", __func__);
	}

	close(fd);
	return 0;
}

/*
隐藏GPIO操作接口
*/
int gpio_unexport(int pin)
{
	char buffer[64];
	int len;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if(fd < 0) {
		MSG("%s Failed to open unexport for writing!\n", __func__);
		return -1;
	}

	len = snprintf(buffer, sizeof(buffer), "%d", pin);
	if(write(fd, buffer, len) < 0) {
		MSG("%s Failed to unexport gpio!\n", __func__);
	}

	close(fd);
	return 0;
}

/*
配置GPIO方向
pin
dir 0 in, 1 out
*/
int gpio_direction(int pin, int dir)
{
	const char dir_str[] = "in\0out";
	char path[64];
	int fd;

	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction",pin);
	fd = open(path, O_WRONLY);
	if(fd < 0 ){
		MSG("%s Failed to open gpio direction for writing!\n", __func__);
		return -1;
	}

	if(write(fd, &dir_str[dir == 0? 0 : 3], dir == 0? 2 :3) < 0) {
		MSG("%s Failed to set direction!\n", __func__);
		return -1;
	}

	close(fd);
	return 0;
}

/*
制GPIO输出
*/
int gpio_write(int pin, int value)
{
    MSG("gpio_write=[%d,%d]\n", pin, value);
	const char dir_str[] = "01";
	char path[64];
	int fd;

	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value",pin);
	fd = open(path, O_WRONLY);
	if(fd < 0 ){
		MSG("%s Failed to open gpio value for writing!\n", __func__);
		return -1;
	}

	if(write(fd, &dir_str[value == 0? 0 : 1], 1) < 0) {
		MSG("%s Failed to write value!\n", __func__);
		return -1;
	}

	close(fd);
	return 0;
}

/*
获得GPIO输入
*/
int gpio_read(int pin)
{
	char value_str[3];
	char path[64];
	int fd;

	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value",pin);
	fd = open(path, O_RDONLY);
	if(fd < 0 ){
		MSG("%s Failed to open gpio value for writing!\n", __func__);
		return -1;
	}

	if(read(fd, value_str, 3) < 0) {
		MSG("%s Failed to read value!\n", __func__);
		return -1;
	}

	close(fd);
	return atoi(value_str);
}

/*
  none表示引脚为输入，不是中断引脚
  rising表示引脚为中断输入，上升沿触发
  falling表示引脚为中断输入，下降沿触发
  both表示引脚为中断输入，边沿触发
  0-->none, 1-->rising, 2-->falling, 3-->both
*/
int gpio_edge(int pin, int edge)
{
	const char dir_str[] = "none\0rising\0falling\0both";

	int ptr;
	char path[64];
	int fd;

	switch(edge) {
		case 0:
			ptr = 0;
			break;
		case 1:
			ptr = 5;
			break;
		case 2:
			ptr = 12;
			break;
		case 3:
			ptr = 20;
			break;
		default:
			ptr = 0;
	}

	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/edge", pin);
	fd = open(path, O_WRONLY);
	if(fd < 0) {
		MSG("%s Failed to open gpio edge for writing!\n", __func__);
		return -1;
	}

	if(write(fd, &dir_str[ptr], strlen(&dir_str[ptr])) < 0) {
		MSG("%s Failed to set edge!\n", __func__);
		return -1;
	}

	close(fd);
	return 0;
}
