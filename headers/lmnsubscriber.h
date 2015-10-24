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

#ifndef LMNSUBSCRIBER_H_
#define LMNSUBSCRIBER_H_

#define WOLFSSL_CALLBACKS
#define HAVE_MAX_FRAGMENT
#define HAVE_SUPPORTED_CURVES
#define WOLFSSL_USER_IO
#define DEBUG_WOLFSSL
#define WOLFSSL_DER_LOAD
#undef NO_CERTS

#include "funcPtr.h"
#include "wolfssl/ssl.h"

#ifdef __cplusplus
extern "C" {
#endif

enum status {
	inactive, active
};

enum channelStatus {
	closed, opened
};

enum tlsState {
	uninitialized, initialized
};

//enum protocolSelector {
//	NO_CONNECTION = 0x0,
//	TLS_SML_COSEM = 0x1,
//	TLS = 0x2,
//	SML_COSEM = 0x3,
//	SML_EDL = 0x4,
//	SML_SYM = 0x5,
//	SYM_HDLC = 0x6,
//	TLS_SML_EDL = 0x7,
//	TLS_SML_SYM = 0x8,
//	SELECTOR_ERROR = 0xFF
//};

void increaseTlnNo(void);

class LmnSubscriber {
private:
	unsigned short address;
	unsigned short myReceiveSequenceNumber;
	unsigned short mySendSequenceNumber;
	unsigned short itsReceiveSequenceNumber;
	unsigned short itsSendSequenceNumber;
	int smlMessageNo;
	status tlnStatus;
	protocolSelector connectionType;
	channelStatus channel;
	unsigned char* tlnIdField;
	static unsigned short tlnNo;
	tlsState tlnTlsState;

public:
	LmnSubscriber();
	LmnSubscriber(unsigned short address);
	~LmnSubscriber();

	unsigned short getAddress();
	unsigned short getMyReceiveSequenceNumber();
	unsigned short getMySendSequenceNumber();
	unsigned short getItsReceiveSequenceNumber();
	unsigned short getItsSendSequenceNumber();

	status getTlnStatus();

	void setMyReceiveSequenceNumber(unsigned short myReceiveSequenceNumber);
	void setMySendSequenceNumber(unsigned short mySendSequenceNumber);

	void setItsReceiveSequenceNumber(unsigned short itsReceiveSequenceNumber);
	void setItsSendSequenceNumber(unsigned short itsSendSequenceNumber);

	void setAddress(unsigned short address);
	void setTlnStatus(status tlnStatus);

	void setTlnTlsState(tlsState tlnTlsState);
	tlsState getTlnTlsState();

	void getTlnIdField(unsigned char* field, unsigned short fieldLength);
	void setTlnIdField(unsigned char* field, unsigned short fieldLength);

	void setSmlMessageNo(int messageNumber);
	int getSmlMessageNo();

	void setChannelStatus(channelStatus status);
	void setConnectionType(protocolSelector connectionType);
	protocolSelector getConnectionType();
	channelStatus getChannelStatus();

	static unsigned short getTlnNo(void) {
		return tlnNo;
	}
};

#ifdef __cplusplus
}
#endif

#endif /* LMNSUBSCRIBER_H_ */
