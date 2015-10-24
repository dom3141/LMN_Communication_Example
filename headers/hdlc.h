/* Copyright 2015 Matthias Köhne, Dominik Lüthe
 * Labor Datentechnik, Ostfalia Hochschule Braunschweig/Wolfenbüttel
 *
 * This file is part of LMN Communication Example for Smart Meters
 *
 * LMN Communication Example for Smart Meters is free software:
 * you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LMN Communication Example for Smart Meters is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LMN Communication Example for Smart Meters. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef HDLC_H_
#define HDLC_H_

#include "lmnadmin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define START_FLAG_LENGTH 1
#define FRAME_FORMAT_LENGTH 2
#define DEST_ADD_LENGTH 2
#define SOURCE_ADD_LENGTH 2
#define CONTROL_LENGTH 1
#define HCS_LENGTH 2
#define FCS_LENGTH 2
#define STOP_FLAG_LENGTH 1

#define START_FLAG 0x7E
#define STOP_FLAG 0x7E
#define FRAME_FORMAT 0xA

#define BROADCAST_TIMEOUT 14000
#define SHORT_BROADCAST_TIMEOUT 1500
#define RECALL_TIMEOUT 15000

#define DELAYTIMEBROADCAST 630;

/*
 enum protocolSelector {	TLS_SML_COSEM = 0x1,
 TLS = 0x2,
 SML_COSEM = 0x3,
 SML_EDL = 0x4,
 SML_SYM = 0x5,
 SYM_HDLC = 0x6,
 TLS_SML_EDL = 0x7,
 TLS_SML_SYM = 0x8,
 SELECTOR_ERROR = 0xFF
 };
 */
/*
 enum iFrame {	I_TLS_SML_COSEM = 0x1,
 I_TLS = 0x2,
 I_SML_COSEM = 0x3,
 I_SML_EDL = 0x4,
 I_SML_SYM = 0x5,
 I_SYM_HDLC = 0x6,
 I_TLS_SML_EDL = 0x7,
 I_TLS_SML_SYM = 0x8,
 iFRAME_ERROR = 0xFF
 };
 */

enum uiFrame {
	ADDR_ALLOC = 0x1, ADDR_CHECK = 0x2, uiFRAME_ERROR = 0xFF
};

enum controlFieldFormat {
	I = 0x10,	// dyn,       xxxx xxx0
	RR = 0x11,	// dyn, 0x01  xxxx 0001
	RNR = 0x15,	// dyn, 0x05  xxxx 0101
	SNRM = 0x93,	// stat, 0x93 100x 0011
	DISC = 0x53,	// stat, 0x53 010x 0011
	UA = 0x73,	// stat, 0x73 011x 0011
	DM = 0x1F,	// stat, 0x0F 000x 1111
	FRMR = 0x97,	// 0x7	      100x 0111
	UI = 0x13,	// stat, 0x13 000x 0011
	FORMAT_ERROR = 0xFF
};

typedef void (*func_ptr)(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);

typedef void (*RS232_func_ptr)(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength,
		func_ptr callback);

void hdlcReceiveData(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);

void hdlcSendData(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength,
		func_ptr callback);

int hdlcBuildFrame(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, controlFieldFormat controlField,
		unsigned char *payload, int payloadLength);

void hdlcBuildBroadcastFrame(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);

void storeCurrentData(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength,
		func_ptr callback);

controlFieldFormat interpretControlFieldFormat(unsigned short controlField);

protocolSelector interpretProtocolSelector(unsigned short selector);

void delayTimerForRRSending(unsigned char destAddr, unsigned sourceAddr,
		protocolSelector protSel, controlFieldFormat controlField,
		unsigned char *payload, int delay_ms);

bool checkCrc(unsigned char* data, int dataLength);

void hdlcInit(void);

void hdlcTerminate(void);

void hdlcBroadcastTimerStart(int milliSeconds);
void hdlcRecallTimerStart(int milliSeconds);

void hdlcBroadcastTimerStop(void);
void hdlcRecallTimerStop(void);

void doHdlcBroadcast(void);
void doHdlcRecall(void);

void delayedSending(unsigned char destAddr, unsigned sourceAddr,
		protocolSelector protSel, controlFieldFormat controlField,
		unsigned char *payload, int payloadLength, int delay_ms);

unsigned char getHdlcAddress(unsigned char * meterId);

#ifdef __cplusplus
}
#endif

#endif /* HDLC_H_ */
