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

#include "test.h"
#include "tls.h"
#include "lmnadmin.h"
#include <sys/time.h>
#include <unistd.h>

WOLFSSL_CTX* ctxArray[128];
WOLFSSL* sslArray[128];
CallbackIORecv CBIORecv = CBIOUserRecv;
CallbackIOSend CBIOSend = CBIOUserSend;

unsigned char globalMeterId[10] = { 0 };

void setGlobalMeterID(unsigned char* meterId) {
	for (int i = 0; i < 10; i++)
		globalMeterId[i] = meterId[i];
	return;
}

int counter = 0, idx = 0, overall = 0, newData = 0, pufferLength = 0, idx2 = 0;

unsigned char puffer[2048];
unsigned char certBuffer[2048];
unsigned char keyBuffer[2048];
unsigned char empfPuffer[2048];

void cb(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength) {

	unsigned char* fromMeterId = new unsigned char[10];
	getMeterId(sourceAddr, fromMeterId);

	if (payloadLength != 0) {
		newData = 1;
		pufferLength = payloadLength;
		for (int i = 0; i < payloadLength; i++)
			puffer[i] = payload[i];
	}
	delete[] fromMeterId;
	return;
}

int myEccSign(WOLFSSL* ssl, const byte* in, word32 inSz, byte* out,
		word32* outSz, const byte* key, word32 keySz, void* ctx) {
	return 0;
}

int myEccVerify(WOLFSSL* ssl, const byte* sig, word32 sigSz, const byte* hash,
		word32 hashSz, const byte* key, word32 keySz, int* result, void* ctx) {
	*result = 1;
	return 0;
}

int handShakeCB(HandShakeInfo* info) {
	(void) info;
	return 0;
}

int timeoutCB(TimeoutInfo* info) {
	(void) info;
	return 0;
}

void InitHandShakeInfo(HandShakeInfo* info) {
	int i;

	info->cipherName[0] = 0;
	for (i = 0; i < MAX_PACKETS_HANDSHAKE; i++)
		info->packetNames[i][0] = 0;
	info->numberPackets = 0;
	info->negotiationError = 0;
	printf("\n\n*****InitHandShakeInfo*****\n\n");
	printf("\n--- PRESS ENTER TO CONTINUE ---");
}

int CBIOUserRecv(WOLFSSL *ssl, char *buf, int sz, void *ctx) {
	if (newData == 1) {
		*buf = (char) puffer[idx2];
		idx2++;
		if (idx2 == pufferLength) {
			idx2 = 0;
			newData = 0;
		}
		return 1;
	} else
		lmnSendData(globalMeterId, TLS_SML_COSEM, puffer, 0, cb);
	return 0;
}

int CBIOUserSend(WOLFSSL* ssl, char* buf, int sz, void* ctx) {
#if PRINT_IO
	printf("\n\nwolfssl->CBIOUserSend: CBIOUserSend wurde aufgerufen\n\n");
#endif
#if PRINT_IO
	printf("\n\nwolfssl->CBIOUserSend wurde aufgerufen mit folgender globalMeterId:\n");
	for (int idx = 0; idx < 10; idx++)
	printf("%02X", globalMeterId[idx]);
	printf("\n\n\n");
#endif
	counter++;
	unsigned char message[2048];
	int messageLength = sz;

	for (int i = 0; i < sz; i++)
		message[i] = static_cast<unsigned char>(buf[i]);

#if PRINT_IO
	printf("\n\nwolfssl->CBIOUserSend: lmnSendData wird aufgerufen mit folgender globalMeterId:\n");
	for (int idx = 0; idx < 10; idx++)
	printf("%02X", globalMeterId[idx]);
	printf("\n\n\n");
#endif
	lmnSendData(globalMeterId, TLS_SML_COSEM, message, messageLength, cb);
#if PRINT_IO
	printf("\n\nwolfssl->CBIOUserSend: lmnSendData wurde aufgerufen mit folgender globalMeterId:\n");
	for (int idx = 0; idx < 10; idx++)
	printf("%02X", globalMeterId[idx]);
	printf("\n\n\n");
#endif

	return messageLength;
}

void tlsInit(void) {
	int ret = 0;
	ret = wolfSSL_Init();
	return;
}

void tlsInitCtxAndSsl(unsigned char* meterId) {

	FILE *fileHandler;
	char fileName[100];
	int fileLength, certLength, keyLength;

	unsigned char hdlcAddress = getHdlcAddress(meterId);

	if (getTlnTlsState(static_cast<unsigned short>(hdlcAddress))
			== initialized) {
#if PRINT_IO
		printf("\n\nwolfssl: TLS ist bereits initialisiert\n\n");
#endif
		return;
	}
	int ret = 0;

	CallbackIORecv CBIORecv = CBIOUserRecv;
	CallbackIOSend CBIOSend = CBIOUserSend;

	ret = wolfSSL_Init();
#if WOLFSSL_PRINT_INIT
	printf("wolfSSL_Init() = %d", ret);
#endif

	ctxArray[static_cast<int>(hdlcAddress)] = wolfSSL_CTX_new(
			wolfTLSv1_2_client_method()); //TLS 1.2
	wolfSSL_CTX_set_verify(ctxArray[static_cast<int>(hdlcAddress)],
			SSL_VERIFY_NONE, 0);

	ret = wolfSSL_CTX_UseMaxFragment(ctxArray[static_cast<int>(hdlcAddress)],
			WOLFSSL_MFL_2_9);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_CTX_UseMaxFragment() = %d", ret);
#endif

	ret =
			wolfSSL_CTX_set_cipher_list(ctxArray[static_cast<int>(hdlcAddress)],
					"TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256:TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384");
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_CTX_set_cipher_list() = %d", ret);
#endif

	ret = wolfSSL_CTX_UseSupportedCurve(ctxArray[static_cast<int>(hdlcAddress)],
			WOLFSSL_ECC_SECP256R1);

	bzero((unsigned char *) &fileName, sizeof(fileName));
	strcat(fileName, "./certkey/");

	for (int i = 0; i < 10; i++)
		sprintf(&fileName[i * 2 + 10], "%02X", meterId[i]);

	strcat(fileName, "/crypto_client.cer");
	fileHandler = fopen(fileName, "rb");
	if (fileHandler != NULL) {
		fseek(fileHandler, 0, SEEK_END);
		fileLength = ftell(fileHandler);
		rewind(fileHandler);
		certLength = fread(certBuffer, sizeof(unsigned char), fileLength,
				fileHandler);
		fclose(fileHandler);
	} else {
		printf("\nFehler in TLS.CPP\n");
	}
#if WOLFSSL_PRINT_INIT
	printf("\n\nClient Certificate:\n");
	for (int i = 0; i < certLength; i++) {
		printf("0x%02X ", certBuffer[i]);
		if (i != 0 && (i+1) % 16 == 0)
		printf("\n");
	}
	printf("\n\n");
#endif
	bzero((unsigned char *) &fileName, sizeof(fileName));
	strcat(fileName, "./certkey/");
	for (int i = 0; i < 10; i++)
		sprintf(&fileName[i * 2 + 10], "%02X", meterId[i]);
	strcat(fileName, "/crypto_client_der.key");

	fileHandler = fopen(fileName, "rb");
	if (fileHandler != NULL) {
		fseek(fileHandler, 0, SEEK_END);
		fileLength = ftell(fileHandler);
		rewind(fileHandler);
		keyLength = fread(keyBuffer, sizeof(unsigned char), fileLength,
				fileHandler);
		fclose(fileHandler);
	} else {
		printf("\nFehler 2 in TLS.CPP\n");
	}
#if WOLFSSL_PRINT_INIT
	printf("\nClient Key:\n");
	for (int i = 0; i < keyLength; i++) {
		printf("0x%02X ", keyBuffer[i]);
		if (i != 0 && (i+1) % 16 == 0)
		printf("\n");
	}
	printf("\n\n");
#endif

	ret = wolfSSL_CTX_use_certificate_buffer(
			ctxArray[static_cast<int>(hdlcAddress)], certBuffer,
			static_cast<long int>(certLength), SSL_FILETYPE_RAW);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_CTX_use_certificate_buffer() = %d", ret);
#endif
	ret = wolfSSL_CTX_use_PrivateKey_buffer(
			ctxArray[static_cast<int>(hdlcAddress)], keyBuffer,
			static_cast<long int>(keyLength), SSL_FILETYPE_RAW);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_CTX_use_PrivateKey_buffer() = %d", ret);
#endif
	ret = wolfSSL_CTX_check_private_key(
			ctxArray[static_cast<int>(hdlcAddress)]);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_CTX_check_private_key() = %d", ret);
#endif
	sslArray[static_cast<int>(hdlcAddress)] = wolfSSL_new(
			ctxArray[static_cast<int>(hdlcAddress)]);

	ret = wolfSSL_use_certificate_buffer(
			sslArray[static_cast<int>(hdlcAddress)], certBuffer,
			static_cast<long int>(certLength), SSL_FILETYPE_RAW);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_use_certificate_buffer() = %d", ret);
#endif
	ret = wolfSSL_use_PrivateKey_buffer(sslArray[static_cast<int>(hdlcAddress)],
			keyBuffer, static_cast<long int>(keyLength), SSL_FILETYPE_RAW);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_use_PrivateKey_buffer() = %d", ret);
#endif
	ret = wolfSSL_CTX_UseSupportedCurve(ctxArray[static_cast<int>(hdlcAddress)],
			WOLFSSL_ECC_SECP256R1);
	ret = wolfSSL_UseSupportedCurve(sslArray[static_cast<int>(hdlcAddress)],
			WOLFSSL_ECC_SECP256R1);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_UseSupportedCurve() = %d", ret);
#endif
	ret = wolfSSL_UseMaxFragment(sslArray[static_cast<int>(hdlcAddress)],
			WOLFSSL_MFL_2_9);
#if WOLFSSL_PRINT_INIT
	printf("\nwolfSSL_UseMaxFragment() = %d", ret);
#endif

	wolfSSL_set_cipher_list(sslArray[static_cast<int>(hdlcAddress)],
			"TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256:TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384");

	wolfSSL_SetIORecv(ctxArray[static_cast<int>(hdlcAddress)], CBIORecv);
	wolfSSL_SetIOSend(ctxArray[static_cast<int>(hdlcAddress)], CBIOSend);

	wolfSSL_CTX_set_verify(ctxArray[static_cast<int>(hdlcAddress)],
			SSL_VERIFY_NONE, 0);

#if PRINT_CIPHER_SUITES
	printf("\n");
	char myArray[2000] = {NULL};
	wolfSSL_get_ciphers(myArray, 2000);
	for (int i = 0; i < 2000 && myArray[i] != NULL; i++) {
		if (myArray[i] == ':')
		printf("\n");
		else
		printf("%c", myArray[i]);
	}
#endif

#if DEGUG_WOLFSSL
	wolfSSL_Debugging_ON();
#endif

	setTlnTlsState(hdlcAddress, initialized);
#if PRINT_IO
	printf("\n\nwolfssl: TLS wurde initialisiert\n\n");
#endif
	return;
}

void tlsSendData(unsigned char* meterId, protocolSelector protSel,
		unsigned char *payload, int payloadLength, func_ptr callback) {

#if PRINT_IO
	printf("\n\n\nDiese MeterID wurde tlsSendData übermittelt:\n");
	for (int idx = 0; idx < 10; idx++)
	printf("%02X", meterId[idx]);
	printf("\n\n\n");
#endif

	int tlsReceived;
	unsigned char hdlcAddress = getHdlcAddress(meterId);
	if (hdlcAddress == 0x00)
		return;
	setGlobalMeterID(meterId);

#if PRINT_IO
	printf("\n\n\nDies ist die globalMeterId:\n");
	for (int idx = 0; idx < 10; idx++)
	printf("%02X", globalMeterId[idx]);
	printf("\n\n\n");
#endif

	tlsInitCtxAndSsl(meterId);

	wolfSSL_send(sslArray[static_cast<int>(hdlcAddress)], (void*) payload,
			payloadLength, 1);
	tlsReceived = wolfSSL_recv(sslArray[static_cast<int>(hdlcAddress)],
			(void*) payload, 2048, 1);
#if WOLFSSL_PRINT_INIT
	printf("\nHier in tlsSend(): Das wurde empfangen: \n");
	for (int i = 0; i < tlsReceived; i++) {
		printf("0x%02X ", payload[i]);
		if ((i+1) % 16 == 0)
		printf("\n");
	}
#endif
	int sourceAddr = getHdlcAddress(meterId);
	callback(0x01, sourceAddr, protSel, payload, tlsReceived);

	return;
}

void tlsTerminate(void) {

	for (unsigned short i = 0; i < 128; i++) {
		if (getTlnTlsState(i) == initialized) {
			wolfSSL_free(sslArray[i]);		// Free SSL object
			wolfSSL_CTX_free(ctxArray[i]);	// Free SSL_CTX object
			setTlnTlsState(i, uninitialized);
		}
	}
	wolfSSL_Cleanup();		// Free CyaSSL

	return;

}
