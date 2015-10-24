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

// 0A01454D4800004AF1FD
// 0100010800FF800200
// getValue(0A01454D4800004AF1FD, 0100010800FF800200)
// getValue(0A01454D4800004AFFFD, 0100010800FF800200) falsche ID
// getValue(0A01454D4800004AF1FD, 0100010600FF800200) falsche Obis
// getValue(0A01454D4800004AF1FD, 0100320700FF800200) andere Obis
// getValue(0A01454D4800004AF1FD, 0100010800FF800200) wirkarbeit
// getValue(0A01454D4800004AF1FD, 01005E310007000100) kryptoreset
// getValue(0A01454D4800004AF1FD, 0100010700FF000300) momentan_Wirkleistung
// getValue(0A01454D4800004AF1FD, 01005E310107000100) pinvergeben
// getValue(0A01454D4800004AF1FD, 0100020700FF000300) SpannungsmesswertL1
// getValue(0A01454D4800004AF1FD, 0100034700FF000300) SpannungsmesswertL2
// getValue(0A01454D4800004AF1FD, 0100048700FF000300) SpannungsmesswertL3
// loadServerCert(0A01454D4800004AF1FD, mk, opt, 3082019b3082013fa00302010202...)
// loadClientCert(0A01454D4800004AF1FD, mk, opt, 3082019b3082013fa00302010202...)
// loadServerKey(0A01454D4800004AF1FD, mk, opt, 3082019b3082013fa00302010202...)
// loadClientKey(0A01454D4800004AF1FD, mk, opt, 3082019b3082013fa00302010202...)
// cryptoInit(0A01454D4800004AF1FD, 3082019b3082013fa00302010202...)
// Antwort eines Zählers auf Broadcast
// 7EA02B020374031325193A190A01454D4800004AF1FD000000000A01454D4800004AF1FD000000000000B3887E
#ifndef SERVER_H_
#define SERVER_H_

#define TCP 0

#include "hdlc.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

enum command {
	GETVALUE,
	CONNECT,
	DISCONNECT,
	RESET,
	CLEAR,
	BROADCAST,
	LOADSERVERCERT,
	LOADSERVERKEY,
	LOADCLIENTCERT,
	LOADCLIENTKEY,
	LOADSERVERROOTCERT,
	LOADCLIENTROOTCERT,
	LOADSERVERROOTCRL,
	LOADCLIENTROOTCRL,
	CRYPTORESET,
	CRYPTOINIT,
	GET,
	SET,
	PUT,
	QUIT,
	SHUTDOWN,
	NOTHING,
	UNKNOWNCOMMAND,
	SMLCOSEM,
	TLSSMLCOSEM
};

#define MAX_COMMAND_SIZE 5000
#define MAX_CERT_SIZE 2048
#define MAX_BUFFER_SIZE 5000

command readCommand(char *command);
void error(const char *msg);
void certHandle(char * commands, unsigned char * meterID,
		unsigned char * masterKey, unsigned char * certOrKey,
		int * certOrKeyLength, int * bug);

void readMeterValue_cb(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_H_ */
