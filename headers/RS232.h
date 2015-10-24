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

#ifndef RS232_H_
#define RS232_H_

#include "hdlc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BAUDRATE_921600 B921600
#define BAUDRATE_115200 B115200
#define BAUDRATE_57600 B57600
#define BAUDRATE_38400 B38400
#define BAUDRATE_28800 B28800
#define BAUDRATE_19200 B19200
#define BAUDRATE_14400 B14400
#define BAUDRATE_9600 B9600
#define BAUDRATE_4800 B4800

void RS232Init(char* device, int baudrate);

void RS232Terminate();

void senden(char* testText, int length, char* buffer, int* bufferLength);

void RS232DataRequest(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength,
		func_ptr callback);

#ifdef __cplusplus
}
#endif

#endif /* RS232_H_ */
