//===============================================
//
//	File: eth_param.h
//	Author: afterGlow,4ever
//	Date: 04282023
//	Version: v1.0
//
// 	This file is including eth basic parameters.
//
//===============================================

#ifndef _ETH_PARAM_H_
#define	_ETH_PARAM_H_

//===============================================
// mac address
//===============================================

#define MAC_ADDR0   0x02U
#define MAC_ADDR1   0x00U
#define MAC_ADDR2   0x00U
#define MAC_ADDR3   0x00U
#define MAC_ADDR4   0x00U
#define MAC_ADDR5   0x00U

//===============================================
// ipv4 address
//===============================================

// destination ip address
#define DEST_IP_ADDR0   ((uint8_t)192U)
#define DEST_IP_ADDR1   ((uint8_t)168U)
#define DEST_IP_ADDR2   ((uint8_t)1U)
#define DEST_IP_ADDR3   ((uint8_t)104U)
#define DEST_PORT       ((uint16_t)5001U)
   
// local ip address
#define IP_ADDR0   ((uint8_t) 192U)
#define IP_ADDR1   ((uint8_t) 168U)
#define IP_ADDR2   ((uint8_t) 1U)
#define IP_ADDR3   ((uint8_t) 30U)
#define LOCAL_PORT       ((uint16_t)5001)

// netmask address
#define NETMASK_ADDR0   ((uint8_t) 255U)
#define NETMASK_ADDR1   ((uint8_t) 255U)
#define NETMASK_ADDR2   ((uint8_t) 255U)
#define NETMASK_ADDR3   ((uint8_t) 0U)

// gateway address
#define GW_ADDR0   ((uint8_t) 192U)
#define GW_ADDR1   ((uint8_t) 168U)
#define GW_ADDR2   ((uint8_t) 1U)
#define GW_ADDR3   ((uint8_t) 1U) 


#endif
