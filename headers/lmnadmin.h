/* Copyright 2015 Dominik Lüthe
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

#ifndef LMNADMIN_H_
#define LMNADMIN_H_

#include "lmnsubscriber.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*func_ptr)(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);

void locateActiveLmnSubscriber(unsigned short* actviveSubscriber,
		unsigned short* noOfActiveSubscriber);
void getSubscriberIDs(unsigned char ** IDs, unsigned short * subscribers);
void registerLmnSubscriber(unsigned short address,
		unsigned short receiveSequenceNumber, unsigned short sendSequenceNumber,
		unsigned char* field, unsigned short fieldLength);
void lmnSendData(unsigned char *ID, protocolSelector protSel,
		unsigned char *payload, int payloadLength, func_ptr callback);
void lmnInit();
void lmnTerminate();
bool lmnSubscriberIsActive(unsigned char address);
void setLmnSubscriberReceiveSequenceNumber(unsigned short address,
		unsigned short receiveSequenceNumber);
void setLmnSubscriberSendSequenceNumber(unsigned short address,
		unsigned short sendSequenceNumber);
void setOwnReceiveSequenceNumber(unsigned short address,
		unsigned short receiveSequenceNumber);
void setOwnSendSequenceNumber(unsigned short address,
		unsigned short sendSequenceNumber);
unsigned short getLmnSubscriberReceiveSequenceNumber(unsigned short address);
unsigned short getLmnSubscriberSendSequenceNumber(unsigned short address);
unsigned short getOwnReceiveSequenceNumber(unsigned short address);
unsigned short getOwnSendSequenceNumber(unsigned short address);
void setTlnIdField(unsigned short address, unsigned char* field,
		unsigned short fieldLength);
void getTlnIdField(unsigned short address, unsigned char* field,
		unsigned short fieldLength);
void deactivateSubscriber(unsigned short address);
void activateSubscriber(unsigned short address);
void setChannelStatus(unsigned short address, channelStatus status);
void setConnectionType(unsigned short address, protocolSelector connectionType);
protocolSelector getConnectionType(unsigned short address);
channelStatus getChannelStatus(unsigned short address);
void getMeterId(unsigned char address, unsigned char* field);
void setTlnTlsState(unsigned short address, tlsState tlnTlsState);
tlsState getTlnTlsState(unsigned short address);
unsigned short getNoOfActiveLmnSubscriber();
void setSmlMessageNo(unsigned short address, int messageNumber);
int getSmlMessageNo(unsigned short address);

#ifdef __cplusplus
}
#endif

#endif /* LMNADMIN_H_ */
