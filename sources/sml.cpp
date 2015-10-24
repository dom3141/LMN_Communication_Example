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

#include "sml.h"
#include "hdlc.h"
#include "lmnadmin.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

int smlGenerateGetProcParameterRequest(unsigned char *meterID,
		unsigned char *data, int dataLength, unsigned char *obis,
		int obisLength) {
	char smlIdHex[] = "00000000";
	unsigned char meterAddress = getHdlcAddress(meterID);
	int smlTransactionId = getSmlMessageNo(
			static_cast<unsigned short>(meterAddress));
	sml_file *newFile = sml_file_init();
	sml_open_request *openRequest = sml_open_request_init();
	sml_message_body *openBody = sml_message_body_init(SML_MESSAGE_OPEN_REQUEST,
			openRequest);
	sml_message_body *closeBody = sml_message_body_init(
			SML_MESSAGE_CLOSE_REQUEST, openRequest);
	octet_string *codepage = NULL;   //OPTIONAL
	char clientIdString[] = "FEEDDCCBBAABBCCDDEEF";
	for (int i = 0; i < 10; i++) {
		clientIdString[2 * i] =
				(meterID[i] >> 4) < 10 ?
						(meterID[i] >> 4) + 0x30 : (meterID[i] >> 4) + 0x37;
		clientIdString[2 * i + 1] =
				(meterID[i] & 0x0F) < 10 ?
						(meterID[i] & 0x0F) + 0x30 : (meterID[i] & 0x0F) + 0x37;
	}
	octet_string *client_id = sml_octet_string_init_from_hex(clientIdString); //clientId
	octet_string *req_file_id = sml_octet_string_init_from_hex("30"); //u32 counter
	octet_string *server_id = sml_octet_string_init_from_hex(
			"00112233445566778899"); //serverId OPTIONAL
	sml_time *ref_time = NULL; //OPTIONAL
	u8 *sml_version = NULL;    //OPTIONAL
	openRequest->codepage = codepage;
	openRequest->client_id = client_id;
	openRequest->req_file_id = req_file_id;
	openRequest->sml_version = sml_version;
	sml_message *openMessage = (sml_message *) malloc(sizeof(sml_message));
	memset(openMessage, 0, sizeof(sml_message));
	sprintf(smlIdHex, "%08X", smlTransactionId);
	octet_string *transaction_id = sml_octet_string_init_from_hex(smlIdHex);
	u8 *group_id = sml_u8_init(0);
	u8 *abort_on_error = sml_u8_init(0);
	u16 *crc = sml_u16_init(0);
	openMessage->transaction_id = transaction_id;
	openMessage->group_id = group_id;
	openMessage->abort_on_error = abort_on_error;
	openMessage->message_body = openBody;
	openMessage->crc = crc;

	sml_get_proc_parameter_request *parameterRequest =
			sml_get_proc_parameter_request_init();
	sml_tree_path *treePath = sml_tree_path_init();
	//octet_string *obisKennzahl = sml_octet_string_init_from_hex("0100010800FF800200");
	octet_string *obisKennzahl = sml_octet_string_init(obis, obisLength);
	octet_string *cosemAttributRef1 = sml_octet_string_init_from_hex("0001");
	octet_string *cosemAttributRef2 = sml_octet_string_init_from_hex("0002");
	octet_string *cosemAttributRef3 = sml_octet_string_init_from_hex("0004");
	sml_tree_path_add_path_entry(treePath, obisKennzahl);
	sml_tree_path_add_path_entry(treePath, cosemAttributRef1);
	sml_tree_path_add_path_entry(treePath, cosemAttributRef2);
	//sml_tree_path_add_path_entry(treePath, cosemAttributRef3);
	parameterRequest->parameter_tree_path = treePath;
	sml_message *requestMessage = (sml_message *) malloc(sizeof(sml_message));
	memset(requestMessage, 0, sizeof(sml_message));
	sprintf(smlIdHex, "%08X", smlTransactionId + 1);
	transaction_id = sml_octet_string_init_from_hex(smlIdHex);
	requestMessage->transaction_id = transaction_id;
	requestMessage->group_id = group_id;
	requestMessage->abort_on_error = abort_on_error;
	requestMessage->message_body = sml_message_body_init(
			SML_MESSAGE_GET_PROC_PARAMETER_REQUEST, parameterRequest);
	//requestMessage->message_body = closeBody;
	requestMessage->crc = crc;

	sml_close_request *closeRequest = sml_close_request_init();
	closeRequest->global_signature = NULL;
	sml_message *closeMessage = (sml_message *) malloc(sizeof(sml_message));
	memset(closeRequest, 0, sizeof(sml_message));
	sprintf(smlIdHex, "%08X", smlTransactionId + 2);
	transaction_id = sml_octet_string_init_from_hex(smlIdHex);
	closeMessage->transaction_id = transaction_id;
	closeMessage->group_id = group_id;
	closeMessage->abort_on_error = abort_on_error;
	closeMessage->message_body = sml_message_body_init(
			SML_MESSAGE_CLOSE_REQUEST, closeRequest);
	closeMessage->crc = crc;
	sml_file_add_message(newFile, openMessage);
	sml_file_add_message(newFile, requestMessage);
	sml_file_add_message(newFile, closeMessage);

	setSmlMessageNo(static_cast<unsigned short>(meterAddress),
			smlTransactionId + 3);
	int ret = smlGenerateFile(newFile, data, dataLength);
	return ret;
}

#ifdef MIT_DOUBLE
double getValueFromSmlMessage(unsigned char *completeSmlFile, int SmlFileLength, int* errorCode) {
	sml_file *file = sml_file_init();
	file = sml_file_parse(completeSmlFile + 8, SmlFileLength - 16);
	*errorCode = 0;

	if (*(file->messages[1]->message_body->tag) == SML_MESSAGE_ATTENTION_RESPONSE) {
		*errorCode = -1;
		return 0.0;    //then fehler, SML-Attention
	}

	if (file->messages_len != 3) {
		*errorCode = -2;
		return 0.0;    //then fehler, da nicht open, response und close
	}

	if ((*(file->messages[1]->message_body->tag)) != SML_MESSAGE_GET_PROC_PARAMETER_RESPONSE) { // 0x501
		*errorCode = -3;
		return 0.0;//then fehler, da nicht SML-GetProcParameter.Response
	}

//hier könnte der Sekundenindex gelesen werden
//sml_open_response *openResponse;
//openResponse = (sml_open_response*)(file->messages[0]->message_body->data);
//unsigned int sekIndex = (unsigned int) *(openResponse->ref_time->data.sec_index);

	sml_get_proc_parameter_response *parameterResponse;
	//msg_body->tag = sml_u32_parse(buf);
	//if (sml_buf_has_errors(buf))
	//parameterResponse = static_cast<sml_get_proc_parameter_response*>(file->messages[1]->message_body->data);
	parameterResponse = (sml_get_proc_parameter_response*)(file->messages[1]->message_body->data);
	sml_value *value = parameterResponse->parameter_tree->child_list[1]->parameter_value->data.value;

#ifdef NEED_MORE_INFO
	printf("\n->parameter_tree_path->path_entries_len: %d \n", parameterResponse->parameter_tree_path->path_entries_len);
	printf("\n->parameter_tree_path->path_entries[2]->len: %d \n", parameterResponse->parameter_tree_path->path_entries[2]->len);
	printf("\n->parameter_tree->child_list_len: %d \n", parameterResponse->parameter_tree->child_list_len);
	printf("\n->parameter_tree->child_list[0]->child_list_len: %d \n", parameterResponse->parameter_tree->child_list[0]->child_list_len);
	printf("\n->parameter_tree->child_list[1]->child_list_len: %d \n", parameterResponse->parameter_tree->child_list[1]->child_list_len);
	printf("\n->parameter_tree->child_list[1]->parameter_value->tag: %d \n", *(parameterResponse->parameter_tree->child_list[1]->parameter_value->tag));

	printf("\nvalue->data.int32: 0x%X \n", *(value->data.int32));
#endif
	//printf("\nmeterValue: %.4lf\n", static_cast<double>(*(value->data.int32))/10000);

	double meterValue = (double)(*(value->data.int32))/10000;
	//printf("\nmeterValue: %.4lf\n", meterValue);
	sml_file_free(file);
	return meterValue;
}
#endif

int getValueFromSmlMessage(unsigned char *completeSmlFile, int SmlFileLength,
		int* errorCode) {
	sml_file *recvFile = sml_file_init();
	recvFile = sml_file_parse(completeSmlFile + 8, SmlFileLength - 16);
	*errorCode = 0;
	/*
	 if (global_debug == 1) {
	 printf("\n\ngetValueFromSmlMessage called...\n\n");

	 printf("SmlFileLength: %d\n", SmlFileLength);

	 printf("\n\nReceived SML-Message:\n\n");
	 for (int i = 0; i < SmlFileLength; i++) {
	 printf("0x%02X ", completeSmlFile[i]);
	 if (i != 0 && (i + 1) % 16 == 0) {
	 printf("\n");
	 }
	 }
	 printf("\n\n");
	 }
	 */
	if (*(recvFile->messages[1]->message_body->tag)
			== SML_MESSAGE_ATTENTION_RESPONSE) {
		*errorCode = -1;
		return 0;	//then fehler, SML-Attention
	}

	if (recvFile->messages_len != 3) {
		*errorCode = -2;
		return 0;	//then fehler, da nicht open, response und close
	}

	if ((*(recvFile->messages[1]->message_body->tag))
			!= SML_MESSAGE_GET_PROC_PARAMETER_RESPONSE) { // 0x501
		*errorCode = -3;
		return 0; //then fehler, da nicht SML-GetProcParameter.Response
	}

//hier könnte der Sekundenindex gelesen werden
//sml_open_response *openResponse;
//openResponse = (sml_open_response*)(recvFile->messages[0]->message_body->data);
//unsigned int sekIndex = (unsigned int) *(openResponse->ref_time->data.sec_index);

	sml_get_proc_parameter_response *parameterResponse;
	//msg_body->tag = sml_u32_parse(buf);
	//if (sml_buf_has_errors(buf))
	//parameterResponse = static_cast<sml_get_proc_parameter_response*>(recvFile->messages[1]->message_body->data);
	parameterResponse =
			(sml_get_proc_parameter_response*) (recvFile->messages[1]->message_body->data);
	sml_value *value =
			parameterResponse->parameter_tree->child_list[1]->parameter_value->data.value;

#ifdef NEED_MORE_INFO
	printf("\n->parameter_tree_path->path_entries_len: %d \n", parameterResponse->parameter_tree_path->path_entries_len);
	printf("\n->parameter_tree_path->path_entries[2]->len: %d \n", parameterResponse->parameter_tree_path->path_entries[2]->len);
	printf("\n->parameter_tree->child_list_len: %d \n", parameterResponse->parameter_tree->child_list_len);
	printf("\n->parameter_tree->child_list[0]->child_list_len: %d \n", parameterResponse->parameter_tree->child_list[0]->child_list_len);
	printf("\n->parameter_tree->child_list[1]->child_list_len: %d \n", parameterResponse->parameter_tree->child_list[1]->child_list_len);
	printf("\n->parameter_tree->child_list[1]->parameter_value->tag: %d \n", *(parameterResponse->parameter_tree->child_list[1]->parameter_value->tag));

	printf("\nvalue->data.int32: 0x%X \n", *(value->data.int32));
#endif
	//printf("\nmeterValue: %.4lf\n", static_cast<double>(*(value->data.int32))/10000);

	int meterValue = *(value->data.int32);
	//printf("\n\n\nSML: DIES IST DER WERT: %d \n\n\n", meterValue);
	//printf("\nmeterValue: %.4lf\n", meterValue);
	sml_file_free(recvFile);
	return meterValue;
}

int smlGenerateFile(sml_file *file, unsigned char *data, int dataLength) {

	if (dataLength < 20)
		return 0;

	unsigned char start_seq[] =
			{ 0x1b, 0x1b, 0x1b, 0x1b, 0x01, 0x01, 0x01, 0x01 };
	unsigned char end_seq[] = { 0x1b, 0x1b, 0x1b, 0x1b, 0x1a };

	sml_buffer *buf = file->buf;
	buf->cursor = 0;

	// add start sequence
	memcpy(sml_buf_get_current_buf(buf), start_seq, 8);
	buf->cursor += 8;

	// add file
	sml_file_write(file);

	// add padding bytes
	int padding = (buf->cursor % 4) ? (4 - buf->cursor % 4) : 0;
	if (padding) {
		// write zeroed bytes
		memset(sml_buf_get_current_buf(buf), 0, padding);
		buf->cursor += padding;
	}

	// begin end sequence
	memcpy(sml_buf_get_current_buf(buf), end_seq, 5);
	buf->cursor += 5;

	// add padding info
	buf->buffer[buf->cursor++] = (unsigned char) padding;

	// add crc checksum
	u16 crc = sml_crc16_calculate(buf->buffer, buf->cursor);
	buf->buffer[buf->cursor++] = (unsigned char) ((crc & 0xFF00) >> 8);
	buf->buffer[buf->cursor++] = (unsigned char) (crc & 0x00FF);

	if (dataLength < buf->cursor)
		return 0;

	int i;
	for (i = 0; i < buf->cursor; i++) {
		data[i] = buf->buffer[i];
	}

	/*
	 //printf("Laenge: %d \n", buf->cursor);
	 for (int i = 0; i < buf->cursor; i++) {
	 printf("0x%02X ", buf->buffer[i]);
	 if ((i+1)%16 == 0)
	 printf("\n");
	 }
	 printf("\n");
	 */
	/*
	 size_t wr = write(fd, buf->buffer, buf->cursor);
	 if (wr == buf->cursor) {
	 return wr;
	 }
	 */

	return buf->cursor - 1;
}

