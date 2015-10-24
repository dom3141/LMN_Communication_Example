/* Copyright 2015 Kai Heine
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

#ifndef AES_CMAC_H_
#define AES_CMAC_H_

#include <wolfssl/wolfcrypt/aes.h>
#include <cmath>

#ifdef __cplusplus
extern "C" {
#endif

const unsigned char BLOCKSIZE = 16;

const unsigned char const_Zero[BLOCKSIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0 };

const unsigned char const_Rb[BLOCKSIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0x87 };

#define MSB(x)	(x & 0x80)

void aes_cmac(const unsigned char *key, const unsigned char *message, int len,
		unsigned char *mac);
void Generate_Subkey(Aes *aes, unsigned char *k1, unsigned char *k2);
void leftshift_onebit(const unsigned char *input, unsigned char *output);
void xor_128(const unsigned char *a, const unsigned char *b,
		unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif /* AES_CMAC_H_ */
