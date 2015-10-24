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

#ifndef TEST_H_
#define TEST_H_

#include "hdlc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEST 0
#define PRINT_IO 0
#define DETAILED_TEST 0
#define CRC_FALSCH 0
#define WINDOWS 0
#define SNIFFER 1
#define DEGUG_WOLFSSL 0
#define PRINT_CIPHER_SUITES 0
#define WOLFSSLTEST	0
#define FAKE_TEST 0
#define WOLFSSL_PRINT_INIT 0
#define NO_FLUSH 0

void testFunktion(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength);

/*
 #define SML_TESTEN
 //#define BROADCAST_TESTEN
 #ifdef SML_TESTEN
 #ifdef BROADCAST_TESTEN
 #undef BROADCAST_TESTEN
 #pragma message ("WARNING: Testfall BROADCAST_TESTEN wurde entfernt")
 #endif
 #endif

 #ifndef SML_TESTEN
 #ifndef BROADCAST_TESTEN
 #error ALLE TESTFÄLLE SIND DEAKTIVERT!!!! BITTE ZUERST AKTIVEREN!!!!
 #endif
 #endif
 */

#ifdef __cplusplus
}
#endif

#endif /* TEST_H_ */
