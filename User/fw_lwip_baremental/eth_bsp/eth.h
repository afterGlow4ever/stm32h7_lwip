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
#include "LAN8720a.h"
#include "eth_bsp.h"
#include "sys.h"
#include "stdio.h"
#include "string.h"
#include "lwip/err.h"
#include "lwip/netif.h"
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
#ifdef STM32_TCP_CLIENT_RAW
void tcp_client_loop(void);
void tcp_client_main(void);
err_t tcp_client_connect(void *arg, struct tcp_pcb *pcb, err_t err);
err_t tcp_client_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
err_t tcp_client_transmit(void *arg, struct tcp_pcb *pcb, uint16_t length);
void tcp_client_transmit_data(struct tcp_pcb *pcb, struct tcp_client_struct *temp);
err_t tcp_client_user_transmit(struct tcp_pcb *pcb);
void tcp_client_error(void *arg, err_t err);
err_t tcp_client_poll(void *arg, struct tcp_pcb *pcb);
void tcp_client_connect_close(struct tcp_pcb *pcb, struct tcp_client_struct *temp);
#endif

#ifdef STM32_TCP_SERVER_RAW
void tcp_server_loop(void);
void tcp_server_main(void);
err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
err_t tcp_server_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
err_t tcp_server_transmit(void *arg, struct tcp_pcb *pcb, uint16_t length);
void tcp_server_transmit_data(struct tcp_pcb *pcb, struct tcp_server_struct *temp);
err_t tcp_server_user_transmit(struct tcp_pcb *pcb);
void tcp_server_error(void *arg, err_t err);
err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb);
void tcp_server_connect_close(struct tcp_pcb *pcb, struct tcp_server_struct *temp);
#endif

#ifdef STM32_UDP_RAW
void udp_loop(void);
void udp_main(void);
void udp_normal_receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);
err_t udp_normal_user_transmit(struct udp_pcb *pcb);
void udp_normal_connect_close(struct udp_pcb *pcb);
#endif

// ethernet interface
u32_t sys_now(void);
static void arp_timer(void *arg);
void pbuf_free_custom(struct pbuf *p);
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);
void ethernet_link_check_state(struct netif *netif);
void lwip_periodic(void);
void lwip_pkt_handle(void);

#endif
