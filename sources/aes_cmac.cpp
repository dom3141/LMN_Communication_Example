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

#include "aes_cmac.h"

void aes_encrypt(Aes *aes, unsigned char *cipher,
		const unsigned char *message) {
	wc_AesSetIV(aes, 0);
	wc_AesCbcEncrypt(aes, cipher, message, BLOCKSIZE);
}

void leftshift_onebit(const unsigned char *input, unsigned char *output) {
	unsigned char overflow = 0;

	for (int i = BLOCKSIZE - 1; i >= 0; i--) {
		output[i] = input[i] << 1;
		output[i] |= overflow;
		overflow = (input[i] & 0x80) ? 1 : 0;
	}
}

void xor_128(const unsigned char *a, const unsigned char *b,
		unsigned char *out) {
	for (int i = 0; i < BLOCKSIZE; i++) {
		out[i] = a[i] ^ b[i];
	}
}

void Generate_Subkey(Aes *aes, unsigned char *k1, unsigned char *k2) {
	unsigned char l[BLOCKSIZE];
	aes_encrypt(aes, l, const_Zero);
	leftshift_onebit(l, k1);
	if (MSB(l[0]) != 0) {
		xor_128(k1, const_Rb, k1);
	}

	leftshift_onebit(k1, k2);
	if (MSB(k1[0]) != 0) {
		xor_128(k2, const_Rb, k2);
	}
}

void padding(const unsigned char *lastblock, unsigned char *padded,
		int length) {
	for (int j = 0; j < BLOCKSIZE; j++) {
		if (j < length) {
			padded[j] = lastblock[j];
		} else if (j == length) {
			padded[j] = 0x80;
		} else {
			padded[j] = 0x00;
		}
	}
}

void aes_cmac(const unsigned char *key, const unsigned char *message, int len,
		unsigned char *mac) {
	Aes aes;
	wc_AesSetKey(&aes, key, BLOCKSIZE, 0, AES_ENCRYPTION);

	unsigned char k1[BLOCKSIZE], k2[BLOCKSIZE], xorResult[BLOCKSIZE],
			lastBlock[BLOCKSIZE];

	Generate_Subkey(&aes, k1, k2);

	int nBlocks = (len + BLOCKSIZE - 1) / BLOCKSIZE;
	bool needs_padding;

	if (nBlocks == 0) {
		nBlocks = 1;
		needs_padding = true;
	} else {
		needs_padding = (len % BLOCKSIZE) != 0;
	}

	if (needs_padding) {
		padding(&message[BLOCKSIZE * (nBlocks - 1)], lastBlock,
				len % BLOCKSIZE);
		xor_128(lastBlock, k2, lastBlock);
	} else {
		xor_128(&message[BLOCKSIZE * (nBlocks - 1)], k1, lastBlock);
	}

	for (int i = 0; i < BLOCKSIZE; i++)
		mac[i] = 0x00;

	for (int i = 0; i < nBlocks - 1; i++) {
		xor_128(&message[i * BLOCKSIZE], mac, xorResult);
		aes_encrypt(&aes, mac, xorResult);
	}

	xor_128(lastBlock, mac, xorResult);
	aes_encrypt(&aes, mac, xorResult);
}
