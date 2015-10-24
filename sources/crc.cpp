/* Copyright 2015 Dominik Lüthe
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

#include "crc.h"
#include "test.h"

#if TEST
#include <iostream>
#endif

unsigned short crcTable[256];

void generateCrcTable(void) {
	for (int b = 0;;) {
		int v = b;
		for (int i = 8; i != 0; i--) {
			if ((v & 1) == 1) {
				v = (v >> 1) ^ KEY;
			} else
				v = v >> 1;
		}
		crcTable[b] = static_cast<unsigned short>(v & 0xffff);
		if (++b == 256)
			break;
	}
}

unsigned short calculateCrc(unsigned char* data, int dataLength) {
	generateCrcTable();
	unsigned short calculatedCrc = INITIAL_CRC;
	for (int i = 0; i < dataLength; i++) {
		calculatedCrc =
				static_cast<unsigned short>(((calculatedCrc & 0xFFFF) >> 8)
						^ crcTable[(calculatedCrc
								^ static_cast<unsigned short>(data[i])) & 0xFF]);
#if DETAILED_TEST
		printf("0x%02X ", data[i]);
		if ((i + 1) % 16 == 0)
		printf("\n");
#endif
	}
#if DETAILED_TEST
	printf("\n");
#endif
	return (((calculatedCrc ^ 0xFFFF) << 8) | ((calculatedCrc ^ 0xFFFF) >> 8))
			& 0xFFFF;
}
