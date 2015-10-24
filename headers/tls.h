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

#ifndef TLS_H_
#define TLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WOLFSSL_CALLBACKS
#define HAVE_MAX_FRAGMENT
#define HAVE_SUPPORTED_CURVES
#define WOLFSSL_USER_IO
#define DEBUG_WOLFSSL
#define WOLFSSL_DER_LOAD
#undef NO_CERTS

#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/ssl.h>
#include <wolfssl/callbacks.h>

void cb(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);
int myEccSign(WOLFSSL* ssl, const byte* in, word32 inSz, byte* out,
		word32* outSz, const byte* key, word32 keySz, void* ctx);
int myEccVerify(WOLFSSL* ssl, const byte* sig, word32 sigSz, const byte* hash,
		word32 hashSz, const byte* key, word32 keySz, int* result, void* ctx);
int handShakeCB(HandShakeInfo* info);
int timeoutCB(TimeoutInfo* info);
void InitHandShakeInfo(HandShakeInfo* info);
int CBIOUserRecv(WOLFSSL *ssl, char *buf, int sz, void *ctx);
int CBIOUserSend(WOLFSSL *ssl, char *buf, int sz, void *ctx);
void tlsInit(void);
void tlsInitCtxAndSsl(unsigned char* meterId);
void tlsTerminate(void);
void tlsSendData(unsigned char* meterId, protocolSelector protSel,
		unsigned char *payload, int payloadLength, func_ptr callback);
void setGlobalMeterID(unsigned char* meterId);

#ifdef __cplusplus
}
#endif

#endif /* TLS_H_ */
