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

#ifndef SENDOBJECT_H_
#define SENDOBJECT_H_


#include "hdlc.h"

#ifdef __cplusplus
extern "C" {
#endif

	class SendObject
	{

	private:

	public:

		unsigned char destAddr;
		unsigned char sourceAddr;
		protocolSelector protSel;
		controlFieldFormat controlField;
		unsigned char *payload;
		int payloadLength;
		int delay_ms;
		func_ptr callback;

		SendObject();
		~SendObject();

	};


#ifdef __cplusplus
}
#endif

#endif /* SENDOBJECT_H_ */
