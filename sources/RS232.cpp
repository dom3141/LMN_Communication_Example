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

#include "RS232.h"
#include "hdlc.h"
#include "test.h"

#include <time.h>
#include <sys/time.h>

extern int global_sim;
extern int global_debug;
extern int global_log;
extern int global_flush;
extern FILE *logfile;
extern int fileOpened;
struct termios tio;
int tty_fd;
int timeout = 0;

#define RS232_TIMEOUT 3000 // 3000 * 250 µs = ca. 750 ms

void RS232Init(char* device, int baudrate) {

	if (global_sim == 0) {
		char anyDevice[] = "/dev/ttyUSB ";
		memset(&tio, 0, sizeof(tio));
		tio.c_iflag = 0;
		tio.c_oflag = 0;
		tio.c_cflag = CS8 | CREAD | CLOCAL;       // 8N1
		tio.c_lflag = 0;
		tio.c_cc[VMIN] = 1;
		tio.c_cc[VTIME] = 5;

		if (device[0] == 'A' && device[1] == 'N' && device[2] == 'Y') {
			for (int i = 0; i < 10; i++) {
				anyDevice[11] = i + 0x30;
				tty_fd = open(anyDevice, O_RDWR | O_NONBLOCK);
				if (tty_fd < 0)
					close(tty_fd);
				else
					i = 100; //break;
			}
		} else
			tty_fd = open(device, O_RDWR | O_NONBLOCK);
		switch (baudrate) {
		case 921600:
			cfsetospeed(&tio, BAUDRATE_921600);
			cfsetispeed(&tio, BAUDRATE_921600);
			break;
		default:
			cfsetospeed(&tio, BAUDRATE_115200);
			cfsetispeed(&tio, BAUDRATE_115200);
			break;
		}
		tcsetattr(tty_fd, TCSANOW, &tio);
	}
	return;
}

void RS232Terminate() {
	if (global_sim == 0)
		close(tty_fd);
	return;
}

void senden(char* testText, int length, char* buffer, int* bufferLength) {
	write(tty_fd, testText, length);
	sleep(3);
	*bufferLength = read(tty_fd, buffer, 100);
	return;
}

void RS232DataRequest(unsigned char destAddr, unsigned char sourceAddr,
		protocolSelector protSel, unsigned char *payload, int payloadLength,
		func_ptr callback) {

	int receivedBytes = 0;
	int broadcast = 0;
	int recall = 0;
	char debugString[20000], tempString[20000];
	bool broadcastRequest = false;
	bool frameLengthRead = false;
	unsigned char receiveBuffer[2080];
	unsigned short offset = 0;
	unsigned short frameLength = 9999;
	unsigned short bufferLength;

	bzero((char *) &debugString, 2200);
	bzero((char *) &tempString, 2048);

	if (payloadLength > 3) {
		if (payload[3] == 0xFE)
			broadcastRequest = true;
	}

	timeval curTime;
	int milliSeconds;
	unsigned char debugDestAddr = (payload[3] >> 1) & 0xFF;
	unsigned char debugSourceAddr = (payload[5] >> 1) & 0xFF;
	unsigned char debugProtSel =
			static_cast<protocolSelector>((payload[4] & 0xF) >> 1);
	unsigned char debugControlField = payload[7] & 0xFF;

	if (global_log != 0 || global_debug != 0) {
		gettimeofday(&curTime, NULL);
		//milliSeconds = curTime.tv_usec / 1000;
		milliSeconds = curTime.tv_usec / 100;
		sprintf(tempString,
				"\nSEND DATA: [TIME]=%06d.%04d [DST]=0x%02X [SRC]=0x%02X [PTC]=",
				(int) (time(NULL) & 0x0EFFFF), milliSeconds, debugDestAddr,
				debugSourceAddr);
		strcat(debugString, tempString);

		if (debugDestAddr == 0x7F) {
			if ((debugProtSel & 0xFF) == 0x1) {
				sprintf(tempString, "BROADCAST");
			}
			if ((debugProtSel & 0xFF) == 0x2) {
				sprintf(tempString, "RECALL");
			}
		} else {
			switch (debugProtSel & 0xFF) {
			case 0x1:
				sprintf(tempString, "TLS_SML_COSEM");
				break;
			case 0x2:
				sprintf(tempString, "TLS");
				break;
			case 0x3:
				sprintf(tempString, "SML_COSEM");
				break;
			case 0x4:
				sprintf(tempString, "SML_EDL");
				break;
			case 0x5:
				sprintf(tempString, "SML_SYM");
				break;
			case 0x6:
				sprintf(tempString, "SYM_HDLC");
				break;
			case 0x7:
				sprintf(tempString, "TLS_SML_EDL");
				break;
			case 0x8:
				sprintf(tempString, "TLS_SML_SYM");
				break;
			default:
				sprintf(tempString, "SELECTOR_ERROR");
				break;
			}
		}
		strcat(debugString, tempString);
		sprintf(tempString, " [CTR]=");
		strcat(debugString, tempString);
		if (((debugControlField & 0xFF) & 0x1) == 0x00)
			sprintf(tempString, "I");
		if (((debugControlField & 0xFF) & 0xF) == 0x01)
			sprintf(tempString, "RR");
		if (((debugControlField & 0xFF) & 0xF) == 0x05)
			sprintf(tempString, "RNR");
		if (((debugControlField & 0xFF) & 0xEF) == 0x83)
			sprintf(tempString, "SNRM");
		if (((debugControlField & 0xFF) & 0xEF) == 0x43)
			sprintf(tempString, "DISC");
		if (((debugControlField & 0xFF) & 0xEF) == 0x63)
			sprintf(tempString, "UA");
		if (((debugControlField & 0xFF) & 0xEF) == 0x0F)
			sprintf(tempString, "DM");
		if (((debugControlField & 0xFF) & 0xEF) == 0x87)
			sprintf(tempString, "FRMR");
		if (((debugControlField & 0xFF) & 0xEF) == 0x03)
			sprintf(tempString, "UI");
		strcat(debugString, tempString);
		sprintf(tempString, "\n");
		strcat(debugString, tempString);
	}

	for (int i = 0; i < payloadLength; i++) {
		sprintf(tempString, "0x%02X ", payload[i]);
		strcat(debugString, tempString);
		if (i != 0 && (i + 1) % 16 == 0) {
			sprintf(tempString, "\n");
			strcat(debugString, tempString);
		}
	}

	timeout = 0;
	write(tty_fd, payload, payloadLength);
	if (broadcastRequest) {
		usleep(900 * 1000);
		receivedBytes = read(tty_fd, payload, 2060);
	} else {
		do {
			usleep(250);
			timeout++;
			receivedBytes = read(tty_fd, receiveBuffer, 2060);
			for (int i = 0; i < receivedBytes; i++)
				payload[offset++] = receiveBuffer[i];

			if (offset > 2 && frameLengthRead == false) {
				frameLength = (((payload[1] - 0xA0) << 8) + payload[2]) + 2;
				frameLengthRead = true;
			}
		} while (offset != frameLength && timeout < RS232_TIMEOUT);
		if (timeout >= RS232_TIMEOUT)
			receivedBytes = 0;
		else
			receivedBytes = frameLength;
	}

	gettimeofday(&curTime, NULL);
	milliSeconds = curTime.tv_usec / 100;
	sprintf(tempString, "\n\nRECEIVE DATA: [TIME]=%06d.%04d ",
			(int) (time(NULL) & 0x0EFFFF), milliSeconds);
	strcat(debugString, tempString);

	if (receivedBytes <= 0) {
		sprintf(tempString, "******* NO REPLY *******");
		strcat(debugString, tempString);
		sprintf(tempString,
				"\n\n-------------------------------------------------------------------------------\n");
		strcat(debugString, tempString);
	} else if (payload[0] != 0x7E || payload[receivedBytes - 1] != 0x7E) {
		sprintf(tempString, "******* FRAME ERROR *******\n");
		strcat(debugString, tempString);
		for (int i = 0; i < receivedBytes; i++) {
			sprintf(tempString, "0x%02X ", payload[i]);
			strcat(debugString, tempString);
			if (i != 0 && (i + 1) % 16 == 0) {
				sprintf(tempString, "\n");
				strcat(debugString, tempString);
			}
		}
		sprintf(tempString,
				"\n\n-------------------------------------------------------------------------------\n");
		strcat(debugString, tempString);
	}

	if (global_debug != 0)
		printf(debugString);

	if (global_log != 0) {
		while (fileOpened == 0)
			;
		fprintf(logfile, debugString);

		if (global_flush != 0) {
			fflush(logfile);
			fsync(fileno(logfile));
		}
	}
	callback(destAddr, sourceAddr, protSel, payload, receivedBytes);

	return;
}
