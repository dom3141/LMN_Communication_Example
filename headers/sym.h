/* Copyright 2015 Kai Heine
 * Labor Datentechnik, Ostfalia Hochschule Braunschweig/Wolfenbüttel
 *
 * based on code by Matthias Köhne copyright 2015 LMN-Kommunikator (SMGw_SIM)
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

#ifndef SYM_H_
#define SYM_H_

#include "aes_cmac.h"

#ifdef __cplusplus
extern "C" {
#endif

enum keytype_e {
	L, K
};

enum {
	ENC = 0, MAC = 1
};

enum cert_key {
	CLIENT_KEY, CLIENT_CERT, SERVER_KEY, SERVER_CERT
};

struct keypair_s {
	unsigned char enc[BLOCKSIZE], mac[BLOCKSIZE];
};

struct meterparams {
	unsigned int transmissionCounter;
	unsigned char *mk, *meterID, dataBuf[1024], encryptedBuf[1024], command;
	char *cipherSuite;
	bool bug;
};

bool symHandshake(unsigned char *meterId, unsigned char *masterKey,
		char *cipherSuite, bool meterBug);
void symSendData();
bool storeCertsAndKeys(unsigned char *meterID, unsigned char *data,
		unsigned int len, char *cipherSuite, cert_key certkey);
bool certsExist(unsigned char *meterID, char *cipherSuite);

const unsigned char METERID_LEN = 10;
const unsigned char TCNT_LEN = sizeof(unsigned int);
const unsigned char MESSAGE_LEN_LEN = sizeof(unsigned int);
const unsigned char KEYCODE_LEN = sizeof(unsigned char);
const unsigned char UNPADDED_LEN = KEYCODE_LEN + TCNT_LEN + METERID_LEN;
const unsigned char PADDING_LEN = 16 - (UNPADDED_LEN % 16);
const unsigned char BUG_PADDING = 0x00;

const unsigned int STARTCOUNT = 0xFFFFFFFF;
const unsigned int MACLEN = BLOCKSIZE;

const char CERTDIR[] = "certkey/";

#ifdef __cplusplus
}
#endif

#endif /* SYM_H_ */
