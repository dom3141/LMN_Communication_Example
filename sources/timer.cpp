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

#include "timer.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include "hdlc.h"

void timerEventBroadcast(int sig) {
	doHdlcBroadcast();
}

void timerEventRecall(int sig) {
	doHdlcRecall();
}

void startTimer(int timeValue, int timeInterval, timerPtr event) {

	int value_sec = 0, value_usec = 0, interval_sec = 0, interval_usec = 0;

	if (timeValue >= 1000)
		value_sec = timeValue / 1000;
	value_usec = (timeValue % 1000) * 1000;

	if (timeInterval >= 1000)
		interval_sec = timeInterval / 1000;
	interval_usec = (timeInterval % 1000) * 1000;

	struct itimerval timer;
	timer.it_value.tv_sec = value_sec;
	timer.it_value.tv_usec = value_usec;
	timer.it_interval.tv_sec = interval_sec;
	timer.it_interval.tv_usec = interval_usec;
	setitimer(ITIMER_REAL, &timer, NULL);
	signal(SIGALRM, event);
}
