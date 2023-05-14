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

struct netif gnetif;
extern int Key_Value;

//===============================================
// netif config
//===============================================

void Netif_Config(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
 
  IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
  
  // add netif 
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
  
  // set default netif
  netif_set_default(&gnetif);
  
  if (netif_is_link_up(&gnetif))
  {
		// netif set up
    netif_set_up(&gnetif);
    printf("Net interface set up.\r\n");
  }
  else
  {
    // netif set down
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
// tcp client raw
//===============================================

#ifdef STM32_TCP_CLIENT_RAW
static struct tcp_pcb *client_pcb = NULL;
uint8_t tcp_client_rx_buffer[TCP_CLIENT_RX_BUFFER_SIZE];
//uint8_t tcp_client_tx_buffer[TCP_CLIENT_TX_BUFFER_SIZE];
uint8_t *tcp_client_tx_buffer="TCP Client test.\r\n";
// bit1: connected 
// bit2: data recieved
// bit3: data processed
// bit4: data transmitting
uint8_t tcp_client_status;
ip4_addr_t server_ip;

void tcp_client_loop(void)
{
	lwip_periodic();
	if(Key_Value == 1)
	{
		if(tcp_client_status & 1<<1)
			printf("TCP Client has already connected.\r\n");
		else
			tcp_client_main();
		Key_Value = 4;
	}
		
	delay_us(10000);				 
}

void tcp_client_main(void)
{        
  uint8_t res=0, i=0;
	
  client_pcb = tcp_new();	  
	if(client_pcb)
	{
	  printf("TCP Client first start connect!\r\n");  
		printf("TCP Server IP: %d.%d.%d.%d and PORT: %d.\r\n",DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3, DEST_PORT);
  	IP4_ADDR(&server_ip, DEST_IP_ADDR0,DEST_IP_ADDR1,DEST_IP_ADDR2,DEST_IP_ADDR3);
	  tcp_connect(client_pcb, &server_ip, DEST_PORT, tcp_client_connect); 
		res = 0;
  }
	else
	{
		printf("TCP PCB created failed.\r\n");
		res=1;
	}

	while(res == 0)
	{
		if(Key_Value == 0)
		{
			printf("TCP Client is transmitting data: %s\r\n", tcp_client_tx_buffer);
			tcp_client_user_transmit(client_pcb);
			Key_Value = 4;
		}
		if(Key_Value == 2)
		{
			printf("User quit.\r\n");
			Key_Value = 4;
			break;
		}
		if(tcp_client_status & 1<<2)
		{
			tcp_client_status &= ~(1<<2);
			printf("TCP Client has received data: %s\r\n", tcp_client_rx_buffer);
		}
//		if(tcp_client_status & 1<<1)
//			printf("TCP Client connected.\r\n");
//		if(!(tcp_client_status & 1<<1))
//			printf("TCP Client disconnected.\r\n");
	
		lwip_periodic();
		delay_us(2000);				 

		i++;
		if(i == 200)
		{
			if(!(tcp_client_status & 1<<1))
			{
				tcp_client_connect_close(client_pcb, 0);
				printf("TCP Client is trying to reconnect.\r\n");
  			client_pcb = tcp_new();	  
				if(client_pcb)
				{
				  printf("TCP Client start connect!\r\n");  
					printf("TCP Server IP: %d.%d.%d.%d and PORT: %d.\r\n",DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3, DEST_PORT);
			
				  tcp_connect(client_pcb, &server_ip, DEST_PORT, tcp_client_connect); 
				}  
				else
				{
					printf("TCP PCB created failed.\r\n");
				}
			}
			i=0;
		}
	}
	
	tcp_client_connect_close(client_pcb, 0);
	printf("TCP Client connect is closed.\r\n");
	memset(client_pcb, 0, sizeof(struct tcp_pcb));
}

err_t tcp_client_connect(void *arg, struct tcp_pcb *pcb, err_t err)
{
	struct tcp_client_struct *temp = NULL;
	printf("TCP Client connect callback!\r\n");
	if(err == ERR_OK)
	{
		temp = (struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));
		if(temp)
		{
			temp->state = TCP_CLIENT_CONNECTED;
			temp->pcb = pcb;
			temp->p = NULL;
			tcp_arg(pcb, temp);
			tcp_recv(pcb, tcp_client_receive);
			tcp_err(pcb, tcp_client_error);
			tcp_sent(pcb, tcp_client_transmit);
			tcp_poll(pcb, tcp_client_poll, 1);
			tcp_client_status |= 1<<1;
			err = ERR_OK;
			printf("TCP Client connect ok!\r\n");
		}
		else
		{
			tcp_client_connect_close(client_pcb, temp);
			err = ERR_MEM;
			printf("TCP Client connect failed!\r\n");
		}
	}
  return err;
}

err_t tcp_client_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	uint32_t length = 0;
	struct pbuf *q;
	struct tcp_client_struct *temp;
	err_t ret_err;

	temp = (struct tcp_client_struct *)arg;

	if(p == NULL)
	{
		printf("TCP Client received no data and is closing...\r\n");
		temp->state = TCP_CLIENT_CLOSING;
		temp->p = p;
		ret_err = ERR_OK;
	}
	else if(err != ERR_OK)
	{
		printf("TCP Client has received data but been in error status.\r\n");
		if(p)
			pbuf_free(p);
		ret_err = err;
	}
	else if(temp->state == TCP_CLIENT_CONNECTED)
	{
	  if (p != NULL) 
  	{
			memset(tcp_client_rx_buffer, 0, TCP_CLIENT_RX_BUFFER_SIZE);
			for(q = p; q != NULL; q = q->next)
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
			tcp_client_status |= 1<<2;
			printf("TCP Client has already received data.\r\n");	
	    // update received window
  	  tcp_recved(pcb, p->tot_len);
   	  // external loop  
			tcp_write(pcb, p->payload, p->tot_len, 1);  
//    memset(p->payload, 0 , p->tot_len);
  	  pbuf_free(p);
			ret_err = ERR_OK;
		}
  } 
  else  
  {
    printf("TCP Server has already received data but been disconnected!\r\n");
		tcp_recved(pcb, p->tot_len);
		temp->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
  }
  return ret_err;
}

err_t tcp_client_transmit(void *arg, struct tcp_pcb *pcb, uint16_t length)
{
	struct tcp_client_struct *temp;
	err_t err;

	temp = (struct tcp_client_struct *)arg;

	if(temp->p)
		tcp_client_transmit_data(pcb, temp);

	err = ERR_OK;

	return err;
}

void tcp_client_transmit_data(struct tcp_pcb *pcb, struct tcp_client_struct *temp)
{
	struct pbuf *q;
	err_t err=ERR_OK;

	while((err == ERR_OK) && temp->p && (temp->p->len <= tcp_sndbuf(pcb)))
	{
		q = temp->p;
		err = tcp_write(pcb, q->payload, q->len, 1);
		printf("TCP Client is transmitting data...\r\n");
		if(err == ERR_OK)
		{
			printf("TCP Client transmit data successfully.\r\n");
			temp->p = q->next;
			if(temp->p)
				pbuf_ref(temp->p);
			pbuf_free(q);
		}
		else if(err == ERR_MEM)
		{
			temp->p = q;
			printf("TCP Client transmit data failed.\r\n");
		}
		tcp_output(pcb);
	}
}

err_t tcp_client_user_transmit(struct tcp_pcb *pcb)
{
	err_t err;
	struct tcp_client_struct *temp;
	temp = pcb->callback_arg;
	if(temp!=NULL)
	{
		printf("TCP Client connect is idle and ready for transmitting data.\r\n");
		temp->p = pbuf_alloc(PBUF_TRANSPORT, strlen((char *)tcp_client_tx_buffer), PBUF_POOL);
		pbuf_take(temp->p, (char *)tcp_client_tx_buffer, strlen((char *)tcp_client_tx_buffer));
		tcp_client_transmit_data(pcb, temp);
		tcp_client_status &= ~(1<<4);
		if(temp->p)
			pbuf_free(temp->p);
		err = ERR_OK;
	}
	else
	{
		tcp_abort(pcb);
		err = ERR_ABRT;
		printf("TCP Client connect is busy and cannot transmit data.\r\n");
	}
	return err;

}

void tcp_client_error(void *arg, err_t err)
{
  printf("TCP Client connect error!!\n");
//  printf("Try to connect to server again!!\r\n");
  
  tcp_close(client_pcb);
	tcp_arg(client_pcb, NULL);
	tcp_recv(client_pcb, NULL);
	tcp_err(client_pcb, NULL);
	tcp_sent(client_pcb, NULL);
	tcp_poll(client_pcb, NULL, 0);
	tcp_client_status &= ~(1<<1);
  printf("TCP Client connect is closed by core!!\r\n");

//	TCP_Client_Init();
}

err_t tcp_client_poll(void *arg, struct tcp_pcb *pcb)
{
	err_t err;
	struct tcp_client_struct *temp;
	temp = (struct tcp_client_struct *)arg;
	if(temp->state == TCP_CLIENT_CLOSING)
	{
		tcp_client_connect_close(pcb, temp);
		printf("TCP Client is disconnected from a TCP Server.\r\n");
	}
	err = ERR_OK;

	return err;
}

void tcp_client_connect_close(struct tcp_pcb *pcb, struct tcp_client_struct *temp)
{
//	tcp_abort(pcb);
	tcp_close(pcb);
	tcp_arg(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_err(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_poll(pcb, NULL, 0);
	if(temp)
		mem_free(temp);
	tcp_client_status &= ~(1<<1);
}

#endif

//===============================================
// tcp server raw
//===============================================

#ifdef STM32_TCP_SERVER_RAW
static struct tcp_pcb *server_pcb = NULL;
static struct tcp_pcb *server_listen_pcb = NULL;
uint8_t tcp_server_rx_buffer[TCP_CLIENT_RX_BUFFER_SIZE];
//uint8_t tcp_client_tx_buffer[TCP_CLIENT_TX_BUFFER_SIZE];
uint8_t *tcp_server_tx_buffer="TCP Server test.\r\n";
// bit1: accepted 
// bit2: data recieved
// bit3: data processed
// bit4: data transmitting
uint8_t tcp_server_status;
uint32_t client_ip;

void tcp_server_loop(void)
{
	lwip_periodic();
	if(Key_Value == 1)
	{
		if(tcp_server_status & 1<<1)
			printf("TCP Server has already accepted.\r\n");
		else
			tcp_server_main();
		Key_Value = 4;
	}
		
		delay_us(2000);				 
}

void tcp_server_main(void)
{        
  ip4_addr_t server_ip;
  uint8_t res=0, i=0;
	
  server_pcb = tcp_new();	  
	if(server_pcb)
	{
	  printf("TCP Server first start establishing!\r\n");  
	  if((tcp_bind(server_pcb, IP_ADDR_ANY, LOCAL_PORT)) == ERR_OK)      
		{
	 		printf("TCP Server established!\r\n");
	  	server_listen_pcb = tcp_listen(server_pcb);				
	  	printf("TCP Server enter listen!\r\n");
			tcp_accept(server_listen_pcb, tcp_server_accept);
			res = 0;
		}
		else
		{
			printf("TCP Server failed to establish!\r\n");
			res = 1;
		}
  }
	else
	{
		printf("TCP PCB created failed.\r\n");
		res=1;
	}

	while(res == 0)
	{
		if(Key_Value == 0)
		{
			printf("TCP Server is transmitting data: %s\r\n", tcp_server_tx_buffer);
			tcp_server_user_transmit(server_pcb);
			Key_Value = 4;
		}
		if(Key_Value == 2)
		{
			printf("User quit.\r\n");
			Key_Value = 4;
			break;
		}
		if(tcp_server_status & 1<<2)
		{
			tcp_server_status &= ~(1<<2);
			printf("TCP Server has received data: %s\r\n", tcp_server_rx_buffer);
		}
//		if(tcp_client_status & 1<<1)
//			printf("TCP Client connected.\r\n");
//		if(!(tcp_client_status & 1<<1))
//			printf("TCP Client disconnected.\r\n");
	
		lwip_periodic();
		delay_us(2000);				 

		i++;
		if(i == 200)
		{
			i=0;
		}
	}
	
	tcp_server_connect_close(server_listen_pcb, 0);
	tcp_server_connect_close(server_pcb, 0);
	printf("TCP Server connect is closed.\r\n");
//	tcp_server_remove_timewait();
	memset(server_pcb, 0, sizeof(struct tcp_pcb));
	memset(server_listen_pcb, 0, sizeof(struct tcp_pcb));
}

err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	struct tcp_server_struct *temp = NULL;
	printf("TCP Server accept callback!\r\n");
	if(err == ERR_OK)
	{
		tcp_setprio(pcb, TCP_PRIO_MIN);
		temp = (struct tcp_server_struct*)mem_malloc(sizeof(struct tcp_server_struct));
		if(temp)
		{
			temp->state = TCP_SERVER_ACCEPT;
			temp->pcb = pcb;
			temp->p = NULL;
			tcp_arg(pcb, temp);
			tcp_recv(pcb, tcp_server_receive);
			tcp_err(pcb, tcp_server_error);
			tcp_sent(pcb, tcp_server_transmit);
			tcp_poll(pcb, tcp_server_poll, 1);
			tcp_server_status |= 1<<1;
			client_ip = pcb->remote_ip.addr;
			
			err = ERR_OK;
			printf("TCP Server accept ok!\r\n");
			printf("TCP Server accept from %d.%d.%d.%d . \r\n", (client_ip & 0x000000ff), (client_ip & 0x0000ff00) >> 8, (client_ip & 0x00ff0000) >> 16, (client_ip & 0xff000000) >> 24);
		}
		else
		{
			err = ERR_MEM;
			printf("TCP Server accept failed!\r\n");
		}
	}
  return err;
}

err_t tcp_server_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	uint32_t length = 0;
	struct pbuf *q;
	struct tcp_server_struct *temp;
	err_t ret_err;

	temp = (struct tcp_server_struct *)arg;

	if(p == NULL)
	{
		printf("TCP Server received no data and is closing...\r\n");
		temp->state = TCP_SERVER_CLOSING;
		temp->p = p;
		ret_err = ERR_OK;
	}
	else if(err != ERR_OK)
	{
		printf("TCP Server has received data but been in error status.\r\n");
		if(p)
			pbuf_free(p);
		ret_err = err;
	}
	else if(temp->state == TCP_SERVER_ACCEPT)
	{
	  if (p != NULL) 
  	{
			memset(tcp_server_rx_buffer, 0, TCP_SERVER_RX_BUFFER_SIZE);
			for(q = p; q != NULL; q = q->next)
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
			tcp_server_status |= 1<<2;
			client_ip = pcb->remote_ip.addr;
			printf("TCP Server has already received data from %d.%d.%d.%d .\r\n", (client_ip & 0x000000ff), (client_ip & 0x0000ff00) >> 8, (client_ip & 0x00ff0000) >> 16, (client_ip & 0xff000000) >> 24);
	    // update received window
  	  tcp_recved(pcb, p->tot_len);
//    memset(p->payload, 0 , p->tot_len);
  	  pbuf_free(p);
			ret_err = ERR_OK;
		}
  } 
  else  
  {
    printf("TCP Server has already received data but been disconnected!\r\n");
		tcp_recved(pcb, p->tot_len);
		temp->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
  }
  return ret_err;
}

err_t tcp_server_transmit(void *arg, struct tcp_pcb *pcb, uint16_t length)
{
	struct tcp_server_struct *temp;
	err_t err;

	temp = (struct tcp_server_struct *)arg;

	if(temp->p)
		tcp_server_transmit_data(pcb, temp);

	err = ERR_OK;

	return err;
}

void tcp_server_transmit_data(struct tcp_pcb *pcb, struct tcp_server_struct *temp)
{
	struct pbuf *q;
	err_t err=ERR_OK;
	uint16_t length;

	while((err == ERR_OK) && temp->p && (temp->p->len <= tcp_sndbuf(pcb)))
	{
		q = temp->p;
		err = tcp_write(pcb, q->payload, q->len, 1);
		printf("TCP Server is transmitting data...\r\n");
		if(err == ERR_OK)
		{
			printf("TCP Server transmit data successfully.\r\n");
			length = q->len;
			temp->p = q->next;
			if(temp->p)
				pbuf_ref(temp->p);
			pbuf_free(q);
			tcp_recved(pcb, length);
		}
		else if(err == ERR_MEM)
		{
			temp->p = q;
			printf("TCP Server transmit data failed.\r\n");
		}
		tcp_output(pcb);
	}
}

err_t tcp_server_user_transmit(struct tcp_pcb *pcb)
{
	err_t err;
	struct tcp_server_struct *temp;
	temp = pcb->callback_arg;
	if(temp!=NULL)
	{
		printf("TCP Server connect is idle and ready for transmitting data.\r\n");
		temp->p = pbuf_alloc(PBUF_TRANSPORT, strlen((char *)tcp_server_tx_buffer), PBUF_POOL);
		pbuf_take(temp->p, (char *)tcp_server_tx_buffer, strlen((char *)tcp_server_tx_buffer));
		tcp_server_transmit_data(pcb, temp);
		tcp_server_status &= ~(1<<4);
		if(temp->p)
			pbuf_free(temp->p);
		err = ERR_OK;
	}
	else
	{
		tcp_abort(pcb);
		err = ERR_ABRT;
		printf("TCP Server connect is busy and cannot transmit data.\r\n");
	}
	return err;

}

void tcp_server_error(void *arg, err_t err)
{
  printf("TCP Server connect error!!\n");
 
 	if(arg != NULL)
		mem_free(arg);
}

err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb)
{
	err_t err;
	struct tcp_server_struct *temp;
	temp = (struct tcp_server_struct *)arg;
	if(temp->state == TCP_SERVER_CLOSING)
	{
		tcp_server_connect_close(pcb, temp);
		tcp_accept(server_listen_pcb, tcp_server_accept);
		printf("TCP Server is disconnected from a TCP Client.\r\n");
	}
	err = ERR_OK;

	return err;
}

void tcp_server_connect_close(struct tcp_pcb *pcb, struct tcp_server_struct *temp)
{
	tcp_close(pcb);
	tcp_arg(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_err(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_poll(pcb, NULL, 0);
	if(temp)
		mem_free(temp);
	tcp_server_status &= ~(1<<1);
}


#endif

//===============================================
// udp raw
//===============================================

#ifdef STM32_UDP_RAW
static struct udp_pcb *local_pcb = NULL;
uint8_t udp_rx_buffer[UDP_RX_BUFFER_SIZE];
uint8_t *udp_tx_buffer="UDP test.\r\n";
// bit1: connected 
// bit2: data recieved
// bit3: data processed
// bit4: data transmitting
uint8_t udp_normal_status;
uint32_t remote_ip;

void udp_loop(void)
{
	lwip_periodic();
	if(Key_Value == 1)
	{
		if(udp_normal_status & 1<<1)
			printf("UDP has already connected.\r\n");
		else
			udp_main();
		Key_Value = 4;
	}
		
		delay_us(2000);				 
}


void udp_main(void)
{        
  uint8_t res=0, i=0;
	err_t err;
	ip_addr_t remote_ip;
  local_pcb = udp_new();	  
	if(local_pcb)
	{
	  printf("UDP first start connect!\r\n");  
		printf("UDP remote IP: %d.%d.%d.%d and PORT: %d.\r\n",DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3, DEST_PORT);
  	IP4_ADDR(&remote_ip, DEST_IP_ADDR0,DEST_IP_ADDR1,DEST_IP_ADDR2,DEST_IP_ADDR3);

	  if((udp_connect(local_pcb, &remote_ip, DEST_PORT)) == ERR_OK)
		{
			if((udp_bind(local_pcb, IP_ADDR_ANY, DEST_PORT)) == ERR_OK)
			{
				udp_recv(local_pcb, udp_normal_receive, NULL);
				udp_normal_status |= 1<<1;
				printf("UDP connect ok!\r\n");
			}
			else
			{
				res = 1;
				printf("UDP bind error!\r\n");
			}
		}
		else
		{		
			res = 1;
			printf("UDP connect error!\r\n");
		}
		res = 0;
  }
	else
	{
		printf("UDP PCB created failed.\r\n");
		res=1;
	}

	while(res == 0)
	{
		if(Key_Value == 0)
		{
			printf("UDP is transmitting data: %s\r\n", udp_tx_buffer);
			udp_normal_user_transmit(local_pcb);
			Key_Value = 4;
		}
		if(Key_Value == 2)
		{
			printf("User quit.\r\n");
			Key_Value = 4;
			break;
		}
		if(udp_normal_status & 1<<2)
		{
			udp_normal_status &= ~(1<<2);
			printf("UDP has received data: %s\r\n", udp_rx_buffer);
		}
	
		lwip_periodic();
		delay_us(2000);				 

		i++;
		if(i == 200)
		{
			i=0;
		}
	}
	
	udp_normal_connect_close(local_pcb);
	printf("UDP connect is closed.\r\n");
	memset(local_pcb, 0, sizeof(struct udp_pcb));
}

void udp_normal_receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
	uint32_t length = 0;
	struct pbuf *q;
	struct udp_normal_struct *temp;

	temp = (struct udp_normal_struct *)arg;

  if (p != NULL) 
	{
		memset(udp_rx_buffer, 0, UDP_RX_BUFFER_SIZE);
		for(q = p; q != NULL; q = q->next)
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
		udp_normal_status |= 1<<2;
		pcb->remote_ip = *addr;
		pcb->remote_port = port;
		remote_ip = pcb->remote_ip.addr;
		printf("UDP has already received data from %d.%d.%d.%d .\r\n", (remote_ip & 0x000000ff), (remote_ip & 0x0000ff00) >> 8, (remote_ip & 0x00ff0000) >> 16, (remote_ip & 0xff000000) >> 24);
    memset(p->payload, 0 , p->tot_len);
	  pbuf_free(p);
	}
   
  else  
  {
		udp_disconnect(pcb);
    printf("UDP disconnected!\r\n");
		udp_normal_status &= ~(1<<1);
		temp->p = NULL;
		pbuf_free(p);
  }
}

err_t udp_normal_user_transmit(struct udp_pcb *pcb)
{
	err_t err;
	struct pbuf *temp;
	temp = pbuf_alloc(PBUF_TRANSPORT, strlen((char *)udp_tx_buffer), PBUF_POOL);
	pbuf_take(temp, (char *)udp_tx_buffer, strlen((char *)udp_tx_buffer));
	udp_send(pcb, temp);
	if(temp)
		pbuf_free(temp);
	err = ERR_OK;
	return err;
}

void udp_normal_connect_close(struct udp_pcb *pcb)
{
	udp_disconnect(pcb);
	udp_remove(pcb);
	udp_normal_status &= ~(1<<1);
}

#endif
