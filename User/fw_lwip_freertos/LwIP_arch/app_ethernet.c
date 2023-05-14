//===============================================
//
//	File: app_ethernet.c
//	Author: afterGlow,4ever
//	Date: 04282023
//	Version: v1.0
//
// 	This file is including netif and communication 
//	applications.
//
//===============================================

#include "lwip/opt.h"
#include "netif/ethernet.h"
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"

#include "eth.h"

extern int Key_Value;

//===============================================
// netif config
//===============================================

struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];

void Netif_Config(void)
{
  tcpip_init(NULL, NULL);
  
  /* IP addresses initialization */
#if LWIP_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  IP4_ADDR(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP4_ADDR(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP4_ADDR(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif 
  /* Initilialize the LwIP stack without RTOS */
  /* add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
    printf("Net interface set up.\r\n");		
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
    printf("Net interface set down.\r\n");		
  }
}

//===============================================
// netif notification
//===============================================

void User_notification(struct netif *netif) 
{
  if (netif_is_up(netif))
  {
    printf("Static IP: %d.%d.%d.%d\r\n",IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
    printf("NETMASK  : %d.%d.%d.%d\r\n",NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
    printf("Gateway  : %d.%d.%d.%d\r\n",GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
  }
  else
  {
    printf ("The network cable is not connected \n");
  }
}

//===============================================
// tcp client netconn
//===============================================

#ifdef STM32_TCP_CLIENT_NETCONN
struct netconn *client_netconn = NULL;
uint8_t tcp_client_rx_buffer[TCP_CLIENT_RX_BUFFER_SIZE];
uint8_t *tcp_client_tx_buffer = "TCP Client test.\r\n";
//bit1: connected
//bit2: data received
//bit3: data processed
//bit4: data transmitting
uint8_t tcp_client_status;

void tcp_client_loop(void)
{
	if(Key_Value == 1)
	{
		tcp_client_status |= 1<<4;
		Key_Value = 4;
	}
	vTaskDelay(10);
}

static void tcp_client_netconn(void *thread_param)
{
  ip4_addr_t ipaddr;
	uint32_t length = 0;
	struct pbuf *q;
	err_t err, err2;
	struct netbuf *recvbuf;
  
	printf("Test for TCP client netconn . \r\n");
  printf("Dest IP is : %d.%d.%d.%d \t port is : %d\r\n",      \
          DEST_IP_ADDR0,DEST_IP_ADDR1,DEST_IP_ADDR2,DEST_IP_ADDR3,DEST_PORT);
  
  IP4_ADDR(&ipaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3);
  vTaskDelay(500);

	while(1)
  {
    client_netconn = netconn_new(NETCONN_TCP);
    if (client_netconn == NULL)
    {
      printf("Create netconn failed!\r\n");
      vTaskDelay(10);
      continue;
    }
// 		err = netconn_bind(client_netconn, &ipaddr, LOCAL_PORT);
//		if(err != ERR_OK)
//		{
//			printf("TCP Client failed to establish!\r\n");
//			vTaskDelay(10);
//      continue;
//		}   
    err = netconn_connect(client_netconn, &ipaddr, DEST_PORT);
    if (err != ERR_OK)
    {
        printf("TCP Client connect failed!\r\n");
        netconn_close(client_netconn);
				netconn_delete(client_netconn);
        vTaskDelay(500);
        continue;
    }
		else if(err == ERR_OK)
		{
	    printf("Connect to TCP Server successful!\r\n");
	
			client_netconn->recv_timeout = 100;
	
			while(1)
			{
				if(tcp_client_status & 1<<4)
				{	
					err = netconn_write(client_netconn, tcp_client_tx_buffer, strlen((char *)tcp_client_tx_buffer), NETCONN_COPY); 
					if(err != ERR_OK)
						printf("TCP Client transmit failed.\r\n");
					printf("TCP Client is transmitting data: %s\r\n", tcp_client_tx_buffer);
					tcp_client_status &= ~(1<<4);
				}
	
				if((err2 = netconn_recv(client_netconn, &recvbuf)) == ERR_OK)
				{
					uint32_t temp;
					taskENTER_CRITICAL();				
					memset(tcp_client_rx_buffer, 0, TCP_CLIENT_RX_BUFFER_SIZE);
		    	for(q = recvbuf->p; q != NULL; q = q->next)
					{
						if(q->len > (TCP_CLIENT_RX_BUFFER_SIZE - length))
							memcpy(tcp_client_rx_buffer + length, q->payload, (TCP_CLIENT_RX_BUFFER_SIZE - length));
						else
							memcpy(tcp_client_rx_buffer + length, q->payload, q->len);
						length += q->len;
						if(length > TCP_CLIENT_RX_BUFFER_SIZE)
						{
							printf("TCP Client rx buffer overflow.\r\n");
							break;
						}
					}
					taskEXIT_CRITICAL();  
					length = 0;
					printf("TCP Client has received data: %s\r\n", tcp_client_rx_buffer);
					netbuf_delete(recvbuf);
				}
				else if(err2 == ERR_CLSD)
				{
					netconn_close(client_netconn);
					netconn_delete(client_netconn);
					printf("TCP Client has disconnected from TCP server.\r\n");
					break;
				}
	
			}
		}
  }
}

void tcp_client_init(void)
{
  sys_thread_new("tcp_client_netconn", tcp_client_netconn, NULL, 512, 4);
}

#endif

//===============================================
// tcp server netconn
//===============================================

#ifdef STM32_TCP_SERVER_NETCONN
struct netconn *server_netconn = NULL;
struct netconn *server_listen_netconn = NULL;
uint8_t tcp_server_rx_buffer[TCP_SERVER_RX_BUFFER_SIZE];
uint8_t *tcp_server_tx_buffer = "TCP Server test.\r\n";
//bit1: accepted
//bit2: data received
//bit3: data processed
//bit4: data transmitting
uint8_t tcp_server_status;
uint32_t client_ip;

void tcp_server_loop(void)
{
	if(Key_Value == 1)
	{
		tcp_server_status |= 1<<4;
		Key_Value = 4;
	}
	vTaskDelay(10);
}

static void tcp_server_netconn(void *thread_param)
{
  ip4_addr_t ipaddr;
	uint16_t port;
	uint32_t length = 0;
	struct pbuf *q;
	err_t err, err2;
	struct netbuf *recvbuf;
  
	printf("Test for TCP server netconn . \r\n");
  
  vTaskDelay(500);

	while(1)
  {
    server_netconn = netconn_new(NETCONN_TCP);
    if (server_netconn == NULL)
    {
      printf("Create netconn failed!\r\n");
      vTaskDelay(10);
      continue;
    }
    printf("TCP Server first start establishing!\r\n");  
		err = netconn_bind(server_netconn, IP_ADDR_ANY, LOCAL_PORT);
		if(err != ERR_OK)
		{
			printf("TCP Server failed to establish!\r\n");
			vTaskDelay(10);
      continue;
		}	
		printf("TCP Server established!\r\n");
		printf("TCP Server enter listen!\r\n");
		netconn_listen(server_netconn);
		server_netconn->recv_timeout = 10;

		while(1)
		{
	    err = netconn_accept(server_netconn, &server_listen_netconn);

			if(err == ERR_OK)
			{
				server_listen_netconn->recv_timeout = 10;
				netconn_getaddr(server_listen_netconn, &ipaddr, &port, 0);
				printf("TCP Server accept ok!\r\n");
				printf("TCP Server accept from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
		
				while(1)
				{
					if(tcp_server_status & 1<<4)
					{	
						err = netconn_write(server_listen_netconn, tcp_server_tx_buffer, strlen((char *)tcp_server_tx_buffer), NETCONN_COPY); 
						if(err != ERR_OK)
							printf("TCP Server transmit failed.\r\n");
						printf("TCP Server is transmitting data: %s\r\n", tcp_server_tx_buffer);
						tcp_server_status &= ~(1<<4);
					}
		
					if((err2 = netconn_recv(server_listen_netconn, &recvbuf)) == ERR_OK)
					{
						uint32_t temp;
						taskENTER_CRITICAL();				
						memset(tcp_server_rx_buffer, 0, TCP_SERVER_RX_BUFFER_SIZE);
			    	for(q = recvbuf->p; q != NULL; q = q->next)
						{
							if(q->len > (TCP_SERVER_RX_BUFFER_SIZE - length))
								memcpy(tcp_server_rx_buffer + length, q->payload, (TCP_SERVER_RX_BUFFER_SIZE - length));
							else
								memcpy(tcp_server_rx_buffer + length, q->payload, q->len);
							length += q->len;
							if(length > TCP_SERVER_RX_BUFFER_SIZE)
							{
								printf("TCP Server rx buffer overflow.\r\n");
								break;
							}
						}
						taskEXIT_CRITICAL();  
						length = 0;
						printf("TCP Server has received data: %s\r\n", tcp_server_rx_buffer);
						printf("TCP Server has received from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
						netbuf_delete(recvbuf);
					}
					else if((err2 == ERR_CLSD) || (err2 == ERR_CONN) || (err2 == ERR_RST))
					{
						netconn_close(server_listen_netconn);
						netconn_delete(server_listen_netconn);
						printf("TCP Server has disconnected from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
						break;
					}
		
				}
			}
		}
  }
}

void tcp_server_init(void)
{
  sys_thread_new("tcp_server_netconn", tcp_server_netconn, NULL, 512, 4);
}

#endif

//===============================================
// udp netconn
//===============================================

#ifdef STM32_UDP_NETCONN
struct netconn *udp_netconn = NULL;
uint8_t udp_rx_buffer[UDP_RX_BUFFER_SIZE];
uint8_t *udp_tx_buffer = "UDP test.\r\n";
//bit1: accepted
//bit2: data received
//bit3: data processed
//bit4: data transmitting
uint8_t udp_normal_status;
uint32_t remote_ip;

void udp_normal_loop(void)
{
	if(Key_Value == 1)
	{
		udp_normal_status |= 1<<4;
		Key_Value = 4;
	}
	vTaskDelay(10);
}

static void udp_normal_netconn(void *thread_param)
{
  ip4_addr_t ipaddr;
	uint16_t port;
	uint32_t length = 0;
	struct pbuf *q;
	err_t err, err2;
	struct netbuf *recvbuf;
	struct netbuf *sentbuf;
  
	printf("Test for UDP netconn . \r\n");
  printf("Dest IP is : %d.%d.%d.%d \t port is : %d\r\n",      \
          DEST_IP_ADDR0,DEST_IP_ADDR1,DEST_IP_ADDR2,DEST_IP_ADDR3,DEST_PORT);
  
  IP4_ADDR(&ipaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3);
  
  vTaskDelay(500);

	while(1)
  {
    udp_netconn = netconn_new(NETCONN_UDP);
    if (udp_netconn == NULL)
    {
      printf("Create netconn failed!\r\n");
      vTaskDelay(10);
      continue;
    }
    printf("UDP first start binding!\r\n");  
		err = netconn_bind(udp_netconn, IP_ADDR_ANY, DEST_PORT);
		if(err != ERR_OK)
		{
			printf("UDP failed to bind!\r\n");
			vTaskDelay(10);
      continue;
		}	
		printf("UDP binded!\r\n");
		printf("UDP first start connecting!\r\n");  
		err = netconn_connect(udp_netconn, &ipaddr, DEST_PORT);
		if(err != ERR_OK)
		{
			printf("UDP failed to connect!\r\n");
			vTaskDelay(10);
      continue;
		}	
//		printf("UDP connected!\r\n");

		udp_netconn->recv_timeout = 10;

		while(1)
		{
			if(udp_normal_status & 1<<4)
			{	
				sentbuf = netbuf_new();
				netbuf_alloc(sentbuf, strlen((char *)udp_tx_buffer));
				memcpy(sentbuf->p->payload, (void *)udp_tx_buffer, strlen((char *)udp_tx_buffer));
				err = netconn_send(udp_netconn, sentbuf); 
				if(err != ERR_OK)
					printf("UDP transmit failed.\r\n");
				printf("UDP is transmitting data: %s\r\n", udp_tx_buffer);
				udp_normal_status &= ~(1<<4);
				netbuf_delete(sentbuf);
			}

			if((err2 = netconn_recv(udp_netconn, &recvbuf)) == ERR_OK)
			{
				uint32_t temp;
				taskENTER_CRITICAL();				
				memset(udp_rx_buffer, 0, UDP_RX_BUFFER_SIZE);
	    	for(q = recvbuf->p; q != NULL; q = q->next)
				{
					if(q->len > (UDP_RX_BUFFER_SIZE - length))
						memcpy(udp_rx_buffer + length, q->payload, (UDP_RX_BUFFER_SIZE - length));
					else
						memcpy(udp_rx_buffer + length, q->payload, q->len);
					length += q->len;
					if(length > UDP_RX_BUFFER_SIZE)
					{
						printf("UDP rx buffer overflow.\r\n");
						break;
					}
				}
				taskEXIT_CRITICAL();  
				length = 0;
				printf("UDP has received data: %s\r\n", udp_rx_buffer);
				printf("UDP has received from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, DEST_PORT);
				netbuf_delete(recvbuf);
			}
			else if((err2 == ERR_CLSD) || (err2 == ERR_CONN) || (err2 == ERR_RST))
			{
				netconn_close(udp_netconn);
				netconn_delete(udp_netconn);
				printf("UDP has disconnected from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
				break;
			}
		}
  }
}

void udp_normal_init(void)
{
  sys_thread_new("udp_netconn", udp_normal_netconn, NULL, 512, 4);
}

#endif

//===============================================
// tcp client socket
//===============================================

#ifdef STM32_TCP_CLIENT_SOCKET
uint8_t tcp_client_rx_buffer[TCP_CLIENT_RX_BUFFER_SIZE];
uint8_t *tcp_client_tx_buffer = "TCP Client test.\r\n";
//bit1: connected
//bit2: data received
//bit3: data processed
//bit4: data transmitting
uint8_t tcp_client_status;

void tcp_client_loop(void)
{
	if(Key_Value == 1)
	{
		tcp_client_status |= 1<<4;
		Key_Value = 4;
	}
	vTaskDelay(10);
}

static void tcp_client_socket(void *thread_param)
{
	int client_sock = -1;
  ip4_addr_t ipaddr;
  struct sockaddr_in client_addr;
	uint32_t length = 0;
	struct pbuf *q;
	int err, ret;
	struct netbuf *recvbuf;
	struct timeval client_timeout;
  
	printf("Test for TCP client socket. \r\n");
  printf("Dest IP is : %d.%d.%d.%d \t port is : %d\r\n",      \
          DEST_IP_ADDR0,DEST_IP_ADDR1,DEST_IP_ADDR2,DEST_IP_ADDR3,DEST_PORT);
  
  IP4_ADDR(&ipaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3); 
  client_addr.sin_family = AF_INET;      
  client_addr.sin_port = htons(DEST_PORT);   
  client_addr.sin_addr.s_addr = ipaddr.addr;
  memset(&(client_addr.sin_zero), 0, sizeof(client_addr.sin_zero));    

  vTaskDelay(500);

	while(1)
  {
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0)
    {
      printf("Create socket failed!\r\n");
      vTaskDelay(10);
      continue;
    }

    err = connect(client_sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
    if (err != 0)
    {
        printf("TCP Client connect failed!\r\n");
        close(client_sock);
        vTaskDelay(500);
        continue;
    }
		else if(err == 0)
		{
	    printf("Connect to TCP Server successful!\r\n");
	
			client_timeout.tv_sec = 0;
			client_timeout.tv_usec = 100000;
			setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &client_timeout, sizeof(client_timeout));

	    printf("Set timeout for receiving data!\r\n");
			while(1)
			{
				if(tcp_client_status & 1<<4)
				{	
					ret = write(client_sock, tcp_client_tx_buffer, (int)strlen((char *)tcp_client_tx_buffer));
					if(ret <= 0)
						printf("TCP Client transmit failed.\r\n");
					else
					{
						printf("TCP Client is transmitting data: %s\r\n", tcp_client_tx_buffer);
						tcp_client_status &= ~(1<<4);
					}
				}
				memset(tcp_client_rx_buffer, 0, TCP_CLIENT_RX_RECV_SIZE);
				ret = recv(client_sock, &tcp_client_rx_buffer, TCP_CLIENT_RX_RECV_SIZE, 0);
				if(ret > 0)
				{
					printf("TCP Client has received data: %s\r\n", tcp_client_rx_buffer);
					memset(tcp_client_rx_buffer, 0, TCP_CLIENT_RX_RECV_SIZE);
				}
				else if(ret == 0)
				{
					close(client_sock);
					printf("TCP Client has disconnected from TCP server.\r\n");
					break;
				}
	
			}
		}
  }
}

void tcp_client_init(void)
{
  sys_thread_new("tcp_client_socket", tcp_client_socket, NULL, 512, 4);
}

#endif

//===============================================
// tcp server socket
//===============================================

#ifdef STM32_TCP_SERVER_SOCKET
uint8_t tcp_server_rx_buffer[TCP_SERVER_RX_BUFFER_SIZE];
uint8_t *tcp_server_tx_buffer = "TCP Server test.\r\n";
//bit1: connected
//bit2: data received
//bit3: data processed
//bit4: data transmitting
uint8_t tcp_server_status;

void tcp_server_loop(void)
{
	if(Key_Value == 1)
	{
		tcp_server_status |= 1<<4;
		Key_Value = 4;
	}
	vTaskDelay(10);
}

static void tcp_server_socket(void *thread_param)
{
	int server_sock = -1, client_sock;
  ip4_addr_t ipaddr;
  struct sockaddr_in server_addr, client_addr;
	uint16_t port;
	uint32_t length = 0;
	struct pbuf *q;
	int err, ret;
	socklen_t sin_size;
	struct netbuf *recvbuf;
	struct timeval server_timeout, client_timeout;
  
	printf("Test for TCP server socket. \r\n");

  server_addr.sin_family = AF_INET;      
  server_addr.sin_port = htons(LOCAL_PORT);   
  server_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));    

  vTaskDelay(500);

	while(1)
  {
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
      printf("Create socket failed!\r\n");
      vTaskDelay(10);
      continue;
    }

    ret = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (ret == -1)
    {
				printf("TCP Server failed to establish!\r\n");
        close(server_sock);
        vTaskDelay(500);
        continue;
    }
		else
		{
			printf("TCP Server established!\r\n");
			printf("TCP Server enter listen!\r\n");

			listen(server_sock, 5);
			server_timeout.tv_sec = 0;
			server_timeout.tv_usec = 100000;
			setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, &server_timeout, sizeof(server_timeout));
			
	    printf("Set timeout for receiving data!\r\n");
			while(1)
			{
				client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
				if(client_sock != -1)
				{
					client_timeout.tv_sec = 0;
					client_timeout.tv_usec = 100000;
					setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &client_timeout, sizeof(client_timeout));
						
  				ipaddr.addr = client_addr.sin_addr.s_addr;
					port = client_addr.sin_port;
					printf("TCP Server accept ok!\r\n");
					printf("TCP Server accept from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
	
					while(1)
					{
						if(tcp_server_status & 1<<4)
						{	
							ret = write(client_sock, tcp_server_tx_buffer, (int)strlen((char *)tcp_server_tx_buffer));
							if(ret <= 0)
								printf("TCP Server transmit failed.\r\n");
							else
							{
								printf("TCP Server is transmitting data: %s\r\n", tcp_server_tx_buffer);
								tcp_server_status &= ~(1<<4);
							}
						}
						memset(tcp_server_rx_buffer, 0, TCP_SERVER_RX_RECV_SIZE);
						ret = recv(client_sock, &tcp_server_rx_buffer, TCP_SERVER_RX_RECV_SIZE, 0);
						if(ret > 0)
						{
							printf("TCP Server has received data: %s\r\n", tcp_server_rx_buffer);
							memset(tcp_server_rx_buffer, 0, TCP_SERVER_RX_RECV_SIZE);
						}
						else if(ret == 0)
						{
							close(client_sock);
							printf("TCP Server has disconnected from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
							break;
						}
					}
				}
			}
		}
  }
}

void tcp_server_init(void)
{
  sys_thread_new("tcp_server_socket", tcp_server_socket, NULL, 512, 4);
}

#endif

//===============================================
// udp virtual connection socket
//===============================================

#ifdef STM32_UDP_VC_SOCKET
uint8_t udp_rx_buffer[UDP_RX_BUFFER_SIZE];
uint8_t *udp_tx_buffer = "UDP virtual connection test.\r\n";
//bit1: accepted
//bit2: data received
//bit3: data processed
//bit4: data transmitting
uint8_t udp_virtual_connect_status;

void udp_virtual_connect_loop(void)
{
	if(Key_Value == 1)
	{
		udp_virtual_connect_status |= 1<<4;
		Key_Value = 4;
	}
	vTaskDelay(10);
}

static void udp_virtual_connect_socket(void *thread_param)
{
	int udp_sock = -1;
  ip4_addr_t ipaddr;
  struct sockaddr_in local_addr, remote_addr;
	uint16_t port;
	uint32_t length = 0;
	struct pbuf *q;
	int err, ret;
	socklen_t sin_size;
	struct netbuf *recvbuf;
	struct timeval udp_timeout;
  
	printf("Test for UDP socket. \r\n");
  printf("Dest IP is : %d.%d.%d.%d \t port is : %d\r\n",      \
          DEST_IP_ADDR0,DEST_IP_ADDR1,DEST_IP_ADDR2,DEST_IP_ADDR3,DEST_PORT);
  
  IP4_ADDR(&ipaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3); 


  remote_addr.sin_family = AF_INET;      
  remote_addr.sin_port = htons(DEST_PORT);   
  remote_addr.sin_addr.s_addr = ipaddr.addr;
  memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));    
  local_addr.sin_family = AF_INET;      
  local_addr.sin_port = htons(LOCAL_PORT);   
  local_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));    

  vTaskDelay(500);

	while(1)
  {
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0)
    {
      printf("Create socket failed!\r\n");
      vTaskDelay(10);
      continue;
    }

    ret = bind(udp_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
    if (ret == -1)
    {
				printf("UDP failed to binded!\r\n");
        close(udp_sock);
        vTaskDelay(500);
        continue;
    }
		else
		{
			printf("UDP binded!\r\n");
			printf("UDP start connecting!\r\n");
 		  err = connect(udp_sock, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
//  	  if (err == -1)
//    	{
//        printf("UDP connect failed!\r\n");
//        close(udp_sock);
//        vTaskDelay(500);
//        continue;
//    	}
//			else
//			{
	    	printf("UDP connected!\r\n");
	
				udp_timeout.tv_sec = 0;
				udp_timeout.tv_usec = 100000;
				setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &udp_timeout, sizeof(udp_timeout));

	    	printf("Set timeout for receiving data!\r\n");

				while(1)
				{
					if(udp_virtual_connect_status & 1<<4)
					{	
						ret = write(udp_sock, udp_tx_buffer, (int)strlen((char *)udp_tx_buffer));
						if(ret <= 0)
							printf("UDP transmit failed.\r\n");
						else
						{
							printf("UDP is transmitting data: %s\r\n", udp_tx_buffer);
							udp_virtual_connect_status &= ~(1<<4);
						}
					}
		
					ret = recv(udp_sock, &udp_rx_buffer, UDP_RX_RECV_SIZE, 0);
					if(ret > 0)
					{
						printf("UDP has received data: %s\r\n", udp_rx_buffer);
					}
					else if(ret == 0)
					{
						close(udp_sock);
						printf("UDP has disconnected from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
						break;
					}
				}
//			}
		}
  }
}

void udp_virtual_connect_init(void)
{
  sys_thread_new("udp_virtual_connect_socket", udp_virtual_connect_socket, NULL, 512, 4);
}

#endif

//===============================================
// udp normal socket
//===============================================

#ifdef STM32_UDP_SOCKET
uint8_t udp_rx_buffer[UDP_RX_BUFFER_SIZE];
uint8_t *udp_tx_buffer = "UDP test.\r\n";
//bit1: accepted
//bit2: data received
//bit3: data processed
//bit4: data transmitting
uint8_t udp_normal_status;

void udp_normal_loop(void)
{
	if(Key_Value == 1)
	{
		udp_normal_status |= 1<<4;
		Key_Value = 4;
	}
	vTaskDelay(10);
}

static void udp_normal_socket(void *thread_param)
{
	int udp_sock = -1;
  ip4_addr_t ipaddr;
  struct sockaddr_in local_addr, remote_toaddr, remote_fromaddr;
	uint16_t port;
	uint32_t length = 0;
	struct pbuf *q;
	int err, ret;
	socklen_t sin_size;
	struct netbuf *recvbuf;
	struct timeval udp_timeout;
  
	printf("Test for UDP normal socket. \r\n");
  printf("Dest IP is : %d.%d.%d.%d \t port is : %d\r\n",      \
          DEST_IP_ADDR0,DEST_IP_ADDR1,DEST_IP_ADDR2,DEST_IP_ADDR3,DEST_PORT);
  
  IP4_ADDR(&ipaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3); 


  remote_toaddr.sin_family = AF_INET;      
  remote_toaddr.sin_port = htons(DEST_PORT);   
  remote_toaddr.sin_addr.s_addr = ipaddr.addr;
  memset(&(remote_toaddr.sin_zero), 0, sizeof(remote_toaddr.sin_zero));    
  memset(&(remote_fromaddr.sin_zero), 0, sizeof(remote_fromaddr.sin_zero));    
  local_addr.sin_family = AF_INET;      
  local_addr.sin_port = htons(LOCAL_PORT);   
  local_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));    

  vTaskDelay(500);

	while(1)
  {
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0)
    {
      printf("Create socket failed!\r\n");
      vTaskDelay(10);
      continue;
    }

    ret = bind(udp_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
    if (ret == -1)
    {
				printf("UDP failed to binded!\r\n");
        close(udp_sock);
        vTaskDelay(500);
        continue;
    }
		else
		{
			printf("UDP binded!\r\n");
			printf("UDP start connecting!\r\n");

			udp_timeout.tv_sec = 0;
			udp_timeout.tv_usec = 100000;
			setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &udp_timeout, sizeof(udp_timeout));

    	printf("Set timeout for receiving data!\r\n");

			while(1)
			{
				if(udp_normal_status & 1<<4)
				{	
					ret = sendto(udp_sock, udp_tx_buffer, (int)strlen((char *)udp_tx_buffer), 0, (struct sockaddr *)&remote_toaddr, sizeof(struct sockaddr));
					if(ret <= 0)
						printf("UDP transmit failed.\r\n");
					else
					{
						printf("UDP is transmitting data: %s\r\n", udp_tx_buffer);
						udp_normal_status &= ~(1<<4);
					}
				}
	
				ret = recvfrom(udp_sock, &udp_rx_buffer, UDP_RX_RECV_SIZE, 0, (struct sockaddr *)&remote_fromaddr, (socklen_t *)sizeof(struct sockaddr));
				if(ret > 0)
				{
  				ipaddr.addr = remote_fromaddr.sin_addr.s_addr;
					port = remote_fromaddr.sin_port;
					printf("UDP has received data: %s\r\n", udp_rx_buffer);
					printf("UDP received from %d.%d.%d.%d, port is %d . \r\n", (ipaddr.addr & 0x000000ff), (ipaddr.addr & 0x0000ff00) >> 8, (ipaddr.addr & 0x00ff0000) >> 16, (ipaddr.addr & 0xff000000) >> 24, port);
				}
				else if(ret == 0)
				{
					close(udp_sock);
					printf("UDP received failed.\r\n");
					break;
				}
			}
		}
  }
}

void udp_normal_init(void)
{
  sys_thread_new("udp_normal_socket", udp_normal_socket, NULL, 512, 4);
}

#endif
