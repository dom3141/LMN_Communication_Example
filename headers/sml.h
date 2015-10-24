/* Copyright 2015 Dominik Lüthe
 * Labor Datentechnik, Ostfalia Hochschule Braunschweig/Wolfenbüttel
 *
 * based on code by Bowei He copyright 2012 Gateway
 *
 * based on code by Juri Glass, Mathias Runge, Nadim El Sayed copyright 2011 libSML
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

#ifndef SML_H_
#define SML_H_

#include <sml/sml_file.h>
#include <sml/sml_transport.h>
#include <sml/sml_crc16.h>

#ifdef __cplusplus
extern "C" {
#endif

int smlGenerateGetProcParameterRequest(unsigned char *meterID,
		unsigned char *data, int dataLength, unsigned char *obis,
		int obisLength);
int getValueFromSmlMessage(unsigned char *completeSmlFile, int SmlFileLength,
		int* errorCode);
int smlGenerateFile(sml_file *file, unsigned char *data, int dataLength);

#ifdef __cplusplus
}
#endif

#endif /* SML_H_ */
