// url_parser_test.c

/*  Copyright 2013 Aalborg University. All rights reserved.
*   
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*  
*  2. Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
*  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
*  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*  
*  The views and conclusions contained in the software and
*  documentation are those of the authors and should not be interpreted
*  as representing official policies, either expressed.
*/

#include "url_parser.c"
#include "unit_test.h"
#include <string.h>

int on_begin_called = 0;
int on_protocol_called = 0;
int on_host_called = 0;
int on_port_called = 0;
int on_path_segment_called = 0;
int on_key_value_called = 0;

int on_begin_called_correctly = 0;
int on_protocol_called_correctly = 0;
int on_host_called_correctly = 0;
int on_port_called_correctly = 0;
int on_path_segment_called_correctly = 0;
int on_key_value_called_correctly = 0;

void reset_values()
{
	on_begin_called_correctly = 0;
	on_protocol_called_correctly = 0;
	on_host_called_correctly = 0;
	on_port_called_correctly = 0;
	on_path_segment_called_correctly = 0;
	on_key_value_called_correctly = 0;

	on_begin_called = 0;
	on_protocol_called = 0;
	on_host_called = 0;
	on_port_called = 0;
	on_path_segment_called = 0;
	on_key_value_called = 0;
}

void on_begin()
{
	on_begin_called_correctly = 1;
}

void on_protocol(const char* protocol, size_t length)
{
	if(strncmp(protocol, "http", 4) == 0)
	{
		on_protocol_called_correctly = 1;
	}
}

void on_host(const char* host, size_t length)
{
	printf("%.*s\n", length , host);

	if(strncmp(host, "localhost", 9) == 0)
	{
		on_host_called_correctly = 1;
	}
}

void on_port(const char* port, size_t length)
{
	if(strncmp(port, "8080", 4) == 0)
	{
		on_port_called_correctly = 1;
	}
}

void on_path_segment(const char* seg, size_t length)
{
	if((length == 6 && strncmp(seg, "device",6) == 0) || (length == 2 && strncmp(seg, "tv",2) == 0) ||
		(length == 1 && (strncmp(seg, "a", 1) == 0 || strncmp(seg,"b", 1) == 0 || strncmp(seg, "c", 1) == 0)))
	{
		on_path_segment_called_correctly++;
	}
}

void on_key_value(const char* key, size_t key_length, const char* value, size_t value_length)
{
	if(key_length == 2 && strncmp(key, "id", 2) == 0 && strncmp(value, "1",1) == 0)
	{
		on_key_value_called_correctly++;
	}

	if(key_length == 5 && strncmp(key, "brand", 5) == 0 && strncmp(value, "Apple",5) == 0)
	{
		on_key_value_called_correctly++;
	}

}

TEST_START("url_parser.c")

TEST(non chunked url parsing)

	reset_values();

	char* url = "http://localhost:8080/device/tv?id=1&brand=Apple";
	int url_len = 48;

	struct url_parser_settings settings = URL_PARSER_SETTINGS_DEFAULT;

 	settings.on_begin = &on_begin;
 	settings.on_protocol = &on_protocol;
 	settings.on_host = &on_host;
 	settings.on_port = &on_port;
 	settings.on_path_segment = &on_path_segment;
 	settings.on_key_value = &on_key_value;

	struct url_parser_instance *instance = up_create(&settings);

	up_add_chunk(instance, url, url_len);
	up_complete(instance);
	up_destroy(instance);

	ASSERT_EQUAL(on_begin_called_correctly,1);
	ASSERT_EQUAL(on_protocol_called_correctly, 1);
	ASSERT_EQUAL(on_host_called_correctly, 1);
	ASSERT_EQUAL(on_port_called_correctly, 1);
	ASSERT_EQUAL(on_path_segment_called_correctly, 2);
	ASSERT_EQUAL(on_key_value_called_correctly, 2);

TSET()

TEST(chunked url parsing)

	reset_values();

	char* url1 = "http://localhost:8080/device/";
	int url1_len = 29;

	char* url2 = "tv?id=1&brand=Apple";
	int url2_len = 19;

	struct url_parser_settings settings = URL_PARSER_SETTINGS_DEFAULT;

 	settings.on_begin = &on_begin;
 	settings.on_protocol = &on_protocol;
 	settings.on_host = &on_host;
 	settings.on_port = &on_port;
 	settings.on_path_segment = &on_path_segment;
 	settings.on_key_value = &on_key_value;

	struct url_parser_instance *instance = up_create(&settings);

	up_add_chunk(instance, url1, url1_len);

	// Check that part of the URL has been parsed
	ASSERT_EQUAL(on_begin_called_correctly,1);
	ASSERT_EQUAL(on_protocol_called_correctly, 1);
	ASSERT_EQUAL(on_host_called_correctly, 1);
	ASSERT_EQUAL(on_port_called_correctly, 1);
	ASSERT_EQUAL(on_path_segment_called_correctly, 1);
	ASSERT_EQUAL(on_key_value_called_correctly, 0);

	up_add_chunk(instance, url2, url2_len);

	up_complete(instance);
	up_destroy(instance);

	ASSERT_EQUAL(on_begin_called_correctly,1);
	ASSERT_EQUAL(on_protocol_called_correctly, 1);
	ASSERT_EQUAL(on_host_called_correctly, 1);
	ASSERT_EQUAL(on_port_called_correctly, 1);
	ASSERT_EQUAL(on_path_segment_called_correctly, 2);
	ASSERT_EQUAL(on_key_value_called_correctly, 2);

TSET()

TEST(fail on bad character)
	reset_values();

	struct url_parser_settings settings = URL_PARSER_SETTINGS_DEFAULT;

 	settings.on_begin = &on_begin;
 	settings.on_protocol = &on_protocol;
 	settings.on_host = &on_host;
 	settings.on_port = &on_port;
 	settings.on_path_segment = &on_path_segment;
 	settings.on_key_value = &on_key_value;

 	struct url_parser_instance *instance = up_create(&settings);

 	up_add_chunk(instance, "/a/b/c/|", 8);

 	up_complete(instance);
	up_destroy(instance);

	ASSERT_EQUAL(on_protocol_called, 0);
	ASSERT_EQUAL(on_host_called,0);
	ASSERT_EQUAL(on_port_called, 0);

	ASSERT_EQUAL(on_begin_called_correctly, 1);
	ASSERT_EQUAL(on_path_segment_called_correctly, 3);

TSET()

TEST_END()