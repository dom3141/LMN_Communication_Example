/* Copyright 2015 Matthias Köhne, Dominik Lüthe
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

#ifndef FUNCPTR_H_
#define FUNCPTR_H_

#include "lmnsubscriber.h"

#ifdef __cplusplus
extern "C" {
#endif

enum Prot_Sel {
	NO_CONNECTION = 0x0,
	TLS_SML_COSEM = 0x1,
	TLS = 0x2,
	SML_COSEM = 0x3,
	SML_EDL = 0x4,
	SML_SYM = 0x5,
	SYM_HDLC = 0x6,
	TLS_SML_EDL = 0x7,
	TLS_SML_SYM = 0x8,
	SELECTOR_ERROR = 0xFF
};

typedef enum Prot_Sel protocolSelector;

#ifdef __cplusplus
}
#endif

#endif /* FUNCPTR_H_ */
