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

#include "sym.h"
#include "aes_cmac.h"
#include "hdlc.h"
#include "test.h"
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

struct meterparams *meter = NULL;

void getCertPath(unsigned char *meterID, char *path) {
	if (meterID == NULL || path == NULL)
		return;

	strcpy(path, CERTDIR);
	for (int i = 0; i < METERID_LEN; i++) {
		sprintf(&path[i * 2 + sizeof(CERTDIR) - 1], "%02X", meterID[i]);
	}

	strcat(path, "/");
}

void appendFileName(char *cipherSuite, cert_key certkey, char *path) {
	strcat(path, cipherSuite);
	strcat(path, "_");

	if (certkey == CLIENT_KEY || certkey == CLIENT_CERT) {
		strcat(path, "client");
	} else {
		strcat(path, "server");
	}

	if (certkey == CLIENT_KEY || certkey == SERVER_KEY) {
		strcat(path, "_der.key");
	} else {
		strcat(path, ".cer");
	}
}

bool certsExist(unsigned char *meterID, char *cipherSuite) {
	if (meterID == NULL || cipherSuite == NULL)
		return false;

	if (cipherSuite[0] == 0) {
#if TEST
		printf("\ncipherSuite darf nicht leer sein\n");
#endif
		return false;
	}

	int len = strlen(cipherSuite);

	char path[128] = "";
	getCertPath(meterID, path);

	struct dirent *dp;
	DIR *fd;
	int i = 0;

	if ((fd = opendir(path)) == NULL) {
		return false;
	}

	while ((dp = readdir(fd)) != NULL) {
		if (strncmp(dp->d_name, cipherSuite, len) == 0)
			i++;
	}
	closedir(fd);

	return i == 4;
}

bool storeCertsAndKeys(unsigned char *meterID, unsigned char *data,
		unsigned int len, char *cipherSuite, cert_key certkey) {
	if (meterID == NULL || data == NULL || cipherSuite == NULL) {
		return false;
	}

	if (cipherSuite[0] == 0) {
#if TEST
		printf("\ncipherSuite darf nicht leer sein\n");
#endif
		return false;
	}

	DIR *fd;

	char path[128] = "";
	getCertPath(meterID, path);

	if ((fd = opendir(CERTDIR)) == NULL) {
		if (mkdir(CERTDIR, 0777) == -1) {
			return false;
		}
	}
	closedir(fd);

	if ((fd = opendir(path)) == NULL) {
		if (mkdir(path, 0777) == -1) {
			return false;
		}
	}
	closedir(fd);

	appendFileName(cipherSuite, certkey, path);

	FILE *f = fopen(path, "wb");
	if (f != NULL) {
		fwrite(data, sizeof(unsigned char), len, f);
		fclose(f);
	} else
		return false;

#if TEST
	if (certsExist(meterID, cipherSuite)) {
		printf("Alle Certs und Keys vorhanden\n");
	} else {
		printf("Es fehlen noch Certs oder Keys\n");
	}
#endif

	return true;
}

void generateKeyPair(keytype_e keytype, keypair_s * keypair) {
	if (meter == NULL || keypair == NULL)
		return;

	unsigned char keycode[2];
	unsigned int i;

//	if (keytype == L) {
	if (meter->bug) {
		if (keytype == L) {
			keycode[ENC] = 0x00;
			keycode[MAC] = 0x01;
		} else {
			keycode[ENC] = 0x10;
			keycode[MAC] = 0x11;
		}
	} else {
		if (keytype == K) {
			keycode[ENC] = 0x00;
			keycode[MAC] = 0x01;
		} else {
			keycode[ENC] = 0x10;
			keycode[MAC] = 0x11;
		}
	}

	// Zusammenbauen der Daten für die Schlüsselgenerierung
	unsigned char paddedblock[BLOCKSIZE];

	paddedblock[0] = keycode[ENC];

	unsigned char *tcnt_ptr = (unsigned char *) &meter->transmissionCounter;
	for (i = 1; i <= sizeof(meter->transmissionCounter); i++) {
		paddedblock[i] = tcnt_ptr[sizeof(meter->transmissionCounter) - i];
	}

	for (i = 0; i < METERID_LEN; i++) {
		paddedblock[KEYCODE_LEN + sizeof(meter->transmissionCounter) + i] =
				meter->meterID[i];
	}

	paddedblock[UNPADDED_LEN] = meter->bug ? 0x00 : PADDING_LEN;

	//Schlüsselgenerierung
	aes_cmac(meter->mk, paddedblock, BLOCKSIZE, keypair->enc);

	paddedblock[0] = keycode[MAC];
	aes_cmac(meter->mk, paddedblock, BLOCKSIZE, keypair->mac);
}

void symReceiveData(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength) {
	if (meter == NULL || payload == NULL)
		return;

	unsigned char command, mac[MACLEN], *decryptedData, datalen,
			unpaddeddatalen;
	keypair_s k;

	command = payload[0];

	meter->transmissionCounter = (payload[5] << 24) + (payload[6] << 16)
			+ (payload[7] << 8) + payload[8];

	generateKeyPair(K, &k);

	datalen = payloadLength - MACLEN - 9;
	unpaddeddatalen = (payload[1] << 24) + (payload[2] << 16)
			+ (payload[3] << 8) + payload[4] - MACLEN - 9;
	decryptedData = new unsigned char[datalen];
	Aes aes;
	wc_AesSetKey(&aes, k.enc, BLOCKSIZE, 0, AES_DECRYPTION);
	wc_AesCbcDecrypt(&aes, decryptedData, &payload[9], datalen);

#if TEST
	printf("\n symCallBack reached... \n");

	printf("\nReceived SYM-Message: \n");
	for (int i = 0; i < (payloadLength); i++) {
		printf("0x%02X ", payload[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
	printf("\n");

	printf("Kommando: 0x%2X\n", command);

	printf("Nachrichtenlänge: %d, payload length: %d\n",
			(payload[1] << 24) + (payload[2] << 16) + (payload[3] << 8)
			+ payload[4], payloadLength);

	printf("Transmission Counter: %d\n", meter->transmissionCounter);

	printf("Empfangene Daten:\n");
	for (unsigned int i = 0; i < datalen; i++) {
		printf("0x%02X ", payload[i + 9]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
	printf("\nEntschlüsselte Daten:\n");
	for (unsigned int i = 0; i < unpaddeddatalen; i++) {
		printf("0x%02X ", decryptedData[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
	}
	printf("\n");

	printf("CMAC:\n");
	for (unsigned int i = 0; i < MACLEN; i++) {
		printf("0x%02X ", payload[i + payloadLength - MACLEN]);
	}
	printf("\n");

#endif

	// mac überprüfung
	aes_cmac(k.mac, payload, payloadLength - MACLEN, mac);

#if TEST
	printf("Berechnete CMAC:\n");
	for (unsigned int i = 0; i < MACLEN; i++) {
		printf("0x%02X ", mac[i]);
	}
	printf("\n");
#endif

	for (unsigned int i = 0; i < MACLEN; i++) {
		if (mac[i] != payload[i + payloadLength - MACLEN]) {
#if TEST
			printf("MAC IST NICHT KORREKT\n");
#endif
			meter->command = 0; // damit es nicht 0x05 sein kann und als Fehler erkannt wird
			return;
		}
#if TEST
		if (i == MACLEN - 1)
		printf("MAC IST KORREKT\n");
#endif
	}

	if (command < 0x85) {
		meter->command = ++command & 0x0F;
		symSendData();
	}

	delete[] decryptedData;
}

void symSendData() {
	if (meter == NULL)
		return;

	unsigned int message_len, data_len, encryptedData_len, offset, i;
	unsigned char *tmpPtr, mac[MACLEN];
	unsigned char message[2048];
	char path[128] = "";
	keypair_s l;
	FILE *cert, *key;

	switch (meter->command) {
	case 0x01: // transmission counter
		meter->transmissionCounter = STARTCOUNT;
		encryptedData_len = 0;
		break;

	case 0x02: // Liste Algorithmen
		meter->transmissionCounter++;
		encryptedData_len = 0;
		break;

	case 0x03: // Private Key und Zertifikat des Zählers
		meter->transmissionCounter++;

		getCertPath(meter->meterID, path);
		appendFileName(meter->cipherSuite, SERVER_CERT, path);
		cert = fopen(path, "rb");

		getCertPath(meter->meterID, path);
		appendFileName(meter->cipherSuite, SERVER_KEY, path);
		key = fopen(path, "rb");
		break;

	case 0x04: // Zertifikat des SMGw
		meter->transmissionCounter++;

		getCertPath(meter->meterID, path);
		appendFileName(meter->cipherSuite, CLIENT_CERT, path);
		cert = fopen(path, "rb");
		break;

	case 0x05: // max. Fragmentgröße
		meter->transmissionCounter++;
		encryptedData_len = 0;
		break;

	default:
		break;
	}

	if (meter->command == 0x03 || meter->command == 0x04) {
		data_len = 0;

		if (cert != NULL) {
			while (fread(&meter->dataBuf[data_len], 1, 1, cert) == 1)
				data_len++;
			fclose(cert);
		} else {
#if TEST
			printf("datei konnte nicht geöffnet werden\n");
#endif
			return;
		}

		if (meter->command == 0x03) {
			if (key != NULL) {
				while (fread(&meter->dataBuf[data_len], 1, 1, key) == 1)
					data_len++;
				fclose(key);
			} else {
#if TEST
				printf("datei konnte nicht geöffnet werden\n");
#endif
				return;
			}
		}

		if (data_len % BLOCKSIZE != 0) {
			encryptedData_len = data_len + (BLOCKSIZE - (data_len % BLOCKSIZE));
		} else {
			encryptedData_len = data_len;
		}

		unsigned char padlen = BLOCKSIZE - (data_len % BLOCKSIZE);
		// EMH Zählerbug
		memset(&meter->dataBuf[data_len], meter->bug ? 0x00 : padlen, padlen);
	}

	generateKeyPair(L, &l);

	if (encryptedData_len != 0) {
		Aes aes;
		wc_AesSetKey(&aes, l.enc, BLOCKSIZE, 0, AES_ENCRYPTION);
		wc_AesCbcEncrypt(&aes, meter->encryptedBuf, meter->dataBuf,
				encryptedData_len);
	}

	// Nachricht zusammenbauen
	message_len = sizeof(meter->command) + sizeof(message_len)
			+ sizeof(meter->transmissionCounter) + encryptedData_len + MACLEN;

	message[0] = meter->command;

	tmpPtr = (unsigned char *) &message_len;
	offset = sizeof(meter->command);
	for (i = 0; i < sizeof(message_len); i++) {
		message[i + offset] = tmpPtr[sizeof(message_len) - i - 1];
	}

	tmpPtr = (unsigned char *) &meter->transmissionCounter;
	offset += sizeof(message_len);
	for (i = 0; i < sizeof(meter->transmissionCounter); i++) {
		message[i + offset] =
				tmpPtr[sizeof(meter->transmissionCounter) - i - 1];
	}

	offset += sizeof(meter->transmissionCounter);
	for (i = 0; i < encryptedData_len; i++) {
		message[i + offset] = meter->encryptedBuf[i];
	}

	aes_cmac(l.mac, message, message_len - MACLEN, mac);
	offset += encryptedData_len;
	for (i = 0; i < MACLEN; i++) {
		message[i + offset] = mac[i];
	}

	int destAddr = getHdlcAddress(meter->meterID);

	if (destAddr != 0x00) {
		hdlcSendData(destAddr, 0x01, SYM_HDLC, message, message_len,
				symReceiveData);
	}
#if TEST
	else
	printf("zähler wurde nicht gefunden\n");
#endif
}

bool symHandshake(unsigned char *meterId, unsigned char *masterKey,
		char *cipherSuite, bool meterBug) {
	if (meterId == NULL || masterKey == NULL) {
		return false;
	}

	if (!certsExist(meterId, cipherSuite)) {
		return false;
	}

	meter = new struct meterparams;
	meter->meterID = meterId;
	meter->mk = masterKey;
	meter->bug = meterBug;
	meter->command = 0x01;
	meter->cipherSuite = cipherSuite;

	symSendData();

	bool ret = meter->command == 0x05;

	delete meter;
	meter = NULL;

	return ret;
}
