/**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: user_hspi.c
**
** 创   建   人: 张进科
**
** 文件创建日期: 2016 年 11 月 12 日
**
** 描        述: HSPI驱动

** 日志:
2016.11.12  创建本文件
*********************************************************************************************************/

#include "user_hspi.h"
#include "command.h"

#include "osapi.h"
#include "user_interface.h"
#include "driver/spi_interface.h"
#include "gpio.h"
#include "mem.h"

#define start_bit (1 << 7) /* 首包标志位 */
#define end_bit   (1 << 6) /* 末包标志位 */

/* 获取结构体成员偏移宏定义 */
#define OFFSET(Type, member)      ((uint32)&(((struct Type*)0)->member))
#define MEMBER_SIZE(Type, member) sizeof(((struct Type*)0)->member)

os_event_t hspi_recv_Queue[HSPI_RECV_QUEUE_LEN];
os_event_t hspi_send_Queue[HSPI_SEND_QUEUE_LEN];
struct wifi_pack wifi_pack_send;
struct wifi_pack wifi_pack_recv;

/* SPI接收部分变量 */
static uint8  recv_pack[1024*5] = {0}; /* 接收缓冲区 */
static uint32 recv_lenth          = 0; /* 接收到的数据包的长度 */
static uint8  is_recv_pack = 0;

/* SPI发送完成标志位 */
volatile static uint8  wr_rdy     = 1;

void ICACHE_FLASH_ATTR hspi_data_process(uint8 *pack, uint32 lenth);

/*******************************************************************************
* 函数名 	: spi_send_data
* 描述   	: 通过SPI传输指定字节数据
* 输入     	: - pack: 待传输数据 - lenth: 长度
* 输出     	: None
* 返回值    : -1: 失败 0: 成功
*******************************************************************************/
s8 spi_send_data(uint8 *pack, uint32 lenth)
{
	static uint32 i              = 0;
	static uint32 j              = 0;
	volatile static uint32 t     = 0;
	static uint32 num            = 0; /* 包数 */
	static uint32 last_byte      = 0; /* 最后一包有效字节数 */
	static uint8  send_buf[32] = {0}; /* 发送缓冲区 */

	num = lenth / 31 + 1;               /* 计算包数 */
	last_byte = lenth % 31;             /* 计算最后一包有效字节数 */
	if (last_byte == 0)
	{
		num--;
		last_byte = 31;
	}
//	os_printf("lenth: %d, num: %d, last_byte: %d\r\n", lenth, num, last_byte);
	/* 检查参数合法性 */
	if (pack == NULL) {
		os_printf("pack addr is NULL\r\n");
		return -1;
	}
	
	for (i = 0; i < num; i++) {
		send_buf[0] = 0;                         /* 清空command byte */
		
		/* 设置command byte */
		if (i != num - 1) { 
			send_buf[0] = 31;                    /* 如果不为首末包,设置当前包长度 */
		} else {
			send_buf[0] = (end_bit | last_byte); /* 设置当前包长度并添加末包标志位 */
		}
		if (i == 0) {
			send_buf[0] |= start_bit;            /* 添加首包标志位 */
			wr_rdy = 1;
		}
		
		/* 将数据复制到发送buf中 */
		if (i == num - 1) {
			for (j = 0; j < last_byte; j++) {
				send_buf[j + 1] = pack[i * 31 + j]; 
			}
		} else {
			for (j = 0; j < 31; j++) {
				send_buf[j + 1] = pack[i * 31 + j];
			}
		}
		
		/* 等待发送准备好标志位 */
		t = 3000000;
		while((wr_rdy != 1)&&(t != 0)) {
			t--;
		}
		wr_rdy = 0;
		if (t == 0) {
			os_printf("spi send data out time\r\n");
			GPIO_OUTPUT_SET(5, 0); /* GPIO5置0 */
			return -1;             /* 等待超时，返回错误 */
		}
		
		/* 将buf中的数据填充到SPI的寄存器中 */
		SPISlaveSendData(SpiNum_HSPI, (uint32_t *)send_buf, 8);
		GPIO_OUTPUT_SET(5, 1); /* GPIO5置1 */
	}
	
	return 0;
}

/*******************************************************************************
* 函数名 	: spi_slave_isr_sta
* 描述   	: SPI中断服务程序回调函数
* 输入     	: None
* 输出     	: None
* 返回值    : None
*******************************************************************************/
void spi_slave_isr_sta(void *para)
{
	static uint32 regvalue            = 0; /* 保存寄存器值 */
	static uint32 i                   = 0; /* 循环变量 */
	
	/* SPI接收部分变量 */
	static uint32 recv_data 		  = 0;   /* 保存SPI接收寄存器的值 */
	static uint8 buf[32]              = {0}; /* 接收缓冲区 */
	static uint8 isReceive = 0;
	static uint32 pack_counter = 0;          /* 接收数据包计数器 */
	
	if (READ_PERI_REG(0x3ff00020) & BIT4)                    /* SPI中断 */
	{
		
		/* 关闭SPI中断 */
		CLEAR_PERI_REG_MASK(SPI_SLAVE(SpiNum_SPI), 0x3ff);
	} else if (READ_PERI_REG(0x3ff00020)&BIT7) {           /* HSPI中断 */
	
		/* 记录中断类型 */
		regvalue = READ_PERI_REG(SPI_SLAVE(SpiNum_HSPI));
		
		/* 关闭中断使能 */
		SPIIntClear(SpiNum_HSPI);
		
		/* 将HSPI恢复到可通信状态, 准备下一次通信 */
		SET_PERI_REG_MASK(SPI_SLAVE(SpiNum_HSPI), SPI_SYNC_RESET);
		
		/* 清除中断标志位 */
		SPIIntClear(SpiNum_HSPI);
		
		/* 打开中断使能 */
		SPIIntEnable(SpiNum_HSPI, SpiIntSrc_WrStaDone |
                                  SpiIntSrc_RdStaDone |
                                  SpiIntSrc_WrBufDone |
                                  SpiIntSrc_RdBufDone);
		
		/* 主机写入，从机接收处理程序 */
		if (0 != (regvalue & SPI_SLV_WR_BUF_DONE)) {
			
			/* GPIO4清0 */
			GPIO_OUTPUT_SET(4, 0); 
			
			/* 取8次，每次取出一个32位数，共取出32*8=256位，也即32个字节 */
			for (i = 0; i < 8; i++) {
				recv_data=READ_PERI_REG(SPI_W0(SpiNum_HSPI)+(i<<2));
				buf[(i<<2)+0] = (recv_data>>0)&0xff;
				buf[(i<<2)+1] = (recv_data>>8)&0xff;
				buf[(i<<2)+2] = (recv_data>>16)&0xff;
				buf[(i<<2)+3] = (recv_data>>24)&0xff;
			}

			if (is_recv_pack == 0) {
				 
				/* 搜索首包 */
				if (( buf[0] & start_bit) == start_bit) {
					i = 0;
					is_recv_pack = 0;
					isReceive = 1;
					pack_counter = 0;
					recv_lenth = 0;
				}
				
				/* 搜索到首包之后开始接收数据 */
				if (isReceive) {
					recv_lenth += buf[0] & 0x3f;					/* 记录接收到的字节数 */
					if (( buf[0] & end_bit) == end_bit) { 			/* 判断是否收到末包 */
						for (i = 0; i < (buf[0] & 0x3f); i++) {
							recv_pack[pack_counter * 31 + i] = buf[i + 1];
						}
						isReceive = 0;
						is_recv_pack = 1;							/* 收到末包之后停止接收数据并置位接收完成标志 */
					} else {
						for (i = 0; i < 31; i++) {
							recv_pack[pack_counter * 31 + i] = buf[i + 1];
						}
						pack_counter++; 							/* 记录接收到的包数 */
					}
				}
				
				/* 接收完成 */
				if (is_recv_pack == 1) {
					//os_printf("%d\r\n",recv_lenth);
					system_os_post(HSPI_RECV_TASK_PRIO, HSPI_RECV, 0);
				}
			}
			
			GPIO_OUTPUT_SET(4, 1); /* GPIO4置1 */
			SET_PERI_REG_MASK(SPI_SLAVE(SpiNum_HSPI),SPI_SLV_WR_BUF_DONE_EN);
		}
		
		/* 主机读取，从机发送处理程序 */
		if (regvalue & SPI_SLV_RD_BUF_DONE) 
		{
			GPIO_OUTPUT_SET(5, 0); /* GPIO5清0 */
			wr_rdy = 1;
		}
	}
}

/*******************************************************************************
* 函数名 	: hspi_slave_init
* 描述   	: 初始化HSPI
* 输入     	: None
* 输出     	: None
* 返回值    : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_slave_init()
{
	// SPI initialization configuration, speed = 0 in slave mode
	SpiAttr hSpiAttr;
	hSpiAttr.bitOrder = SpiBitOrder_MSBFirst;
	hSpiAttr.speed = 0;
	hSpiAttr.mode = SpiMode_Slave;
	hSpiAttr.subMode = SpiSubMode_0;
	// Init HSPI GPIO
	// Configure MUX to allow HSPI
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);//GPIO4主机发送从机接收缓存状态
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);//GPIO5主机接收从机发送缓存状态

	GPIO_OUTPUT_SET(4, 0);	//GPIO4清0
	GPIO_OUTPUT_SET(5, 0);	//GPIO5清0
	os_printf("\r\n============= spi init slave =============\r\n");
	SPIInit(SpiNum_HSPI, &hSpiAttr);
	// Set spi interrupt information.
	SpiIntInfo spiInt;
	spiInt.src = (SpiIntSrc_TransDone
	|SpiIntSrc_WrStaDone
	|SpiIntSrc_RdStaDone
	|SpiIntSrc_WrBufDone
	|SpiIntSrc_RdBufDone);
	spiInt.isrFunc = spi_slave_isr_sta;
	SPIIntCfg(SpiNum_HSPI, &spiInt);
	// SHOWSPIREG(SpiNum_HSPI);
	SPISlaveRecvData(SpiNum_HSPI);
	// set the value of status register
	WRITE_PERI_REG(SPI_RD_STATUS(SpiNum_HSPI), 0x8A);
	WRITE_PERI_REG(SPI_WR_STATUS(SpiNum_HSPI), 0x83);
	GPIO_OUTPUT_SET(4, 1);	//GPIO4置1,表示从机准备好，主机可以发送数据到从机
}

/*******************************************************************************
* 函数名 	: hspi_send_task
* 描述   	: hspi发送任务
* 输入     	: - e: 事件
* 输出     	: None
* 返回值    : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_send_task(os_event_t *e)
{
    switch(e->sig)
	{
	    case HSPI_SEND:
		{
			wifi_send(wifi_pack_send.cmd, wifi_pack_send.lenth, wifi_pack_send.data);
		} break;
        default: break;
    }
}

/*******************************************************************************
* 函数名 	: hspi_recv_task
* 描述   	: hspi接收任务
* 输入     	: - e: 事件
* 输出     	: None
* 返回值    : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_recv_task(os_event_t *e)
{
    switch(e->sig)
	{
	    case HSPI_RECV:
		{
			hspi_data_process(recv_pack,recv_lenth);
			is_recv_pack = 0;
		} break;
        default: break;
    }
}

/*******************************************************************************
* 函数名 	: hspi_data_process
* 描述   	: hspi数据处理
* 输入     	: - pack: 接收到的数据 - lenth: 数据大小
* 输出     	: None
* 返回值     : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_data_process(uint8 *pack, uint32 lenth)
{
	static uint32 par_lenth = sizeof(struct wifi_pack) - MEMBER_SIZE(wifi_pack, data); /* 包中的参数大小 */
	static uint32 crc_lenth = MEMBER_SIZE(wifi_pack, crc); /* CRC大小 */
	static uint32 i = 0;

	os_memcpy(&wifi_pack_recv, recv_pack, par_lenth);
	
	/* 校验包长度 */
	if (wifi_pack_recv.lenth + par_lenth != recv_lenth) {
		is_recv_pack = 0; /* 重新等待接收数据 */
		os_printf("lenth verify failed\r\n");
		return;
	}
	
	/* CRC校验 */
	if (wifi_pack_recv.crc != 
        CRC32Software(recv_pack + crc_lenth, par_lenth - crc_lenth + wifi_pack_recv.lenth)) {
		is_recv_pack = 0; /* 重新等待接收数据 */
		os_printf("crc verify failed %08X\r\n", wifi_pack_recv.crc);
		return;
	}
	
	wifi_pack_recv.data = recv_pack + par_lenth;
	command_execute();
#if 0
	wifi_send(0x09, wifi_pack_recv.lenth, wifi_pack_recv.data);
	os_printf("cmd: %d, lenth: %d, crc: %08X\r\n", wifi_pack_recv.cmd, wifi_pack_recv.lenth, wifi_pack_recv.crc);
	
	for (i = 0; i < wifi_pack_recv.lenth; i++)
	{
		os_printf("%02X ", *(wifi_pack_recv.data + i));
	}
		os_printf("\r\n");
#endif
	is_recv_pack = 0; /* 重新等待接收数据 */
}

s8 wifi_send(u8 cmd, u16 data_lenth, u8 *data)
{
	struct wifi_pack wifi_pack_send;
	u8 *send_pack = NULL;
	u32 par_lenth = sizeof(struct wifi_pack) - MEMBER_SIZE(wifi_pack, data); /* 包中的参数大小 */
	u32 crc_lenth = MEMBER_SIZE(wifi_pack, crc); /* CRC大小 */
	
	/* 申请内存 */
	send_pack = (u8 *)os_malloc(data_lenth + par_lenth);
    if (!send_pack) 
    { 
        os_printf("send_pack memory failed\r\n");
        return -1;
    }

	wifi_pack_send.cmd = cmd;                /* 命令 */
	wifi_pack_send.lenth = data_lenth;       /* 数据长度 */
	
	/* 将除CRC之外的其它参数拷贝到缓冲区 */
	os_memcpy(send_pack + crc_lenth, (u8 *)&wifi_pack_send + crc_lenth, par_lenth - crc_lenth);
	
	/* 将数据拷贝到缓冲区 */
	os_memcpy(send_pack + par_lenth, data, data_lenth);
	
	/* 计算CRC */
	wifi_pack_send.crc = CRC32Software(send_pack + crc_lenth, par_lenth - crc_lenth + data_lenth);
	
	/* 将CRC拷贝到缓冲区 */
	os_memcpy(send_pack, &wifi_pack_send, crc_lenth);
	
	/* 发送数据 */
	if (spi_send_data(send_pack, par_lenth + data_lenth) != 0)
	{
		os_free(send_pack);
		return -1;
	}
	
	os_free(send_pack);
	return 0;
}

