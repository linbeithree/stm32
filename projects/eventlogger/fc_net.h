/*! \file fc_net.h
 *
 * Define the Flight Computer network information
 *
 */

#ifndef _FC_NET_H
#define _FC_NET_H


#if LWIP_NETCONN
	#include "lwip/ip_addr.h"
	#define         IP_PSAS_FC(p)           IP4_ADDR(p, 10, 0, 0, 10)
#endif
#define         FC_LISTEN_PORT_IMU_A    36000
#define         FC_TX_PORT_IMU_A        36020

#define         FC_LISTEN_PORT_ROLL_CTL 36002
#define         FC_TX_PORT_ROLL_CTL     36020

#endif
