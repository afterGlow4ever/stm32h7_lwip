//===============================================
//
//	File: ethernetif.c
//	Author: afterGlow,4ever
//	Date: 04282023
//	Version: v1.0
//
// 	This file is including ethernet interface.
//
//===============================================

#include "eth.h" 

#define IFNAME0 's'
#define IFNAME1 't'

//===============================================
// eth descriptors and buffers
//===============================================

__attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
__attribute__((at(0x30040200))) uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_RX_BUFFER_SIZE]; /* Ethernet Receive Buffer */
//__attribute__((section(".RxDecripSection"))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
//__attribute__((section(".TxDecripSection"))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
//__attribute__((section(".RxArraySection"))) uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_RX_BUFFER_SIZE]; /* Ethernet Receive Buffer */

uint32_t current_pbuf_idx =0;

ETH_HandleTypeDef EthHandle;
ETH_TxPacketConfig TxConfig; 
ETH_MACConfigTypeDef MACConfig;
xSemaphoreHandle s_xSemaphore = NULL;
sys_sem_t tx_sem = NULL;
sys_mbox_t eth_tx_mb = NULL;
typedef __IO uint32_t vu32;
LWIP_MEMPOOL_DECLARE(RX_POOL, 20, sizeof(struct pbuf_custom), "Zero-copy RX PBUF pool");

//===============================================
// ethernet init low level
//===============================================

static void low_level_init(struct netif *netif)
{ 
  uint32_t idx = 0;
  uint32_t sn0 = *(vu32*)(0x1FF1E800);

  // mac address
  uint8_t macaddress[6]= {MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5}; 
  
  // eth init  
  EthHandle.Instance = ETH;  
  EthHandle.Init.MACAddr = macaddress;
  EthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
  EthHandle.Init.RxDesc = DMARxDscrTab;
  EthHandle.Init.TxDesc = DMATxDscrTab;
  EthHandle.Init.RxBuffLen = 0x000005F8;
  if(HAL_ETH_Init(&EthHandle) == HAL_ERROR)
    printf("HAL_ETH_Init ERROR...\r\n");
  HAL_ETH_SetMDIOClockRange(&EthHandle);

  // netif config
 	// set netif mac address length
  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  
  // set netif mac address
  netif->hwaddr[0] =  MAC_ADDR0;
  netif->hwaddr[1] =  MAC_ADDR1;
  netif->hwaddr[2] =  MAC_ADDR2;
  netif->hwaddr[3] =  MAC_ADDR3;
  netif->hwaddr[4] =  MAC_ADDR4;
  netif->hwaddr[5] =  MAC_ADDR5;
  
  // set netif mtu
  netif->mtu = ETH_MAX_PAYLOAD;
  
  // set broadcast & arp enable
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  
  // clear rx buffer
  for(idx = 0; idx < ETH_RX_DESC_CNT; idx ++)
  {
    HAL_ETH_DescAssignMemory(&EthHandle, idx, Rx_Buff[idx], NULL);
  }
	
  s_xSemaphore = xSemaphoreCreateCounting(40,0);  
  if(sys_sem_new(&tx_sem , 0) == ERR_OK)
    PRINT_DEBUG("sys_sem_new ok\n");
  
  if(sys_mbox_new(&eth_tx_mb , 50) == ERR_OK)
    PRINT_DEBUG("sys_mbox_new ok\n");

  /* create the task that handles the ETH_MAC */
	sys_thread_new("ETHIN",
                  ethernetif_input,  /* 任务入口函数 */
                  netif,        	  /* 任务入口函数参数 */
                  NETIF_IN_TASK_STACK_SIZE,/* 任务栈大小 */
                  NETIF_IN_TASK_PRIORITY); /* 任务的优先级 */
	
  LWIP_MEMPOOL_INIT(RX_POOL);
	

  // set tx config
  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  
  // init lan8720a
  if(LAN8720_Init(&EthHandle) == HAL_OK) 
  {    
      ethernet_link_check_state(netif);
  }
}

//===============================================
// ethernet output low level
//===============================================

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  uint32_t i=0, framelen = 0;
  struct pbuf *q;
  err_t errval = ERR_OK;
  
  ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];

	static sys_sem_t ousem = NULL;
	if(ousem == NULL)
  {
    sys_sem_new(&ousem,0);
    sys_sem_signal(&ousem);
  }
  
  memset(Txbuffer, 0 , ETH_TX_DESC_CNT*sizeof(ETH_BufferTypeDef));
  
  sys_sem_wait(&ousem);
  
  for(q = p; q != NULL; q = q->next)
  {
    if(i >= ETH_TX_DESC_CNT)	
      return ERR_IF;
    
    Txbuffer[i].buffer = q->payload;
    Txbuffer[i].len = q->len;
    framelen += q->len;
    
    if(i>0)
    {
      Txbuffer[i-1].next = &Txbuffer[i];
    }
    
    if(q->next == NULL)
    {
      Txbuffer[i].next = NULL;
    }
    
    i++;
  }

  TxConfig.Length = framelen;
  TxConfig.TxBuffer = Txbuffer;
  
  HAL_ETH_Transmit(&EthHandle, &TxConfig, ETH_DMA_TRANSMIT_TIMEOUT);

  sys_sem_signal(&ousem);
  
  return errval;
}

//===============================================
// ethernet input low level
//===============================================

static struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p = NULL;
  ETH_BufferTypeDef RxBuff;
  uint32_t framelength = 0;
  
  struct pbuf_custom* custom_pbuf;
  
  if(HAL_ETH_GetRxDataBuffer(&EthHandle, &RxBuff) == HAL_OK) 
  {
    HAL_ETH_GetRxDataLength(&EthHandle, &framelength);

    /* Invalidate data cache for ETH Rx Buffers */
    SCB_InvalidateDCache_by_Addr((uint32_t *)Rx_Buff, (ETH_RX_DESC_CNT*ETH_RX_BUFFER_SIZE));
    
    custom_pbuf  = (struct pbuf_custom*)LWIP_MEMPOOL_ALLOC(RX_POOL);
    custom_pbuf->custom_free_function = pbuf_free_custom;
    
    p = pbuf_alloced_custom(PBUF_RAW, framelength, PBUF_REF, custom_pbuf, RxBuff.buffer, ETH_RX_BUFFER_SIZE);

  }
  
  return p;
}

//===============================================
// ethernet input
//===============================================

void ethernetif_input(void *pParams)
{
	struct netif *netif;
	struct pbuf *p = NULL;
	netif = (struct netif*) pParams;
  LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
  PRINT_DEBUG("ethernetif_input running\n");
	while(1) 
  {
    if(xSemaphoreTake( s_xSemaphore, portMAX_DELAY ) == pdTRUE)
    {
      /* move received packet into a new pbuf */
      taskENTER_CRITICAL();
TRY_GET_NEXT_FRAGMENT:
      p = low_level_input(netif);
      
      /* Build Rx descriptor to be ready for next data reception */
      HAL_ETH_BuildRxDescriptors(&EthHandle);
      
      taskEXIT_CRITICAL();
      /* points to packet payload, which starts with an Ethernet header */
      if(p != NULL)
      {
        taskENTER_CRITICAL();
        if (netif->input(p, netif) != ERR_OK)
        {
          LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
          pbuf_free(p);
          p = NULL;
        }
        else
        {
          xSemaphoreTake( s_xSemaphore, 0);
          goto TRY_GET_NEXT_FRAGMENT;
        }
        taskEXIT_CRITICAL();
      }
    }
	}
}

//===============================================
// ethernet init
//===============================================

err_t ethernetif_init(struct netif *netif)
{
 	struct ethernetif *ethernetif;

//	LWIP_ASSERT("netif != NULL", (netif != NULL));

	ethernetif = mem_malloc(sizeof(ethernetif));

	if (ethernetif == NULL) {
		return ERR_MEM;
	}
  
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */
  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);
  
//  ethernetif->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);
  
  return ERR_OK;
}

//===============================================
// pbuf free
//===============================================

void pbuf_free_custom(struct pbuf *p)
{
  struct pbuf_custom* custom_pbuf = (struct pbuf_custom*)p;
  /* invalidate data cache: lwIP and/or application may have written into buffer */
  SCB_InvalidateDCache_by_Addr((uint32_t *)p->payload, p->tot_len);
  LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf);
}

//===============================================
// ethernet link state check
//===============================================

void ethernet_link_check_state(struct netif *netif)
{
    
    uint32_t PHYLinkState;
    uint32_t linkchanged = 0, speed = 0, duplex =0;
  
    PHYLinkState = LAN8720_GetLinkState(&EthHandle);
  
    if(netif_is_link_up(netif) && (PHYLinkState))
    {
      HAL_ETH_Stop_IT(&EthHandle);
      netif_set_down(netif);
      netif_set_link_down(netif);
      printf("STM32 ethernet suspend.\r\n");		
    }
    else if(!netif_is_link_up(netif) && (PHYLinkState))
    {
      switch ((PHYLinkState & PHY_SPEED_Indication))
      {
        case LAN8740_100MBITS_FULLDUPLEX:{
          duplex = ETH_FULLDUPLEX_MODE;
          speed = ETH_SPEED_100M;
          linkchanged = 1;
					printf("Ethernet 100Mbit/s with full duplex.\r\n");
          break;
        }
        case LAN8740_100MBITS_HALFDUPLEX:{
          duplex = ETH_HALFDUPLEX_MODE;
          speed = ETH_SPEED_100M;
          linkchanged = 1;
					printf("Ethernet 100Mbit/s with half duplex.\r\n");
          break;
        }
        case LAN8740_10MBITS_FULLDUPLEX:{
          duplex = ETH_FULLDUPLEX_MODE;
          speed = ETH_SPEED_10M;
          linkchanged = 1;
					printf("Ethernet 10Mbit/s with full duplex.\r\n");
          break;
        }
        case LAN8740_10MBITS_HALFDUPLEX:{
          duplex = ETH_HALFDUPLEX_MODE;
          speed = ETH_SPEED_10M;
          linkchanged = 1;
					printf("Ethernet 10Mbit/s with half duplex.\r\n");
          break;
        }
        default:
          break;      
      }
    
      if(linkchanged)
      {
          /* Get MAC Config MAC */
          HAL_ETH_GetMACConfig(&EthHandle, &MACConfig); 
          MACConfig.DuplexMode = duplex;
          MACConfig.Speed = speed;
          HAL_ETH_SetMACConfig(&EthHandle, &MACConfig);
          HAL_ETH_Start_IT(&EthHandle);
          netif_set_up(netif);
          netif_set_link_up(netif);
					printf("STM32 ethernet start.\r\n");
      }
    }
}

//===============================================
// lwip timer
//===============================================

static void arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}
