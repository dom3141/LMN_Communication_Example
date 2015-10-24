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

#include "server.h"
#include "hdlc.h"
#include "lmnsubscriber.h"
#include "lmnadmin.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

char getvalueString[] = "getvalue";
char connectString[] = "connect";
char disconnectString[] = "disconnect";
char smlCosemString[] = "SML_COSEM";
char tlsSmlCosemString[] = "TLS_SML_COSEM";
char quitString[] = "quit";
char loadserverkeyString[] = "loadserverkey";
char loadservercertString[] = "loadservercert";
char loadclientkeyString[] = "loadclientkey";
char loadclientcertString[] = "loadclientcert";
char cryptoInitString[] = "cryptoinit";
char shutdownString[] = "shutdown";
char broadcastString[] = "broadcast";
char clearString[] = "clear";

void certHandle(char * commands, unsigned char * meterId,
		unsigned char * masterKey, unsigned char * certOrKey,
		int * certOrKeyLength, int * bug) {

	unsigned char zeichen;

	//read MeterID
	for (int idx = 0; *(commands + idx) != '\0' && idx < (2 * 10); idx++) {
		zeichen = static_cast<unsigned char>(*(commands + idx));
		zeichen = zeichen < 0x40 ? zeichen - 0x30 : zeichen - 0x37;
		if (idx % 2 == 0)
			*(meterId + idx / 2) = (zeichen << 4);
		else
			*(meterId + idx / 2) |= (zeichen);
	}
	//read Master Key
	for (int idx = 0;
			*(commands + idx + MAX_COMMAND_SIZE) != '\0' && idx < (2 * 16);
			idx++) {
		zeichen =
				static_cast<unsigned char>(*(commands + idx + MAX_COMMAND_SIZE));
		zeichen = zeichen < 0x40 ? zeichen - 0x30 : zeichen - 0x37;
		if (idx % 2 == 0)
			masterKey[idx / 2] = (zeichen << 4);
		else
			masterKey[idx / 2] |= (zeichen);
	}
	//read Bug
	zeichen = static_cast<unsigned char>(*(commands + 2 * MAX_COMMAND_SIZE));
	zeichen = zeichen < 0x40 ? zeichen - 0x30 : zeichen - 0x37;
	*bug = static_cast<int>(zeichen);
	//read Client Certificate
	for (int idx = 0;
			*(commands + idx + 3 * MAX_COMMAND_SIZE) != '\0'
					&& idx < (2 * MAX_CERT_SIZE); idx++) {
		zeichen = static_cast<unsigned char>(*(commands + idx
				+ 3 * MAX_COMMAND_SIZE));
		zeichen = zeichen < 0x40 ? zeichen - 0x30 : zeichen - 0x37;
		if (idx % 2 == 0)
			*(certOrKey + idx / 2) = (zeichen << 4);
		else
			*(certOrKey + idx / 2) |= (zeichen);
		*certOrKeyLength = (idx + 1) / 2;
	}
	return;
}

command readCommand(char *command) {
	if (strcmp(command, getvalueString) == 0)
		return GETVALUE;
	if (strcmp(command, connectString) == 0)
		return CONNECT;
	if (strcmp(command, disconnectString) == 0)
		return DISCONNECT;
	if (strcmp(command, quitString) == 0)
		return QUIT;
	if (strcmp(command, shutdownString) == 0)
		return SHUTDOWN;
	if (strcmp(command, clearString) == 0)
		return CLEAR;
	if (strcmp(command, smlCosemString) == 0)
		return SMLCOSEM;
	if (strcmp(command, tlsSmlCosemString) == 0)
		return TLSSMLCOSEM;
	if (strcmp(command, loadserverkeyString) == 0)
		return LOADSERVERKEY;
	if (strcmp(command, loadservercertString) == 0)
		return LOADSERVERCERT;
	if (strcmp(command, loadclientkeyString) == 0)
		return LOADCLIENTKEY;
	if (strcmp(command, loadclientcertString) == 0)
		return LOADCLIENTCERT;
	if (strcmp(command, cryptoInitString) == 0)
		return CRYPTOINIT;
	if (strcmp(command, broadcastString) == 0)
		return BROADCAST;
	if (command[0] == 0)
		return NOTHING;
	return UNKNOWNCOMMAND;
}

void error(const char *msg) {
	perror(msg);
	exit(1);
}
