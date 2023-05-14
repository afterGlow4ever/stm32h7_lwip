//===============================================
//
//	File: LAN8720a.h
//	Author: afterGlow,4ever
//	Date: 04282023
//	Version: v1.0
//
// 	This file is including lan8720a basic parameters.
//
//===============================================

#ifndef _LAN8720A_H_
#define	_LAN8720A_H_

//===============================================
// lan8720a reg define
//===============================================

// address
#define LAN8720A_PHY_ADDRESS            0x00

// delay time
#define PHY_RESET_DELAY                 ((uint32_t)0x00000F)
#define PHY_CONFIG_DELAY                ((uint32_t)0x00000F)

// pending time
#define PHY_READ_TO                     ((uint32_t)0x0000FFFF)
#define PHY_WRITE_TO                    ((uint32_t)0x0000FFFF)

// basic regs
#define PHY_BCR                         ((uint16_t)0x00)   //R0--»ù±¾¿ØÖÆ¼Ä´æÆ÷
#define PHY_BSR                         ((uint16_t)0x01)   //R1--»ù±¾×´Ì¬¼Ä´æÆ÷

// mask for basic regs
#define PHY_RESET                       ((uint16_t)0x8000)  /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000)  /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100)  /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000)  /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100)  /*!< Set the full-duplex mode at 10 Mb/s  */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000)  /*!< Set the half-duplex mode at 10 Mb/s  */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000)  /*!< Enable auto-negotiation function     */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200)  /*!< Restart auto-negotiation function    */
#define PHY_POWERDOWN                   ((uint16_t)0x0800)  /*!< Select the power down mode           */
#define PHY_ISOLATE                     ((uint16_t)0x0400)  /*!< Isolate PHY from MII                 */
#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020)  /*!< Auto-Negotiation process completed   */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004)  /*!< Valid link established               */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002)  /*!< Jabber condition detected            */
#define PHY_SPEED_Indication            ((uint16_t)0x001C)
#define LAN8740_10MBITS_HALFDUPLEX      ((uint16_t)0x0004)
#define LAN8740_10MBITS_FULLDUPLEX      ((uint16_t)0x0014)
#define LAN8740_100MBITS_HALFDUPLEX     ((uint16_t)0x0008)
#define LAN8740_100MBITS_FULLDUPLEX     ((uint16_t)0x0018)

// specific & status regs
#define PHY_SR                          ((uint16_t)0x1F)    /*!< PHY special control/ status register Offset     */
#define PHY_SPEED_STATUS                ((uint16_t)0x0004)  /*!< PHY Speed mask                                  */
#define PHY_DUPLEX_STATUS               ((uint16_t)0x0010)  /*!< PHY Duplex mask                                 */
#define PHY_ISFR                        ((uint16_t)0x1D)    /*!< PHY Interrupt Source Flag register Offset       */
#define PHY_ISFR_INT4                   ((uint16_t)0x0010)  /*!< PHY Link down inturrupt */

#endif
