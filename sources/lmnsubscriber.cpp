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

#include "lmnsubscriber.h"

unsigned short LmnSubscriber::tlnNo = 0;

LmnSubscriber::LmnSubscriber() {
	address = 0;
	myReceiveSequenceNumber = 0;
	mySendSequenceNumber = 0;
	itsReceiveSequenceNumber = 0;
	itsSendSequenceNumber = 0;
	smlMessageNo = 0;
	tlnNo++;
	tlnIdField = new unsigned char[32];
	channel = closed;
	connectionType = NO_CONNECTION;
	tlnStatus = inactive;
	tlnTlsState = uninitialized;
}

LmnSubscriber::LmnSubscriber(unsigned short address) {
	this->address = address;
	myReceiveSequenceNumber = 0;
	mySendSequenceNumber = 0;
	itsReceiveSequenceNumber = 0;
	itsSendSequenceNumber = 0;
	smlMessageNo = 0;
	tlnNo++;
	tlnIdField = new unsigned char[32];
	channel = closed;
	connectionType = NO_CONNECTION;
	tlnStatus = inactive;
	tlnTlsState = uninitialized;
}

LmnSubscriber::~LmnSubscriber() {
	tlnNo--;
}

unsigned short LmnSubscriber::getAddress(void) {
	return this->address;
}

unsigned short LmnSubscriber::getMyReceiveSequenceNumber(void) {
	return this->myReceiveSequenceNumber;
}

unsigned short LmnSubscriber::getItsReceiveSequenceNumber(void) {
	return this->itsReceiveSequenceNumber;
}

unsigned short LmnSubscriber::getMySendSequenceNumber(void) {
	return this->mySendSequenceNumber;
}

unsigned short LmnSubscriber::getItsSendSequenceNumber(void) {
	return this->itsSendSequenceNumber;
}

status LmnSubscriber::getTlnStatus(void) {
	return this->tlnStatus;
}

void LmnSubscriber::setAddress(unsigned short address) {
	this->address = address;
	return;
}

void LmnSubscriber::setMyReceiveSequenceNumber(
		unsigned short myReceiveSequenceNumber) {
	this->myReceiveSequenceNumber = myReceiveSequenceNumber;
	return;
}

void LmnSubscriber::setItsReceiveSequenceNumber(
		unsigned short itsReceiveSequenceNumber) {
	this->itsReceiveSequenceNumber = itsReceiveSequenceNumber;
	return;
}

void LmnSubscriber::setMySendSequenceNumber(
		unsigned short mySendSequenceNumber) {
	this->mySendSequenceNumber = mySendSequenceNumber;
	return;
}

void LmnSubscriber::setItsSendSequenceNumber(
		unsigned short itsSendSequenceNumber) {
	this->itsSendSequenceNumber = itsSendSequenceNumber;
	return;
}

void LmnSubscriber::setTlnStatus(status tlnStatus) {
	this->tlnStatus = tlnStatus;
	return;
}

void LmnSubscriber::getTlnIdField(unsigned char* field,
		unsigned short fieldLength) {
	fieldLength = fieldLength > 32 ? 32 : fieldLength;
	for (int i = 0; i < fieldLength; i++)
		field[i] = tlnIdField[i];
	return;
}

void LmnSubscriber::setTlnIdField(unsigned char* field,
		unsigned short fieldLength) {
	fieldLength = fieldLength > 32 ? 32 : fieldLength;
	for (int i = 0; i < fieldLength; i++)
		tlnIdField[i] = field[i];
	return;
}

void LmnSubscriber::setConnectionType(protocolSelector connectionType) {
	this->connectionType = connectionType;
	return;
}

protocolSelector LmnSubscriber::getConnectionType() {
	return this->connectionType;
}

channelStatus LmnSubscriber::getChannelStatus() {
	return this->channel;
}

void LmnSubscriber::setChannelStatus(channelStatus status) {
	this->channel = status;
	return;
}

void LmnSubscriber::setTlnTlsState(tlsState tlnTlsState) {
	this->tlnTlsState = tlnTlsState;
	return;
}

tlsState LmnSubscriber::getTlnTlsState() {
	return this->tlnTlsState;
}

void LmnSubscriber::setSmlMessageNo(int messageNumber) {
	this->smlMessageNo = messageNumber;
}

int LmnSubscriber::getSmlMessageNo() {
	return this->smlMessageNo;
}
