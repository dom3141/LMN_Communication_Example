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

#include "hdlc.h"
#include "crc.h"
#include "timer.h"
#include "RS232.h"
#include "fakeRS232.h"
//#include "RS232_admin.h"
#include "lmnsubscriber.h"
#include "lmnadmin.h"
#include "sendobject.h"
#include "test.h"
//#include <Windows.h>
#include <stdio.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#if DETAILED_TEST
#include <iostream>
#endif

using namespace std;

func_ptr callbackToUpperLayer;
func_ptr callbackToSameLayer = hdlcReceiveData;
extern int global_sim;
extern int global_flush;
RS232_func_ptr RS232SendFunction;
//RS232_func_ptr RS232SendFunction = RS232_SendRequest;
unsigned char localPayloadBuffer[2048];
unsigned char upperPayloadBuffer[2048];
int upperPayloadBufferLength;
SendObject dataToSend = SendObject();
int sendenAktiv = 0;
int broadcastAktiv = 0;
int receiveReadyCounter = 0;
int receiveNotReadyCounter = 0;
int dm_sent = 0;
int noSubscriber = 0;
int firstRun = 1;
int channelOpenRequest = 0;

int broadcast = 0;
int recall = 0;

extern int global_debug;
extern int global_log;
extern FILE *logfile;
extern int fileOpened;

char debugString[25000], tempString[25000];

unsigned char storedDestAddr;
unsigned char storedSourceAddr;
protocolSelector storedProtSel;
unsigned char storedPayload[2048];
int storedPayloadLength;
func_ptr storedCallback;

void hdlcSendData(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength,
		func_ptr callback) {

	storeCurrentData(destAddr, sourceAddr, protSel, payload, payloadLength,
			callback);

	if (global_sim != 0)
		RS232SendFunction = fakeRS232DataRequest;
	else
		RS232SendFunction = RS232DataRequest;

	callbackToUpperLayer = callback;
	int frameLength;

	for (int i = 0; i < payloadLength; i++)
		upperPayloadBuffer[i] = payload[i];
	upperPayloadBufferLength = payloadLength;

	try {

		if (lmnSubscriberIsActive(destAddr)) {
			//no Broadcast needed
#if DETAILED_TEST
			printf("\nKEIN BROADCAST ERFORDERLICH\n");
#endif
			switch (protSel) {

			case TLS_SML_COSEM:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= TLS_SML_COSEM) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//usleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht TLS_SML_COSEM wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nTLS_SML_COSEM Kanal ist geschlossen\n");
#endif
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr,
							TLS_SML_COSEM, SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nTLS_SML_COSEM Kanalaufbau angefordert\n");
#endif

					RS232SendFunction(0, 0, protSel, payload, frameLength,
							hdlcReceiveData);
					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird TLS_SML_COSEM Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				if (upperPayloadBufferLength == 0) {
					frameLength = hdlcBuildFrame(destAddr, sourceAddr,
							TLS_SML_COSEM, RR, upperPayloadBuffer, 0);
#if DETAILED_TEST
					printf("\nAufgrund leerer Payload wird ein RR gesendet\n");
#endif
				} else
					frameLength = hdlcBuildFrame(destAddr, sourceAddr,
							TLS_SML_COSEM, I, upperPayloadBuffer,
							upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nTLS_SML_COSEM Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif
				if (getConnectionType(static_cast<unsigned short>(destAddr))
						== TLS_SML_COSEM) {
					RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
							frameLength, hdlcReceiveData);
				} else {
					//printf("\n\nZaehler im DC-Mode!!!\n\n");
				}

				sendenAktiv = 0;
				break;
			case TLS:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= TLS) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//usleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht TLS wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nTLS Kanal ist geschlossen\n");
#endif
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr, TLS,
							SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nTLS Kanalaufbau angefordert\n");
#endif
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							== TLS) {
						RS232SendFunction(0, 0, TLS, payload, frameLength,
								hdlcReceiveData);
					} else {
						//printf("\n\nZaehler im DC-Mode!!!\n\n");
					}

					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird TLS Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				if (upperPayloadBufferLength == 0) {
					frameLength = hdlcBuildFrame(destAddr, sourceAddr, TLS, RR,
							upperPayloadBuffer, 0);
#if DETAILED_TEST
					printf("\nAufgrund leerer Payload wird ein RR gesendet\n");
#endif
				} else
					frameLength = hdlcBuildFrame(destAddr, sourceAddr, TLS, I,
							upperPayloadBuffer, upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nTLS Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif
				RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
						frameLength, hdlcReceiveData);
				sendenAktiv = 0;
				break;
			case SML_COSEM:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= SML_COSEM) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//usleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht SML_COSEM wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nSML_COSEM Kanal ist geschlossen\n");
#endif
					setTlnTlsState(destAddr, uninitialized);
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr,
							SML_COSEM, SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nKanalaufbau angefordert\n");
#endif
					RS232SendFunction(0, 0, protSel, payload, frameLength,
							hdlcReceiveData);
					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird SML_COSEM Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				frameLength = hdlcBuildFrame(destAddr, sourceAddr, SML_COSEM, I,
						upperPayloadBuffer, upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nSML_COSEM Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif

				if (getConnectionType(static_cast<unsigned short>(destAddr))
						== SML_COSEM) {
					RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
							frameLength, hdlcReceiveData);
				} else {
					//printf("\n\nZaehler im DC-Mode!!!\n\n");
				}
				sendenAktiv = 0;
				break;

			case SML_EDL:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= SML_EDL) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//usleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht SML_COSEM wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nSML_EDL Kanal ist geschlossen\n");
#endif
					setTlnTlsState(destAddr, uninitialized);
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr, SML_EDL,
							SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nSML_EDL Kanalaufbau angefordert\n");
#endif
					RS232SendFunction(0, 0, protSel, payload, frameLength,
							hdlcReceiveData);
					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird SML_EDL Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				frameLength = hdlcBuildFrame(destAddr, sourceAddr, SML_EDL, I,
						upperPayloadBuffer, upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nSML_EDL Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif
				RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
						frameLength, hdlcReceiveData);
				sendenAktiv = 0;
				break;
			case SML_SYM:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= SML_SYM) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//usleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht SML_SYM wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nSML_SYM Kanal ist geschlossen\n");
#endif
					setTlnTlsState(destAddr, uninitialized);
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr, SML_SYM,
							SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nSML_SYM Kanalaufbau angefordert\n");
#endif
					RS232SendFunction(0, 0, protSel, payload, frameLength,
							hdlcReceiveData);
					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird SML_SYM Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				frameLength = hdlcBuildFrame(destAddr, sourceAddr, SML_SYM, I,
						upperPayloadBuffer, upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nSML_SYM Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif
				RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
						frameLength, hdlcReceiveData);
				sendenAktiv = 0;
				break;
			case SYM_HDLC:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= SYM_HDLC) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//sleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht SYM_HDLC wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nSYM_HDLC Kanal ist geschlossen\n");
#endif
					setTlnTlsState(destAddr, uninitialized);
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr, SYM_HDLC,
							SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nSYM_HDLC Kanalaufbau angefordert\n");
#endif
					RS232SendFunction(0, 0, protSel, payload, frameLength,
							hdlcReceiveData);
					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird SML_COSEM Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				frameLength = hdlcBuildFrame(destAddr, sourceAddr, SYM_HDLC, I,
						upperPayloadBuffer, upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nSYM_HDLC Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif
				RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
						frameLength, hdlcReceiveData);
				sendenAktiv = 0;
				break;
			case TLS_SML_EDL:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= TLS_SML_EDL) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//usleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht TLS_SML_EDL wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nTLS_SML_EDL Kanal ist geschlossen\n");
#endif
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr,
							TLS_SML_EDL, SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nTLS_SML_EDL Kanalaufbau angefordert\n");
#endif
					RS232SendFunction(0, 0, protSel, payload, frameLength,
							hdlcReceiveData);
					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird TLS_SML_EDL Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				frameLength = hdlcBuildFrame(destAddr, sourceAddr, TLS_SML_EDL,
						I, upperPayloadBuffer, upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nTLS_SML_EDL Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif
				RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
						frameLength, hdlcReceiveData);
				sendenAktiv = 0;
				break;
			case TLS_SML_SYM:
				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== opened) {
					if (getConnectionType(static_cast<unsigned short>(destAddr))
							!= TLS_SML_SYM) {
#if DETAILED_TEST
						printf("\nBestehender Kanal wird geschlossen\n");
#endif
						channelOpenRequest = 0;
						frameLength = hdlcBuildFrame(destAddr, sourceAddr,
								getConnectionType(
										static_cast<unsigned short>(destAddr)),
								DISC, payload, 0);
						while (broadcastAktiv == 1)
							;
						sendenAktiv = 1;
						RS232SendFunction(0, 0, protSel, payload, frameLength,
								hdlcReceiveData);
						sendenAktiv = 0;
						//usleep(500 * 1000);
					}
				} //Falls Kanal offen und nicht TLS_SML_SYM wird der Kanal abgebaut

				if (getChannelStatus(static_cast<unsigned short>(destAddr))
						== closed) {
#if DETAILED_TEST
					printf("\nTLS_SML_SYM Kanal ist geschlossen\n");
#endif
					//*************************** KANAL SSN UND RSN AUF NULL
					setLmnSubscriberReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setLmnSubscriberSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnReceiveSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					setOwnSendSequenceNumber(
							static_cast<unsigned short>(destAddr), 0);
					channelOpenRequest = 1;
					frameLength = hdlcBuildFrame(destAddr, sourceAddr,
							TLS_SML_SYM, SNRM, payload, 0);
					while (broadcastAktiv == 1)
						;
					sendenAktiv = 1;
#if PRINT_IO
					printf("\nhdlcSendData: Data that will be sent:\n");
					for (int i = 0; i < (frameLength); i++) {
						printf("0x%02X ", payload[i]);
						if ((i + 1) % 16 == 0)
						printf("\n");
					}
					printf("\n");
#endif
#if DETAILED_TEST
					printf("\nTLS_SML_SYM Kanalaufbau angefordert\n");
#endif
					RS232SendFunction(0, 0, protSel, payload, frameLength,
							hdlcReceiveData);
					sendenAktiv = 0;
					//usleep(500 * 1000);
				} //Falls kein Kanal offen, wird SML_COSEM Kanal aufgebaut

				//ansonsten besteht bereits ein offener Kanal
				frameLength = hdlcBuildFrame(destAddr, sourceAddr, TLS_SML_SYM,
						I, upperPayloadBuffer, upperPayloadBufferLength);
#if DETAILED_TEST
				printf("\nTLS_SML_SYM Anfrage wird gesendet\n");
#endif
				while (broadcastAktiv == 1)
					;
				sendenAktiv = 1;
#if PRINT_IO
				printf("\nhdlcSendData: Data that will be sent:\n");
				for (int i = 0; i < (frameLength); i++) {
					printf("0x%02X ", upperPayloadBuffer[i]);
					if ((i + 1) % 16 == 0)
					printf("\n");
				}
				printf("\n");
#endif
				RS232SendFunction(0, 0, protSel, upperPayloadBuffer,
						frameLength, hdlcReceiveData);
				sendenAktiv = 0;
				break;
			default:
				break;

			}
		} else {
			throw 0xE1;
		}

#if DETAILED_TEST
		printf("\nhdlcSendData: returned data from hdlcBuildFrame():\n");
		for (int i = 0; i < (frameLength); i++) {
			printf("0x%02X ", payload[i]);
			if ((i + 1) % 16 == 0)
			printf("\n");
		}
		printf("\n");
#endif
	}

	catch (int e) {
		printf(
				"\n\n\n ****** E R R O R *******     Fehler in hdlcSendData() - Fehlercode: 0x%02X     ************ \n",
				e);
		switch (e) {
		case 0xE1:
			printf("\n 0xE1: Teilnehmer (0x%02X) nicht bekannt\n", destAddr);
			break;
		case 0xE2:
			printf("\n 0xE2: \n");
			break;
		case 0xE3:
			printf("\n 0xE3: \n");
			break;
		case 0xE4:
			printf("\n 0xE4: \n");
			break;
		case 0xE5:
			printf("\n 0xE5: \n");
			break;
		default:
			printf("\n  :-o");
		}
	}
	return;
}

void hdlcReceiveData(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength) {

	controlFieldFormat fieldFormat;
	unsigned short tempSequenzNumber;

#if DETAILED_TEST
#if WINDOWS
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
#endif

#if PRINT_IO
	printf("\nhdlcReceiveData: Data that has been received:\n");
	for (int i = 0; i < (payloadLength); i++) {
		printf("0x%02X ", payload[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
	printf("\n");
#endif
	noSubscriber = 0;
	if (payloadLength == -1) {
		broadcast = 0;
		recall = 0;
#if PRINT_IO
		printf("\n\nKeine Antwort vom Zaehler erhalten :-( \n\n");
#endif
		if (firstRun == 1) {
			hdlcBroadcastTimerStart(SHORT_BROADCAST_TIMEOUT);
			noSubscriber = 1;
		}
		if (getNoOfActiveLmnSubscriber() == 0)
			noSubscriber = 1;
		//hier evtl. Fehlerbehandlung ;-)
	} else {
		if (firstRun == 1) {
			hdlcRecallTimerStart(RECALL_TIMEOUT);
		}
		try {

			if (payload[0] != 0x7E || payload[payloadLength - 1] != 0x7E) { // RAHMEN 0x7E FEHLT
				if (broadcastAktiv == 0) {
					callbackToUpperLayer(destAddr, sourceAddr, protSel, payload,
							-100);
				}
			} else {

#if DEBUG
				printf("\n\nPAYLOAD KOMPLETT IN HDLC\n");
				for (int i = 0; i < payloadLength; i++) {
					printf("0x%02X ", payload[i]);
					if (i != 0 && (i+1) % 16 == 0)
					printf("\n");
				}
#endif
				unsigned short payloadlengthstore = payloadLength;
				unsigned char* payloadsave = new unsigned char[payloadLength];

				for (int i = 0; i < payloadLength; i++) {
					payloadsave[i] = payload[i];
				}
				int offset = 0;
				bool nochmehr = false;
				bool nextLine = false;

				do {
					if (nochmehr == true)
						nextLine = true;
					else
						nextLine = false;

					unsigned short telgrammlaenge = ((payloadsave[1 + offset]
							- 0xA0) << 8) + payloadsave[2 + offset];

					if (telgrammlaenge + 2 + offset < payloadlengthstore) {
						payloadLength = telgrammlaenge + 2;
						for (int i = 0; i < payloadLength; i++) {
							payload[i] = payloadsave[i + offset];
						}
						offset += telgrammlaenge + 2;

						nochmehr = true;

					} else {
						for (int i = 0; i < telgrammlaenge + 2; i++) {
							payload[i] = payloadsave[i + offset];
						}

						nochmehr = false;
					}

#if DEBUG
					printf("\n\nPAYLOAD IN HDLC\n");
					for (int i = 0; i < payloadLength; i++) {
						printf("0x%02X ", payload[i]);
						if (i != 0 && (i+1) % 16 == 0)
						printf("\n");
					}

					printf("\n");
#endif
					bzero((char *) &debugString, 25000);
					bzero((char *) &tempString, 25000);

					unsigned char debugSourceAddr = (payload[3] >> 1) & 0xFF;
					unsigned char debugDestAddr = (payload[5] >> 1) & 0xFF;
					unsigned char debugProtSel =
							static_cast<protocolSelector>((payload[4] & 0xF)
									>> 1);
					unsigned char debugControlField = payload[7] & 0xFF;
					if (nextLine)
						sprintf(tempString,
								"\n[DST]=0x%02X [SRC]=0x%02X [PTC]=",
								debugSourceAddr, debugDestAddr);
					else
						sprintf(tempString, "[DST]=0x%02X [SRC]=0x%02X [PTC]=",
								debugSourceAddr, debugDestAddr);
					strcat(debugString, tempString);
					if (broadcast == 1)
						sprintf(tempString, "BROADCAST REPLY");
					else if (recall == 1)
						sprintf(tempString, "RECALL REPLY");
					else {
						switch (debugProtSel & 0xFF) {
						case 0x1:
							sprintf(tempString, "TLS_SML_COSEM");
							break;
						case 0x2:
							sprintf(tempString, "TLS");
							break;
						case 0x3:
							sprintf(tempString, "SML_COSEM");
							break;
						case 0x4:
							sprintf(tempString, "SML_EDL");
							break;
						case 0x5:
							sprintf(tempString, "SML_SYM");
							break;
						case 0x6:
							sprintf(tempString, "SYM_HDLC");
							break;
						case 0x7:
							sprintf(tempString, "TLS_SML_EDL");
							break;
						case 0x8:
							sprintf(tempString, "TLS_SML_SYM");
							break;
						default:
							sprintf(tempString, "SELECTOR_ERROR");
							break;
						}
					}
					strcat(debugString, tempString);
					sprintf(tempString, " [CTR]=");
					strcat(debugString, tempString);
					if (((debugControlField & 0xFF) & 0x1) == 0x00)
						sprintf(tempString, "I");
					if (((debugControlField & 0xFF) & 0xF) == 0x01)
						sprintf(tempString, "RR");
					if (((debugControlField & 0xFF) & 0xF) == 0x05)
						sprintf(tempString, "RNR");
					if (((debugControlField & 0xFF) & 0xEF) == 0x83)
						sprintf(tempString, "SNRM");
					if (((debugControlField & 0xFF) & 0xEF) == 0x43)
						sprintf(tempString, "DISC");
					if (((debugControlField & 0xFF) & 0xEF) == 0x63)
						sprintf(tempString, "UA");
					if (((debugControlField & 0xFF) & 0xEF) == 0x0F)
						sprintf(tempString, "DM");
					if (((debugControlField & 0xFF) & 0xEF) == 0x87)
						sprintf(tempString, "FRMR");
					if (((debugControlField & 0xFF) & 0xEF) == 0x03)
						sprintf(tempString, "UI");
					strcat(debugString, tempString);
					sprintf(tempString, "\n");
					strcat(debugString, tempString);

					//sprintf(tempString, "\nRECEIVE DATA:\n");
					for (int i = 0; i < payloadLength; i++) {
						sprintf(tempString, "0x%02X ", payload[i]);
						strcat(debugString, tempString);
						if (i != 0 && (i + 1) % 16 == 0) {
							sprintf(tempString, "\n");
							strcat(debugString, tempString);
						}
					}

					if (!nochmehr
							|| (((debugControlField & 0xFF) & 0xF) == 0x05)) { //oder RNR
						sprintf(tempString,
								"\n\n-------------------------------------------------------------------------------\n");
						strcat(debugString, tempString);
					}

					if (global_debug != 0)
						printf(debugString);
					if (global_log != 0) {
						while (fileOpened == 0)
							;
						fprintf(logfile, debugString);

						if (global_flush != 0) {
							fflush(logfile);
							fsync(fileno(logfile));
						}
					}

					destAddr = (payload[3] >> 1) & 0xFF;
					sourceAddr = (payload[5] >> 1) & 0xFF;
					protSel = static_cast<protocolSelector>((payload[4] & 0xF)
							>> 1);

					if (!checkCrc(payload + 1, 9)) // HCS IST FALSCH
						throw 0xF3;

					if (checkCrc(payload + 1, payloadLength - 2)) { // FCS IST NICHT FALSCH, DER REST SCHEINT I.O. ZU SEIN

						fieldFormat = interpretControlFieldFormat(
								(payload[7] & 0xFF));
#if DETAILED_TEST
						printf("\n\nField Format of received Data: 0x%02X: ", fieldFormat);
#endif

						switch (fieldFormat) {

						case FRMR:
							printf(
									"\n\n ?!?!?!?! FRAME ERROR ?!?!?!?! :-/\n\n");
							break;

						case UI:
							registerLmnSubscriber(sourceAddr,
									((payload[7] >> 5) & 0x7),
									((payload[7] >> 1) & 0x7), payload + 10,
									32);
							//                    sourceAddr    receiveSequenceNumber      sendSequenceNumber       ID-Field      32 Bytes
							receiveReadyCounter = 0;
							receiveNotReadyCounter = 0;
							break;

						case I:
							setLmnSubscriberReceiveSequenceNumber(
									static_cast<unsigned short>(sourceAddr),
									static_cast<unsigned short>(((payload[7]
											>> 5) & 0x7)));
							setLmnSubscriberSendSequenceNumber(
									static_cast<unsigned short>(sourceAddr),
									static_cast<unsigned short>((payload[7] >> 1)
											& 0x7));
							tempSequenzNumber = getOwnReceiveSequenceNumber(
									static_cast<unsigned short>(sourceAddr));
							tempSequenzNumber < 7 ?
									tempSequenzNumber++ : tempSequenzNumber = 0;
							setOwnReceiveSequenceNumber(
									static_cast<unsigned short>(sourceAddr),
									tempSequenzNumber);
#if DETAILED_TEST
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
							printf("received: valid Data...    forwanding to main\n\n\n");
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#endif

							callbackToUpperLayer(destAddr, sourceAddr, protSel,
									payload + 10, payloadLength - 10 - 2 - 1);

#if DETAILED_TEST
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
							printf("Data forwanded :-) \n\n\n");
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#endif
							receiveReadyCounter = 0;
							receiveNotReadyCounter = 0;
							break;

						case RNR://frameLength = hdlcBuildFrame(sourceAddr, destAddr, protSel, RR, payload, 0);
#if DETAILED_TEST
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
							printf("received: R    N     R...    sending R R\n\n\n");
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#endif
							receiveNotReadyCounter++;
							if (receiveNotReadyCounter > 100) {
								if (receiveNotReadyCounter > 200) {
									receiveNotReadyCounter = 0;
								}
								delayedSending(sourceAddr, destAddr, protSel,
										RR, payload, 0, 100);
							} else
								delayedSending(sourceAddr, destAddr, protSel,
										RR, payload, 0, 0);

#if DETAILED_TEST
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
							printf("RR has been sent :-)\n\n\n");
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#endif
							receiveReadyCounter = 0;
							receiveNotReadyCounter = 0;

							break;

						case RR://frameLength = hdlcBuildFrame(sourceAddr, destAddr, protSel, RR, payload, 0);
#if DETAILED_TEST
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
							printf("R         R...    sending R R\n\n\n");

#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#endif
							switch (protSel) {
							case TLS_SML_COSEM:
								receiveReadyCounter++;
								if (receiveReadyCounter > 0) {
									receiveReadyCounter = 0;
									callbackToUpperLayer(destAddr, sourceAddr,
											protSel, payload, 0);
								} else
									delayedSending(sourceAddr, destAddr,
											protSel, RR, payload, 0, 0);
								break;

							default:
								receiveReadyCounter++;
								if (receiveReadyCounter > 100) {
									if (receiveReadyCounter > 200) {
										receiveReadyCounter = 0;
										// + Fehlerbehandlung
									}
									delayedSending(sourceAddr, destAddr,
											protSel, RR, payload, 0, 100);
								} else
									delayedSending(sourceAddr, destAddr,
											protSel, RR, payload, 0, 0);

							}

#if DETAILED_TEST
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
							printf("RR has been sent :-)\n\n\n");
#if WINDOWS
							SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#endif
#endif
							receiveNotReadyCounter = 0;
							break;

						case UA:

							if (channelOpenRequest == 1) {
								setConnectionType(
										static_cast<unsigned short>(sourceAddr),
										protSel);
								channelOpenRequest = 0;
#if DETAILED_TEST
								printf("\nKanalaufbau bestätigt\n");
#endif
							} else {
								setConnectionType(
										static_cast<unsigned short>(sourceAddr),
										NO_CONNECTION);
#if DETAILED_TEST
								printf("\nKanalabbbau bestätigt\n");
#endif
							}
							receiveReadyCounter = 0;
							receiveNotReadyCounter = 0;
							break;

						case DM:
							setConnectionType(
									static_cast<unsigned short>(sourceAddr),
									NO_CONNECTION);
#if TEST
							printf("\n\n DISCONNECTED MODE :-O\n\n\n");
#endif

							receiveReadyCounter = 0;
							receiveNotReadyCounter = 0;

							setTlnTlsState(sourceAddr, uninitialized);

							callbackToUpperLayer(destAddr, sourceAddr, protSel,
									payload, -200);

							break;

						default:
							printf("fieldFormat: 0x%02X\n", fieldFormat);
							receiveReadyCounter = 0;
							receiveNotReadyCounter = 0;
							throw 0xF5;
						}

#if DETAILED_TEST
						unsigned short aktiveTln[128];
						unsigned short anzahlAktiverTln = 0;

						locateActiveLmnSubscriber(aktiveTln, &anzahlAktiverTln);

						//aktive Teilnehmer ausgeben
						printf("\n\nhdlcReceiveData(): %d Teilnehmer aktiv: ", anzahlAktiverTln);
						for (int i = 0; i < anzahlAktiverTln; i++) {
							printf("0x%02X ", aktiveTln[i]);
						}
#endif

#if DETAILED_TEST
						unsigned char field[32];
						getTlnIdField(sourceAddr, field, 32);

						for (int i = 0; i < 32; i++) {
							printf("0x%02X ", field[i]);
							if ((i + 1) % 16 == 0)
							printf("\n");
						}
						printf("\n");
#endif

						protSel = interpretProtocolSelector(
								(payload[6] & 0xFF) >> 1); // nur wichtig für höhere Layer

					} else { //hier ggf. FCS falsch
						int index = 0;
						for (index = 0; index < (payloadLength - 1); index++) {
							if (payload[index] == 0x7E
									&& payload[index + 1] == 0x7E)
								break;
						}
						index++;
						if (index != (payloadLength)) {
							for (int i = 0; i < (payloadLength - index); i++)
								payload[i] = payload[i + index];
							payloadLength = payloadLength - index;
							fieldFormat = interpretControlFieldFormat(
									(payload[7] & 0xFF));
							if (fieldFormat == DM) {
								setConnectionType(
										static_cast<unsigned short>(sourceAddr),
										NO_CONNECTION);
								//deactivateSubscriber(static_cast<unsigned short>(sourceAddr));
								printf(
										"\n\n\n ****** W A R N I N G *******     Subscriber disconnected      ************ \n");
							}
						} else
							printf(
									"\n\n*******************************************************************\n");
						printf(
								"****************** Ein Fehler trat auf (FCS) **********************\n");
						printf(
								"*******************************************************************\n");
						delayedSending(sourceAddr, destAddr, protSel, RR,
								payload, 0, 0);
					}

				} while (nochmehr);
				delete payloadsave;
				broadcast = 0;
				recall = 0;
			}
		}

		catch (int e) {
			printf(
					"\n\n\n ****** E R R O R *******     Fehler in hdlcReceiveData() - Fehlercode: 0x%02X     ************ \n",
					e);
			switch (e) {
			case 0xF1:
				printf(
						"\n 0xF1: Fehlerhafte HDLC-Struktur (Beginn oder Ende des Telegramms ungleich 0x7E)\n");
				break;
			case 0xF2:
				printf("\n 0xF2: Fehlerhafte CRC-Checksumme (FCS)\n");
				break;
			case 0xF3:
				printf("\n 0xF3: Fehlerhafte CRC-Checksumme (HCS)\n");
				break;
			case 0xF4:
				printf("\n 0xF4: Keine Payload vorhanden\n");
				break;
			case 0xF5:
				printf(
						"\n 0xF5: Unbekanntes Control Field Format (erwartet: I, RR, RNR, SNRM, DISC, UA, DM, FRMR oder UI)\n");
				break;
			default:
				printf("\n  :-o");
			}
		}
	}
	return;
}

int hdlcBuildFrame(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, controlFieldFormat controlField,
		unsigned char * payload, int payloadLength) {

#if DETAILED_TEST
	printf("\n\nSchon in hdlcBuildFrame vor dem Umbauen in hdlc-Frames, also immer noch rein Payload\n");
	for (int i = 0; i < payloadLength; i++) {
		printf("0x%02X ", payload[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
#endif
	unsigned short headerCheckSequence, frameCheckSequence;
	unsigned char* type_3_Frame = new unsigned char[1 + 2 + 2 + 2 + 1 + 2
			+ payloadLength + 2 + 1];
	unsigned short frameFormatBuild, frameLength, tempSequenzNumber;
	if (payloadLength != 0) {
		frameFormatBuild = static_cast<unsigned short>(payloadLength + 11);	//Lengt Subfield (Bit0-Bit10) = static_cast<unsigned short>(payloadLength + 11);	//Lengt Subfield (Bit0-Bit10)
		frameLength = payloadLength + 13;
	} else {
		frameFormatBuild = static_cast<unsigned short>(9);//Length Subfield (Bit0-Bit10) = static_cast<unsigned short>(9);	 //Lengt Subfield (Bit0-Bit10)
		frameLength = 11;
	}
	frameFormatBuild |= (FRAME_FORMAT << 12);//Frame Format = 1010 (0xA)	(Bit11)
	frameFormatBuild &= ~(1 << 11);	//Optional Segmentation Subfield = 0 (Bit12-Bit15)
	type_3_Frame[0] = START_FLAG;									//Start Flag
	type_3_Frame[1] = static_cast<unsigned char>(frameFormatBuild >> 8);//Frame Format (2 Bytes)
	type_3_Frame[1 + 1] = static_cast<unsigned char>(frameFormatBuild & 0xFF);
	type_3_Frame[3] = (destAddr << 1);			//Destination Address (2 Bytes)
	type_3_Frame[3 + 1] = ((protSel << 1) | 0x1);		//	Protocol Selector
	type_3_Frame[5] = (sourceAddr << 1);			//Source Address (2 Bytes)
	type_3_Frame[5 + 1] = ((protSel << 1) | 0x1);		//	Protocol Selector

	switch (controlField) {
	case I:
		type_3_Frame[7] = (getOwnReceiveSequenceNumber(destAddr) << 5)
				| (1 << 4) | ((getOwnSendSequenceNumber(destAddr) & 0x7) << 1);
		tempSequenzNumber = getOwnSendSequenceNumber(
				static_cast<unsigned short>(destAddr));
		tempSequenzNumber < 7 ? tempSequenzNumber++ : tempSequenzNumber = 0;
		setOwnSendSequenceNumber(static_cast<unsigned short>(destAddr),
				tempSequenzNumber);
		break;
	case UI:
		type_3_Frame[7] = 0x13;
		break;
	case RR:
		type_3_Frame[7] = ((getOwnReceiveSequenceNumber(destAddr) << 5)
				| (1 << 4)) | 0x1;
		break;
	case RNR:
		type_3_Frame[7] = ((getOwnReceiveSequenceNumber(destAddr) << 5)
				| (1 << 4)) | 0x5;
		break;
	default:
		type_3_Frame[7] = controlField;
		break;
	}

	headerCheckSequence = calculateCrc(type_3_Frame + 1, 7);
	type_3_Frame[8] = static_cast<unsigned char>(headerCheckSequence >> 8);	//HCS (2 Bytes)
	type_3_Frame[8 + 1] =
			static_cast<unsigned char>(headerCheckSequence & 0xFF);

	if (payloadLength != 0) {
		for (int i = 0; i < payloadLength; i++)
			type_3_Frame[10 + i] = payload[i];						//Payload

		frameCheckSequence = calculateCrc(type_3_Frame + 1,
				7 + 2 + payloadLength);
		type_3_Frame[frameLength - 3] =
				static_cast<unsigned char>(frameCheckSequence >> 8);//FCS (2 Bytes)
		type_3_Frame[frameLength - 2] =
				static_cast<unsigned char>(frameCheckSequence & 0xFF);
	}
	type_3_Frame[frameLength - 1] = STOP_FLAG;//Stop Flag, wenn keine Payload und keine FCS

	for (int i = 0; i < (frameLength); i++)
		payload[i] = type_3_Frame[i];

#if DETAILED_TEST
	printf("\nhdlcBuildFrame: generated frame in hdlcBuildFrame():\n");
#if DETAILED_TEST
	for (int i = 0; i < (frameLength); i++) {
		printf("0x%02X ", type_3_Frame[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
	printf("\n");
#endif
#endif

#if DETAILED_TEST
	printf("hdlcBuildFrame: test of checkCrc()-function:\n");
#if CRC_FALSCH
	type_3_Frame[(frameLength - 1) - 1] = 0xFF;
#endif
#if DETAILED_TEST
	if (checkCrc(type_3_Frame + 1, frameLength - 1 - 1)) //dadurch, type_3_Frame + 1 und das 0x7E am Ende fehlen
	printf("checksum correct\n");
	else
	printf("checksum wrong\n");
#endif
#endif

	delete[] type_3_Frame;
	return frameLength;
}

controlFieldFormat interpretControlFieldFormat(unsigned short controlField) {
	if (((controlField & 0xFF) & 0x1) == 0x00)
		return I;
	if (((controlField & 0xFF) & 0xF) == 0x01)
		return RR;
	if (((controlField & 0xFF) & 0xF) == 0x05)
		return RNR;
	if (((controlField & 0xFF) & 0xEF) == 0x83)
		return SNRM;
	if (((controlField & 0xFF) & 0xEF) == 0x43)
		return DISC;
	if (((controlField & 0xFF) & 0xEF) == 0x63)
		return UA;
	if (((controlField & 0xFF) & 0xEF) == 0x0F)
		return DM;
	if (((controlField & 0xFF) & 0xEF) == 0x87)
		return FRMR;
	if (((controlField & 0xFF) & 0xEF) == 0x03)
		return UI;
	return FORMAT_ERROR;
}
protocolSelector interpretProtocolSelector(unsigned short selector) {
	switch (selector & 0xFF) {
	case 0x1:
		return TLS_SML_COSEM;
	case 0x2:
		return TLS;
	case 0x3:
		return SML_COSEM;
	case 0x4:
		return SML_EDL;
	case 0x5:
		return SML_SYM;
	case 0x6:
		return SYM_HDLC;
	case 0x7:
		return TLS_SML_EDL;
	case 0x8:
		return TLS_SML_SYM;
	default:
		return SELECTOR_ERROR;
	}
}
bool checkCrc(unsigned char* data, int dataLength) {
	unsigned short calculatedCrc, appendedCrc;
	appendedCrc = (data[(dataLength - 1) - 1] << 8)
			| (data[(dataLength - 1)] & 0xFF); //bei Indizierung (dataLength - 1)
	calculatedCrc = calculateCrc(data, dataLength - 2);	//Berechnung ohne die 2 Byte CRC
#if DETAILED_TEST
			printf("appendedCrc = %04X\n", appendedCrc);
			printf("calculatedCrc = %04X\n", calculatedCrc);
#endif
	return appendedCrc == calculatedCrc;
}

void delayedSending(unsigned char destAddr, unsigned sourceAddr,
		protocolSelector protSel, controlFieldFormat controlField,
		unsigned char *payload, int payloadLength, int delay_ms) {

	int frameLength;

	dataToSend.destAddr = destAddr;
	dataToSend.sourceAddr = sourceAddr;
	dataToSend.protSel = protSel;
	dataToSend.controlField = controlField;

	for (int i = 0; i < payloadLength; i++)
		dataToSend.payload[i] = payload[i];
	dataToSend.payload = payload;
	dataToSend.payloadLength = payloadLength;
	dataToSend.delay_ms = delay_ms;

	if (dataToSend.delay_ms > 0)
		usleep(dataToSend.delay_ms * 1000);
	frameLength = hdlcBuildFrame(dataToSend.destAddr, dataToSend.sourceAddr,
			dataToSend.protSel, dataToSend.controlField, dataToSend.payload,
			dataToSend.payloadLength);

#if PRINT_IO
	printf("\ndelayedSending: Data that will be sent delayed:\n");
	for (int i = 0; i < (frameLength); i++) {
		printf("0x%02X ", dataToSend.payload[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
	printf("\n");
#endif
	while (broadcastAktiv == 1)
		;
	sendenAktiv = 1;
	RS232SendFunction(0, 0, dataToSend.protSel, dataToSend.payload, frameLength,
			hdlcReceiveData);
	sendenAktiv = 0;
#if DETAILED_TEST
	printf("\n\n<<<<<<<<<<<<<<<<<<<<<<<<< hier kam was >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
#endif
	return;
}

void hdlcInit(void) {
	lmnInit();
#if TEST
	hdlcBroadcastTimerStart(100);
#else
	hdlcBroadcastTimerStart(100);
#endif
	return;
}
void hdlcTerminate(void) {
	lmnTerminate();
	hdlcRecallTimerStop();
	hdlcBroadcastTimerStop();
	return;
}
void hdlcBroadcastTimerStart(int milliSeconds) {
	startTimer(milliSeconds, 0, timerEventBroadcast);
	return;
}
void hdlcBroadcastTimerStop(void) {
	return;
}
void hdlcRecallTimerStart(int milliSeconds) {
	startTimer(milliSeconds, 0, timerEventRecall);
	return;
}
void hdlcRecallTimerStop(void) {
	return;
}
void doHdlcBroadcast(void) {
	hdlcBroadcastTimerStop();
	if (sendenAktiv == 1) {
		hdlcBroadcastTimerStart(250);
		return;
	}
	broadcast = 1;
	broadcastAktiv = 1;
#if TEST
	printf("\n\nHDLC BROADCAST");
#endif

	unsigned short* actviveSubscriber = new unsigned short[128];
	unsigned short anzahlAktiverTln;
	int payloadLength, frameLength;

	if (global_sim != 0)
		RS232SendFunction = fakeRS232DataRequest; // TEST
	else
		RS232SendFunction = RS232DataRequest;

	locateActiveLmnSubscriber(actviveSubscriber, &anzahlAktiverTln);
	payloadLength = anzahlAktiverTln * 32;
	while (anzahlAktiverTln > 0) {
		getTlnIdField(actviveSubscriber[anzahlAktiverTln - 1],
				localPayloadBuffer + (anzahlAktiverTln - 1) * 32, 32);
		localPayloadBuffer[((anzahlAktiverTln - 1) * 32) + 1] = 0x00;
		anzahlAktiverTln--;
	}

#if DETAILED_TEST

	printf("Der Puffer vor dem Umbauen in hdlc-Frames, also rein Payload\n");

	for (int i = 0; i < payloadLength; i++) {
		printf("0x%02X ", localPayloadBuffer[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}

#endif
	frameLength = hdlcBuildFrame(0x7F, 0x1,
			static_cast<protocolSelector>(ADDR_ALLOC), UI, localPayloadBuffer,
			payloadLength);

#if DETAILED_TEST
	printf("payloadLength = %d   frameLength = %d \n", payloadLength, frameLength);

	for (int i = 0; i < frameLength; i++) {
		printf("0x%02X ", localPayloadBuffer[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
#endif
#if PRINT_IO
	printf("\ndoHdlcBroadcast: Data that will be sent:\n");
	for (int i = 0; i < (frameLength); i++) {
		printf("0x%02X ", localPayloadBuffer[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
	printf("\n");
#endif
	RS232SendFunction(0, 0, static_cast<protocolSelector>(ADDR_ALLOC),
			localPayloadBuffer, frameLength, hdlcReceiveData);
	broadcastAktiv = 0;

	delete[] actviveSubscriber;

	if (firstRun == 1)
		firstRun = 0;
	else {
		if (noSubscriber == 1)
			hdlcBroadcastTimerStart(SHORT_BROADCAST_TIMEOUT);
		else
			hdlcRecallTimerStart(RECALL_TIMEOUT);
	}
	return;
}
void doHdlcRecall(void) {
	hdlcRecallTimerStop();
	if (sendenAktiv == 1) {
		hdlcRecallTimerStart(250);
		return;
	}
	recall = 1;
#if TEST
	printf("\n\nHDLC RECALL");
#endif
	unsigned short* actviveSubscriber = new unsigned short[128];
	unsigned short anzahlAktiverTln;
	int payloadLength, frameLength;

	if (global_sim != 0)
		RS232SendFunction = fakeRS232DataRequest; // TEST
	else
		RS232SendFunction = RS232DataRequest;

	locateActiveLmnSubscriber(actviveSubscriber, &anzahlAktiverTln);
	for (int i = 0; i < anzahlAktiverTln; i++)
		deactivateSubscriber(static_cast<unsigned short>(actviveSubscriber[i]));

	if (anzahlAktiverTln != 0) {
		broadcastAktiv = 1;
		payloadLength = anzahlAktiverTln * 32;
		while (anzahlAktiverTln > 0) {
			getTlnIdField(actviveSubscriber[anzahlAktiverTln - 1],
					localPayloadBuffer + (anzahlAktiverTln - 1) * 32, 32);
			localPayloadBuffer[((anzahlAktiverTln - 1) * 32) + 1] =
					static_cast<unsigned char>(anzahlAktiverTln); //Zeitschlitze von 0x1 - ... vergeben
			anzahlAktiverTln--;
		}
#if DETAILED_TEST

		printf("Der Puffer vor dem Umbauen in hdlc-Frames, also rein Payload\n");

		for (int i = 0; i < payloadLength; i++) {
			printf("0x%02X ", localPayloadBuffer[i]);
			if ((i + 1) % 16 == 0)
			printf("\n");
		}

#endif

		frameLength = hdlcBuildFrame(0x7F, 0x1,
				static_cast<protocolSelector>(ADDR_CHECK), UI,
				localPayloadBuffer, payloadLength);

#if DETAILED_TEST
		printf("payloadLength = %d   frameLength = %d \n", payloadLength, frameLength);

		for (int i = 0; i < frameLength; i++) {
			printf("0x%02X ", localPayloadBuffer[i]);
			if ((i + 1) % 16 == 0)
			printf("\n");
		}
		printf("\n");
#endif

#if PRINT_IO
		printf("\ndoHdlcRecall: Data that will be sent:\n");
		for (int i = 0; i < (frameLength); i++) {
			printf("0x%02X ", localPayloadBuffer[i]);
			if ((i + 1) % 16 == 0)
			printf("\n");
		}
		printf("\n");
#endif
		RS232SendFunction(0, 0, static_cast<protocolSelector>(ADDR_CHECK),
				localPayloadBuffer, frameLength, hdlcReceiveData);
		locateActiveLmnSubscriber(actviveSubscriber, &anzahlAktiverTln);
		if (anzahlAktiverTln == 0) {
			noSubscriber = 1;
			hdlcBroadcastTimerStart(SHORT_BROADCAST_TIMEOUT);
		} else {
			noSubscriber = 0;
			hdlcBroadcastTimerStart(BROADCAST_TIMEOUT);
		}

		broadcastAktiv = 0;
	}

	else {
		noSubscriber = 1;
		hdlcBroadcastTimerStart(SHORT_BROADCAST_TIMEOUT);
	}
	delete[] actviveSubscriber;
	return;
}

void storeCurrentData(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength,
		func_ptr callback) {
	storedDestAddr = destAddr;
	storedSourceAddr = sourceAddr;
	storedProtSel = protSel;
	for (int i = 0; i < payloadLength; i++)
		storedPayload[i] = payload[i];
	storedPayloadLength = payloadLength;
	storedCallback = callback;
	return;
}

unsigned char getHdlcAddress(unsigned char * meterId) {

	unsigned short* actviveSubscriber = new unsigned short[128];
	unsigned short anzahlAktiverTln;
	unsigned char hdlcAddress = 0x00;
	unsigned char* idField = new unsigned char[12];
	int idx;

	locateActiveLmnSubscriber(actviveSubscriber, &anzahlAktiverTln);
#if PRINT_IO
	printf("\nAktive Tln: %d\n", anzahlAktiverTln);
#endif
	for (int i = 0; i < anzahlAktiverTln; i++) {
#if PRINT_IO
		printf("\n %d. Durchlauf\n", i+1);
#endif
		getTlnIdField(actviveSubscriber[i], idField, 12);
		for (idx = 0; idx < 10; idx++) {
#if PRINT_IO
			printf("\nIst id = 0x%02X    gesuchte ID =  0x%02X ", idField[idx + 2], meterId[idx]);
#endif
			if (idField[idx + 2] != meterId[idx])
				break;
		}
		if (idx == 10) {
			hdlcAddress = actviveSubscriber[i];
			i = 1000; //stop outer loop
			break;
		}
	}
	delete[] actviveSubscriber;
	delete[] idField;
	return hdlcAddress;
}
