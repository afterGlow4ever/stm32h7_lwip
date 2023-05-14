//===============================================
//
//	File: eth.h
//	Author: afterGlow,4ever
//	Date: 04282023
//	Version: v1.0
//
// 	This file is including eth define structure.
//
//===============================================

#ifndef _ETH_H_
#define	_ETH_H_

#include "eth_param.h"
#include "eth_bsp.h"

#include "LAN8720a.h"

#include "sys.h"
#include "stdio.h"
#include "string.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "lwip/arch.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sio.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"

#include "debug_printf.h"

#include "design_define.h"

//===============================================
// network basic enum and structure
//===============================================

enum tcp_client_states
{
	TCP_CLIENT_NONE = 0,
	TCP_CLIENT_CONNECTED,
	TCP_CLIENT_CLOSING,
};

struct tcp_client_struct
{
	uint8_t state;
	struct tcp_pcb *pcb;
	struct pbuf *p;
};

enum tcp_server_states
{
	TCP_SERVER_NONE = 0,
	TCP_SERVER_ACCEPT,
	TCP_SERVER_CLOSING,
};

struct tcp_server_struct
{
	uint8_t state;
	struct tcp_pcb *pcb;
	struct pbuf *p;
};

enum udp_normal_states
{
	UDP_NORMAL_NONE = 0,
	UDP_NORMAL_CONNECTED,
	UDP_NORMAL_CLOSING,
};

struct udp_normal_struct
{
	uint8_t state;
	struct udp_pcb *pcb;
	struct pbuf *p;
};

extern struct netif gnetif;

//===============================================
// network basic define
//===============================================

#define TCP_CLIENT_RX_BUFFER_SIZE 1500
#define TCP_CLIENT_TX_BUFFER_SIZE 200

#define TCP_SERVER_RX_BUFFER_SIZE 1500
#define TCP_SERVER_TX_BUFFER_SIZE 200

#define UDP_RX_BUFFER_SIZE 1500
#define UDP_TX_BUFFER_SIZE 200

#define TCP_CLIENT_RX_RECV_SIZE 1500
#define TCP_SERVER_RX_RECV_SIZE 1500
#define UDP_RX_RECV_SIZE 1500

#define ETH_RX_BUFFER_SIZE                     (1536UL)
#define ETH_DMA_TRANSMIT_TIMEOUT                (5U)

#define NETIF_MTU								      ( 1500 )
#define NETIF_IN_TASK_STACK_SIZE			( 2048 )
#define NETIF_IN_TASK_PRIORITY			  ( 4 )
#define NETIF_OUT_TASK_STACK_SIZE			( 2048 )
#define NETIF_OUT_TASK_PRIORITY			  ( 4 )

//===============================================
// network function head
//===============================================

// lan8720a
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth);
HAL_StatusTypeDef LAN8720_Init(ETH_HandleTypeDef *heth);
uint32_t LAN8720_GetLinkState(ETH_HandleTypeDef *heth);
void ETH_IRQHandler(void);
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth);
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth);
void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth);

// netif
void Netif_Config(void);
void User_notification(struct netif *netif);

// network communication
#ifdef STM32_TCP_CLIENT_NETCONN
void tcp_client_loop(void);
void tcp_client_init(void);
#endif

#ifdef STM32_TCP_SERVER_NETCONN
void tcp_server_loop(void);
void tcp_server_init(void);
#endif

#ifdef STM32_UDP_NETCONN
void udp_normal_loop(void);
void udp_normal_init(void);
#endif

#ifdef STM32_TCP_CLIENT_SOCKET
void tcp_client_loop(void);
void tcp_client_init(void);
#endif

#ifdef STM32_TCP_SERVER_SOCKET
void tcp_server_loop(void);
void tcp_server_init(void);
#endif

#ifdef STM32_UDP_VC_SOCKET
void udp_virtual_connect_loop(void);
void udp_virtual_connect_init(void);
#endif

#ifdef STM32_UDP_SOCKET
void udp_normal_loop(void);
void udp_normal_init(void);
#endif

// ethernet interface
u32_t sys_now(void);
static void arp_timer(void *arg);
void pbuf_free_custom(struct pbuf *p);
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void *pParams);
void ethernet_link_check_state(struct netif *netif);

#endif
