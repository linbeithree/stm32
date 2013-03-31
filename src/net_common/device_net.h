/*! \file device_net.h
 *
 * Network information for devices connected to the Flight Computer network
 *
 */

#ifndef _DEVICE_NET_H
#define _DEVICE_NET_H

#if LWIP_NETCONN
#include "lwip/ip_addr.h"
#endif

#if LWIP_NETCONN
#define         IMU_A_IP_ADDR(p)            IP4_ADDR(p, 192, 168, 0, 196);
#define         IMU_A_GATEWAY(p)            IP4_ADDR(p, 192, 168, 1,   1  );
#define         IMU_A_NETMASK(p)            IP4_ADDR(p, 255, 255, 255, 0  );
#endif
#define         IMU_A_IP_ADDR_STRING        "192.168.0.196"
#define         IMU_A_LISTEN_PORT           35001
#define         IMU_A_TX_PORT               35002
#define         IMU_A_MAC_ADDRESS           {0xE6, 0x10, 0x20, 0x30, 0x40, 0x11};

#if LWIP_NETCONN
#define         ROLL_CTL_IP_ADDR(p)         IP4_ADDR(p, 192, 168, 0, 197);
#endif
#define         ROLL_CTL_IP_ADDR_STRING     "192.168.0.197"
#define         ROLL_CTL_LISTEN_PORT        35003
#define         ROLL_CTL_TX_PORT            35004

#endif
