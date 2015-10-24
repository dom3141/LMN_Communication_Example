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

#include <stdio.h>
#include "lmnadmin.h"
#include "lmnsubscriber.h"
#include "test.h"
#include "hdlc.h"
#if TEST
#include <iostream>
#endif

#define MAX_NO_OF_SUBSCRIBER 128

LmnSubscriber* subscriberArray;

void lmnInit() {
	subscriberArray = new LmnSubscriber[MAX_NO_OF_SUBSCRIBER];
	for (int i = 0; i < MAX_NO_OF_SUBSCRIBER; i++) {
		subscriberArray[i] = LmnSubscriber(static_cast<unsigned int>(i));
		subscriberArray[i].setTlnStatus(inactive);
	}
}

void lmnSendData(unsigned char *ID, protocolSelector protSel,
		unsigned char *payload, int payloadLength, func_ptr callback) {

	int destAddr = getHdlcAddress(ID);
#if PRINT_IO
	printf("\n\nlmnSendSata: folgende HDLC-Adresse wurde ermittelt: %d\n\n", destAddr);
#endif
	if (destAddr != 0x00) {
		hdlcSendData(destAddr, 0x01, protSel, payload, payloadLength, callback);
	}
	return;
}

void lmnTerminate() {
	delete[] subscriberArray;
}

bool lmnSubscriberIsActive(unsigned char address) {
	if ((subscriberArray[static_cast<unsigned short>(address)].getTlnStatus()
			== active))
		return true;
	return false;
}

void setConnectionType(unsigned short address,
		protocolSelector connectionType) {
	subscriberArray[address].setConnectionType(connectionType);
	if (connectionType == NO_CONNECTION)
		setChannelStatus(address, closed);
	else
		setChannelStatus(address, opened);
	return;
}

void setChannelStatus(unsigned short address, channelStatus status) {
	subscriberArray[address].setChannelStatus(status);
	return;
}

protocolSelector getConnectionType(unsigned short address) {
	return subscriberArray[address].getConnectionType();
}

channelStatus getChannelStatus(unsigned short address) {
	return subscriberArray[address].getChannelStatus();
}

void locateActiveLmnSubscriber(unsigned short* actviveSubscriber,
		unsigned short* noOfActiveSubscriber) {

	/* aktive Teilnehmer finden */
	unsigned short subscriberIndex = 0;
	for (unsigned short i = 0; i < MAX_NO_OF_SUBSCRIBER; i++) {
		if (subscriberArray[i].getTlnStatus() == active) {
			actviveSubscriber[subscriberIndex] =
					subscriberArray[i].getAddress();
			subscriberIndex++;
		}
	}
	*noOfActiveSubscriber = subscriberIndex;
	return;
}

unsigned short getNoOfActiveLmnSubscriber() {
	unsigned short noOfActiveLmnSubscriber = 0;
	for (unsigned short i = 0; i < MAX_NO_OF_SUBSCRIBER; i++) {
		if (subscriberArray[i].getTlnStatus() == active)
			noOfActiveLmnSubscriber++;
	}
	return noOfActiveLmnSubscriber;
}

void getSubscriberIDs(unsigned char ** IDs, unsigned short * subscribers) {
	unsigned short sub, idx;
	unsigned short* subscriberArray = new unsigned short[128];
	unsigned char* idField = new unsigned char[12];

	locateActiveLmnSubscriber(subscriberArray, subscribers);
	for (sub = 0; sub < *subscribers; sub++) {
		getTlnIdField(subscriberArray[sub], idField, 12);
		for (idx = 0; idx < 10; idx++)
			IDs[sub][idx] = idField[idx + 2];
	}
	delete[] subscriberArray;
	delete[] idField;
	return;
}

void registerLmnSubscriber(unsigned short address,
		unsigned short receiveSequenceNumber, unsigned short sendSequenceNumber,
		unsigned char* field, unsigned short fieldLength) {

	subscriberArray[address].setTlnStatus(active);
	subscriberArray[address].setItsReceiveSequenceNumber(receiveSequenceNumber);
	subscriberArray[address].setItsSendSequenceNumber(sendSequenceNumber);
	subscriberArray[address].setTlnIdField(field, fieldLength);
	return;
}

void setLmnSubscriberReceiveSequenceNumber(unsigned short address,
		unsigned short receiveSequenceNumber) {
	subscriberArray[address].setItsReceiveSequenceNumber(receiveSequenceNumber);
	return;
}

void setLmnSubscriberSendSequenceNumber(unsigned short address,
		unsigned short sendSequenceNumber) {
	subscriberArray[address].setItsSendSequenceNumber(sendSequenceNumber);
	return;
}

void setOwnReceiveSequenceNumber(unsigned short address,
		unsigned short receiveSequenceNumber) {
	subscriberArray[address].setMyReceiveSequenceNumber(receiveSequenceNumber);
	return;
}

void setOwnSendSequenceNumber(unsigned short address,
		unsigned short sendSequenceNumber) {
	subscriberArray[address].setMySendSequenceNumber(sendSequenceNumber);
	return;
}

unsigned short getLmnSubscriberReceiveSequenceNumber(unsigned short address) {
	return subscriberArray[address].getItsReceiveSequenceNumber();
}

unsigned short getLmnSubscriberSendSequenceNumber(unsigned short address) {
	return subscriberArray[address].getItsSendSequenceNumber();
}

unsigned short getOwnReceiveSequenceNumber(unsigned short address) {
	return subscriberArray[address].getMyReceiveSequenceNumber();
}

unsigned short getOwnSendSequenceNumber(unsigned short address) {
	return subscriberArray[address].getMySendSequenceNumber();
}

void getTlnIdField(unsigned short address, unsigned char* field,
		unsigned short fieldLength) {
	subscriberArray[address].getTlnIdField(field, fieldLength);
	return;
}

void setTlnIdField(unsigned short address, unsigned char* field,
		unsigned short fieldLength) {
	subscriberArray[address].setTlnIdField(field, fieldLength);
	return;
}

void getMeterId(unsigned char address, unsigned char* field) {
	unsigned char* tmpField = new unsigned char[12];
	subscriberArray[address].getTlnIdField(tmpField, 12);
	for (int i = 0; i < 10; i++)
		field[i] = tmpField[i + 2];
	delete[] tmpField;
	return;
}

void deactivateSubscriber(unsigned short address) {
	subscriberArray[address].setTlnStatus(inactive);
	return;
}

void activateSubscriber(unsigned short address) {
	subscriberArray[address].setTlnStatus(active);
	return;
}

void setTlnTlsState(unsigned short address, tlsState tlnTlsState) {
	subscriberArray[address].setTlnTlsState(tlnTlsState);
	return;
}

tlsState getTlnTlsState(unsigned short address) {
	return subscriberArray[address].getTlnTlsState();
}

void setSmlMessageNo(unsigned short address, int messageNumber) {
	subscriberArray[address].setSmlMessageNo(messageNumber);
	return;
}

int getSmlMessageNo(unsigned short address) {
	return subscriberArray[address].getSmlMessageNo();
}
