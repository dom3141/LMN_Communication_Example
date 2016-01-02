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

#include "hdlc.h"
#include "funcPtr.h"
#include "test.h"
#include "fakeRS232.h"
#include "lmnsubscriber.h"
#include "lmnadmin.h"
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "RS232.h"
#include "tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include "sml.h"
#include "sym.h"
#include "server.h"

void readConfig();
void readMeterValue_cb(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);

#define MAX_LINES 11
#define MAX_SYMBOL 65
#define DEFAULT_PORT 50001
int global_port = DEFAULT_PORT;
int global_tcp = 0;
int global_sim = 0;
int global_log = 1;
int global_clear = 1;
int global_debug = 0;
int global_details = 0;
int global_moredetails = 0;
int global_flush = 1;
int global_RNR_us = 1;
char line[MAX_LINES][MAX_SYMBOL];
int returnValueSize = 17;
int retStringCount = 0;
char socketMessage[65];
int socketMessageSize = 0;
char returnValue[30] = "METER ID UNKNOWN\n";
char noMeterFound[] = "NO METER FOUND\n";
char wrongProtSel[] = "UNKNOWN PROTOCOL SELECTOR\n";
char nix[] = "";
int globalDebug = 0, fileOpened = 0;
FILE *logfile;
bool symDone = false, symSuccess = false;

int main(int argc, char *argv[]) {

	unsigned char MASKE = 0;
	readConfig();
	if (argc > 1)
		if ((strcmp(argv[1], "debug")) == 0)
			globalDebug = 1;
	char serialPort[] = "ANY";
	RS232Init(serialPort, 921600);
	hdlcInit();
	tlsInit();

	unsigned char message[2048];
	int messageLength;
	int destAddr, sourceAddr;
	unsigned short* actviveSubscriber = new unsigned short[128];
	unsigned short anzahlAktiverTln;
	unsigned char meterId[10]; // = { 0x0A, 0x01, 0x45, 0x4D, 0x48, 0x00, 0x00, 0x4A, 0xF1, 0xFD };
	unsigned char obis[9]; // = { 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF, 0x80, 0x02, 0x00 };
	unsigned char masterKey[16];
	int emhBug;
	unsigned char clientCert[MAX_CERT_SIZE];
	unsigned char clientKey[MAX_CERT_SIZE];
	unsigned char serverCert[MAX_CERT_SIZE];
	unsigned char serverKey[MAX_CERT_SIZE];
	int clientCertLength, clientKeyLength, serverCertLength, serverKeyLength;
	unsigned char activeMeterIds[128][10];
	unsigned char zeichen;
	command befehl;
	char separator[] = ",;()\" \"";
	char *ptr;
	char commands[5][MAX_COMMAND_SIZE]; // = { 0 };
	char errorMsg[] = "ERROR\n";
	char busyMsg[] = "BUSY\n";
	char okMsg[] = "OK\n";
	char readyMsg[] = "READY\n";
	char ethReturnString[2048];

	int sockfd, connfd, portno, i, j;
	socklen_t clilen;
	char buffer[MAX_BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if (global_log != 0) {
		if (global_clear == 0) {
			logfile = fopen("logfile.log", "a+");
			fwrite("\n\n\n\n\n", sizeof(unsigned char), 5, logfile);
		} else
			logfile = fopen("logfile.log", "w+");
		if (logfile != NULL) {
			fwrite(
					"*******************************************************************************",
					sizeof(unsigned char), 79, logfile);
			fwrite(
					"\n*******************************************************************************",
					sizeof(unsigned char), 80, logfile);
			fwrite(
					"\n**                    New      Session      Initiated                        **",
					sizeof(unsigned char), 80, logfile);
			fwrite(
					"\n*******************************************************************************",
					sizeof(unsigned char), 80, logfile);
			fwrite(
					"\n*******************************************************************************\n",
					sizeof(unsigned char), 81, logfile);
			fileOpened = 1;
		} else {
			//fehler
		}
	}
	if (global_tcp != 0)
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
	else
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 50001;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(global_port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}

	if (global_tcp != 0) {
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		connfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	}

	do {
		destAddr = 0x3A;
		sourceAddr = 0x1;
		func_ptr callback = readMeterValue_cb;
		bzero(buffer, MAX_BUFFER_SIZE);
		bzero(commands, 5 * MAX_BUFFER_SIZE);
		bzero(meterId, 10);

		if (global_tcp != 0)
			n = recv(connfd, buffer, MAX_BUFFER_SIZE, 0);
		else {
			clilen = sizeof(cli_addr);
			n = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0,
					(struct sockaddr *) &cli_addr, &clilen);
		}
		if (global_tcp != 0) {
			if (n == 0) {
				listen(sockfd, 5);
				clilen = sizeof(cli_addr);
				connfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
				if (connfd < 0)
					error("ERROR on accept");
			}
			if (n < 0) {
				listen(sockfd, 5);
				clilen = sizeof(cli_addr);
				connfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
				if (connfd < 0)
					error("ERROR on accept");
			}
		}
		i = 0;
		j = 0;
		ptr = strtok(buffer, separator);
		while (ptr != NULL) {
			while (*ptr != 0 && *ptr != '\n') {
				if (i == 0)
					commands[i][j] = tolower(*(ptr));
				else
					commands[i][j] = toupper(*(ptr));
				ptr++;
				j++;
			}
			i++;
			j = 0;
			ptr = strtok(NULL, separator);
		}

		befehl = readCommand(commands[0]);
		if (befehl == GETVALUE) {
			//read MeterID
			for (int idx = 0;
					commands[1][idx] != '\0'
							&& idx < static_cast<int>(2 * sizeof(meterId));
					idx++) {
				zeichen = static_cast<unsigned char>(commands[1][idx]);
				zeichen = zeichen < 0x40 ? zeichen - 0x30 : zeichen - 0x37;
				if (idx % 2 == 0)
					meterId[idx / 2] = (zeichen << 4);
				else
					meterId[idx / 2] |= (zeichen);
			}
			//read Obis
			for (int idx = 0;
					commands[2][idx] != '\0'
							&& idx < static_cast<int>(2 * sizeof(obis));
					idx++) {
				zeichen = static_cast<unsigned char>(commands[2][idx]);
				zeichen = zeichen < 0x40 ? zeichen - 0x30 : zeichen - 0x37;
				if (idx % 2 == 0)
					obis[idx / 2] = (zeichen << 4);
				else
					obis[idx / 2] |= (zeichen);
			}

#if PRINT_IO
			printf("\n\n\nDiese MeterID wurde der Main übermittelt:\n");
			for (int idx = 0; idx < 10; idx++)
			printf("%02X", meterId[idx]);
			printf("\n\n\n");
#endif

			if (global_tcp != 0)
				send(connfd, busyMsg, 5, 0);
			else {
				clilen = sizeof(cli_addr);
				sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
						sizeof(cli_addr));
			}

			locateActiveLmnSubscriber(actviveSubscriber, &anzahlAktiverTln);
#if PRINT_IO
			printf("\nAktive Teilnehmer sind: ");
			for (int i = 0; i < anzahlAktiverTln; i++)
			printf("0x%02X ", actviveSubscriber[i]);
			printf("\n");
#endif
#if PRINT_IO
			destAddr = getHdlcAddress(meterId);
			printf("\n\nDer Zaehler hat die HDLC-Adresse: 0x%02X\n\n", destAddr);
#endif

			if (getHdlcAddress(meterId) == 0x00) {
				returnValueSize = sprintf(returnValue, "Meter ID unknown\n");
			} else {
				messageLength = smlGenerateGetProcParameterRequest(meterId,
						message, sizeof(message), obis, sizeof(obis));

				if (global_sim != 0)
					lmnSendData(meterId, SML_COSEM, message, messageLength,
							callback);
				else {
					befehl = readCommand(commands[3]);
					if (befehl == TLSSMLCOSEM)
						tlsSendData(meterId, TLS_SML_COSEM, message,
								messageLength, callback);
					else if (befehl == SMLCOSEM)
						lmnSendData(meterId, SML_COSEM, message, messageLength,
								callback);
					else {
						for (int i = 0; i < sizeof(wrongProtSel); i++)
							returnValue[i] = wrongProtSel[i];
						returnValueSize = sizeof(wrongProtSel);
					}
				}
			}

			if (global_sim != 0)
				sleep(1);
			if (global_tcp != 0)
				send(connfd, returnValue, returnValueSize, 0);
			else
				sendto(sockfd, returnValue, returnValueSize, 0,
						(struct sockaddr *) &cli_addr, sizeof(cli_addr));
			returnValueSize = sprintf(returnValue, "METER ID UNKNOWN\n");
		} else {
			switch (befehl) {
			default:
				if (global_tcp != 0)
					send(connfd, errorMsg, 6, 0);
				else
					sendto(sockfd, errorMsg, 6, 0,
							(struct sockaddr *) &cli_addr, sizeof(cli_addr));
				break;
			case BROADCAST:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				locateActiveLmnSubscriber(actviveSubscriber, &anzahlAktiverTln);
				retStringCount = 0;

				if (anzahlAktiverTln == 0) {
					if (global_tcp != 0)
						send(connfd, noMeterFound, sizeof(noMeterFound), 0);
					else
						sendto(sockfd, noMeterFound, sizeof(noMeterFound), 0,
								(struct sockaddr *) &cli_addr,
								sizeof(cli_addr));
				} else {
					for (int i = 0; i < anzahlAktiverTln; i++) {
						getMeterId(actviveSubscriber[i], activeMeterIds[i]);
						for (int ii = 0; ii < 10; ii++) {
							ethReturnString[retStringCount] =
									(((activeMeterIds[i][ii] >> 4) & 0x0F)
											+ 0x30);
							ethReturnString[retStringCount] =
									ethReturnString[retStringCount] < 0x3A ?
											ethReturnString[retStringCount] :
											(ethReturnString[retStringCount]
													+ 0x07);
							ethReturnString[++retStringCount] =
									(((activeMeterIds[i][ii]) & 0x0F) + 0x30);
							ethReturnString[retStringCount] =
									ethReturnString[retStringCount] < 0x3A ?
											ethReturnString[retStringCount] :
											(ethReturnString[retStringCount]
													+ 0x07);
							retStringCount++;
						}
						ethReturnString[retStringCount++] = ';';
					}
					ethReturnString[--retStringCount] = '\n';
					if (global_tcp != 0)
						send(connfd, ethReturnString, retStringCount + 1, 0);
					else
						sendto(sockfd, ethReturnString, retStringCount + 1, 0,
								(struct sockaddr *) &cli_addr,
								sizeof(cli_addr));
				}
				break;
			case DISCONNECT:
				if (global_tcp != 0)
					send(connfd, okMsg, 3, 0);
				else
					sendto(sockfd, okMsg, 3, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				break;
			case CONNECT:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				break;
			case QUIT:
				if (global_tcp != 0)
					send(connfd, okMsg, 3, 0);
				else
					sendto(sockfd, okMsg, 3, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				printf("terminated\n");
				break;
			case CLEAR:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				if (global_log != 0) {
					fileOpened = 0;
					fclose(logfile);
					logfile = fopen("logfile.log", "w+");
					if (logfile != NULL) {
						fwrite(
								"*******************************************************************************",
								sizeof(unsigned char), 79, logfile);
						fwrite(
								"\n*******************************************************************************",
								sizeof(unsigned char), 80, logfile);
						fwrite(
								"\n**                         LOGFILE    CLEARED                                **",
								sizeof(unsigned char), 80, logfile);
						fwrite(
								"\n*******************************************************************************",
								sizeof(unsigned char), 80, logfile);
						fwrite(
								"\n*******************************************************************************\n",
								sizeof(unsigned char), 81, logfile);
						fileOpened = 1;
					} else {
						//error
					}
				}
				if (global_tcp != 0)
					send(connfd, okMsg, 3, 0);
				else
					sendto(sockfd, okMsg, 3, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				break;

			case LOADSERVERCERT:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				certHandle(commands[1], meterId, masterKey, serverCert,
						&serverCertLength, &emhBug);
				storeCertsAndKeys(meterId, serverCert,
						static_cast<unsigned int>(serverCertLength), "crypto",
						SERVER_CERT);
				MASKE |= (1 << 3);
				if (MASKE == 0x0F) {
					symSuccess = symHandshake(meterId, masterKey, "crypto",
							(emhBug == 1));
					symDone = true;
					MASKE = 0x00;
				}
				if (symDone && symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys successfully transmitted\n");
				else if (symDone && !symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys transmission failed\n");
				else
					socketMessageSize = sprintf(socketMessage, "READY\n");
				if (global_tcp != 0)
					send(connfd, socketMessage, socketMessageSize - 1, 0);
				else
					sendto(sockfd, socketMessage, socketMessageSize - 1, 0,
							(struct sockaddr *) &cli_addr, sizeof(cli_addr));
				symSuccess = false;
				symDone = false;
				break;
			case LOADSERVERKEY:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				certHandle(commands[1], meterId, masterKey, serverKey,
						&serverKeyLength, &emhBug);
				storeCertsAndKeys(meterId, serverKey,
						static_cast<unsigned int>(serverKeyLength), "crypto",
						SERVER_KEY);
				MASKE |= (1 << 2);
				if (MASKE == 0x0F) {
					symSuccess = symHandshake(meterId, masterKey, "crypto",
							(emhBug == 1));
					symDone = true;
					MASKE = 0x00;
				}
				if (symDone && symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys successfully transmitted\n");
				else if (symDone && !symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys transmission failed\n");
				else
					socketMessageSize = sprintf(socketMessage, "READY\n");
				if (global_tcp != 0)
					send(connfd, socketMessage, socketMessageSize - 1, 0);
				else
					sendto(sockfd, socketMessage, socketMessageSize - 1, 0,
							(struct sockaddr *) &cli_addr, sizeof(cli_addr));
				symSuccess = false;
				symDone = false;
				break;
			case LOADCLIENTCERT:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				certHandle(commands[1], meterId, masterKey, clientCert,
						&clientCertLength, &emhBug);
				storeCertsAndKeys(meterId, clientCert,
						static_cast<unsigned int>(clientCertLength), "crypto",
						CLIENT_CERT);
				MASKE |= (1 << 1);
				if (MASKE == 0x0F) {
					symSuccess = symHandshake(meterId, masterKey, "crypto",
							(emhBug == 1));
					symDone = true;
					MASKE = 0x00;
				}
				if (symDone && symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys successfully transmitted\n");
				else if (symDone && !symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys transmission failed\n");
				else
					socketMessageSize = sprintf(socketMessage, "READY\n");
				if (global_tcp != 0)
					send(connfd, socketMessage, socketMessageSize - 1, 0);
				else
					sendto(sockfd, socketMessage, socketMessageSize - 1, 0,
							(struct sockaddr *) &cli_addr, sizeof(cli_addr));
				symSuccess = false;
				symDone = false;
				break;
			case LOADCLIENTKEY:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				certHandle(commands[1], meterId, masterKey, clientKey,
						&clientKeyLength, &emhBug);
				storeCertsAndKeys(meterId, clientKey,
						static_cast<unsigned int>(clientKeyLength), "crypto",
						CLIENT_KEY);
				MASKE |= (1 << 0);
				if (MASKE == 0x0F) {
					symSuccess = symHandshake(meterId, masterKey, "crypto",
							(emhBug == 1));
					symDone = true;
					MASKE = 0x00;
				}
				if (symDone && symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys successfully transmitted\n");
				else if (symDone && !symSuccess)
					socketMessageSize = sprintf(socketMessage,
							"READY. Certs & Keys transmission failed\n");
				else
					socketMessageSize = sprintf(socketMessage, "READY\n");
				if (global_tcp != 0)
					send(connfd, socketMessage, socketMessageSize - 1, 0);
				else
					sendto(sockfd, socketMessage, socketMessageSize - 1, 0,
							(struct sockaddr *) &cli_addr, sizeof(cli_addr));
				symSuccess = false;
				symDone = false;
				break;
			case CRYPTOINIT:
				if (global_tcp != 0)
					send(connfd, busyMsg, 5, 0);
				else
					sendto(sockfd, busyMsg, 5, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				sleep(1);
				if (global_tcp != 0)
					send(connfd, readyMsg, 6, 0);
				else
					sendto(sockfd, readyMsg, 6, 0,
							(struct sockaddr *) &cli_addr, sizeof(cli_addr));
				break;
			case NOTHING:
				if (global_tcp != 0)
					send(connfd, nix, 1, 0);
				else
					sendto(sockfd, nix, 1, 0, (struct sockaddr *) &cli_addr,
							sizeof(cli_addr));
				break;
			}
		}
	} while (befehl != QUIT);
#if PRINT_IO
	cout << "\n*** press enter for terminating program ***\n" << endl;
	std::cin.ignore();
#endif
	if (global_tcp != 0)
		close(connfd);
	close(sockfd);
	tlsTerminate();
	hdlcTerminate();
	RS232Terminate();
	fileOpened = 0;
	if (global_log != 0)
		fclose(logfile);

	return 0;
}

void readMeterValue_cb(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength) {

	unsigned char pay2[] = { 0x1B, 0x1B, 0x1B, 0x1B, 0x01, 0x01, 0x01, 0x01,
			0x76, 0x05, 0x00, 0x00, 0x00, 0x06, 0x62, 0x00, 0x62, 0x00, 0x72,
			0x63, 0x01, 0x01, 0x76, 0x01, 0x0B, 0x0A, 0x01, 0x45, 0x4D, 0x48,
			0x00, 0x00, 0x53, 0xFC, 0x30, 0x02, 0x30, 0x0B, 0x0A, 0x01, 0x45,
			0x4D, 0x48, 0x00, 0x00, 0x53, 0xFC, 0x30, 0x72, 0x62, 0x01, 0x64,
			0x05, 0x52, 0x58, 0x01, 0x63, 0x59, 0x6C, 0x00, 0x76, 0x05, 0x00,
			0x00, 0x00, 0x07, 0x62, 0x00, 0x62, 0x00, 0x72, 0x63, 0x05, 0x01,
			0x73, 0x0B, 0x0A, 0x01, 0x45, 0x4D, 0x48, 0x00, 0x00, 0x53, 0xFC,
			0x30, 0x73, 0x0A, 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF, 0x80, 0x02,
			0x00, 0x03, 0x00, 0x01, 0x03, 0x00, 0x02, 0x73, 0x0A, 0x01, 0x00,
			0x01, 0x08, 0x00, 0xFF, 0x80, 0x02, 0x00, 0x01, 0x72, 0x73, 0x03,
			0x00, 0x01, 0x72, 0x62, 0x01, 0x07, 0x01, 0x00, 0x01, 0x08, 0x00,
			0xFF, 0x01, 0x73, 0x03, 0x00, 0x02, 0x72, 0x62, 0x01, 0x63, 0x3C,
			0x7F, 0x01, 0x63, 0x83, 0x0C, 0x00, 0x76, 0x05, 0x00, 0x00, 0x00,
			0x08, 0x62, 0x00, 0x62, 0x00, 0x72, 0x63, 0x02, 0x01, 0x71, 0x01,
			0x63, 0xB1, 0xA4, 0x00, 0x00, 0x00, 0x1B, 0x1B, 0x1B, 0x1B, 0x1A,
			0x02, 0xF3, 0x8E };

	unsigned char message[2048];
	unsigned char meterId[10] = { 0 };
	unsigned char obis[9] = { 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF, 0x80, 0x02,
			0x00 };

	unsigned char* fromMeterId = new unsigned char[10];
	int errorCode = 0;
	if (global_debug == 1)
		printf("\n\nreadMeterValue_cb called...\n\n");

	if (payloadLength == -100) {
		returnValueSize = sprintf(returnValue, "Frame Error\n");
	} else if (payloadLength == -200) {
		returnValueSize = sprintf(returnValue, "Disconnected Mode\n");
	} else if (payloadLength == 0) {
		getMeterId(sourceAddr, fromMeterId);
		lmnSendData(fromMeterId, protSel, payload, 0, readMeterValue_cb);
	} else {

		if (global_debug == 1) {
			printf("\n\nReceived SML-Message:\n\n");
			for (int i = 0; i < payloadLength; i++) {
				printf("0x%02X ", payload[i]);
				if (i != 0 && (i + 1) % 16 == 0) {
					printf("\n");
				}
			}
			printf("\n\n");
		}

		getMeterId(sourceAddr, fromMeterId);
		int eMeterValue = 0;
		//do this ten times with dummy messages as a workaround to prevent wrong values
		//being returned from sml layer before parsing a real message
		for (int smlLoop = 0; smlLoop < 9; smlLoop++) {
			eMeterValue = getValueFromSmlMessage(pay2, sizeof(pay2), &errorCode);
			smlGenerateGetProcParameterRequest(meterId, message, sizeof(message),
					obis, sizeof(obis));
		}
		eMeterValue = getValueFromSmlMessage(payload, payloadLength,
				&errorCode);
		if (errorCode == 0)
			returnValueSize = sprintf(returnValue, "%d\n", eMeterValue);
		else if (errorCode == -1)
			returnValueSize = sprintf(returnValue, "SML_Attention\n");
		else if (errorCode == -2)
			returnValueSize = sprintf(returnValue, "SML_Error");
		else if (errorCode == -3)
			returnValueSize = sprintf(returnValue,
					"no SML_MESSAGE_GET_PROC_PARAMETER_RESPONSE received\n");
		else
			returnValueSize = sprintf(returnValue, "SML parser failure\n");

#if TEST
		printf("\n\nmain.cpp: received data from tls layer:\n");
		printf("Meter ID [Hex]: ");
		for (int i = 0; i < 10; i++)
		printf("0x%02X ", fromMeterId[i]);
		printf("\nMeter ID: ");
		for (int i = 0; i < 10; i++)
		printf("%02X", fromMeterId[i]);
		printf("\nDestination Address: 0x%02X\n", destAddr);
		printf("Source Address: 0x%02X\n", sourceAddr);
		printf("Protocol Selector: 0x%02X\n", protSel);
		printf("Wirkarbeit A+: %0.4f kWh\n\n", eMeterValue);
		printf("\nReceived SML-Message: \n");
		for (int i = 0; i < (payloadLength); i++) {
			printf("0x%02X ", payload[i]);
			if ((i + 1) % 16 == 0)
			printf("\n");
		}
		printf("\n");
		printf("\n\nend of callback in main.cpp reached. ready to terminate\n");
#endif
	}
	delete[] fromMeterId;
	return;
}

void readConfig() {
	FILE * fileHandler = fopen("config.ini", "r");
	if (fileHandler != NULL) {
		int lineNumber = 0;
		while (lineNumber < MAX_LINES && !feof(fileHandler)) {
			fgets(line[lineNumber], MAX_SYMBOL, fileHandler);
			switch (lineNumber) {
			case 0:
				break;
			case 1:
				global_port = 0;
				if ((line[1][5] >= 0x30) && (line[1][5] <= 0x39))
					global_port += ((line[1][5]) - 0x30);
				if ((line[1][6] >= 0x30) && (line[1][6] <= 0x39)) {
					global_port *= 10;
					global_port += ((line[1][6]) - 0x30);
				}
				if ((line[1][7] >= 0x30) && (line[1][7] <= 0x39)) {
					global_port *= 10;
					global_port += ((line[1][7]) - 0x30);
				}
				if ((line[1][8] >= 0x30) && (line[1][8] <= 0x39)) {
					global_port *= 10;
					global_port += ((line[1][8]) - 0x30);
				}
				if ((line[1][9] >= 0x30) && (line[1][9] <= 0x39)) {
					global_port *= 10;
					global_port += ((line[1][9]) - 0x30);
				}
				if (global_port == 0)
					global_port = DEFAULT_PORT;
#if TEST
				printf("\n\n\n*********** global_port = %d ***************\n", global_port);
#endif
				break;
			case 2:
				global_tcp = (line[2][4]) - 0x30;
#if TEST
				printf("*********** global_tcp = %d ***************\n", global_tcp);
#endif
				break;
			case 3:
				global_sim = (line[3][4]) - 0x30;
#if TEST
				printf("*********** global_sim = %d ***************\n", global_sim);
#endif
				break;
			case 4:
				global_log = (line[4][4]) - 0x30;
#if TEST
				printf("*********** global_log = %d ***************\n", global_log);
#endif
				break;
			case 5:
				global_clear = (line[5][6]) - 0x30;
#if TEST
				printf("*********** global_clear = %d ***************\n", global_clear);
#endif
				break;
			case 6:
				global_debug = (line[6][6]) - 0x30;
#if TEST
				printf("*********** global_debug = %d ***************\n", global_debug);
#endif
				break;
			case 7:
				global_flush = (line[7][6]) - 0x30;
#if TEST
				printf("*********** global_flush = %d ***************\n", global_flush);
#endif
				break;
			case 8:
				global_RNR_us = (line[8][6]) - 0x30;
#if TEST
				printf("*********** global_RNR_us = %d ***************\n", global_RNR_us);
#endif
				break;
			case 9:
				global_details = (line[9][8]) - 0x30;
#if TEST
				printf("*********** global_details = %d ***************\n", global_details);
#endif
				break;
			case 10:
				global_moredetails = (line[10][12]) - 0x30;
#if TEST
				printf("*********** global_moredetails = %d ***************\n\n\n\n", global_moredetails);
#endif
				break;
			}
			lineNumber++;
		}
		fclose(fileHandler);
	}
	return;
}
