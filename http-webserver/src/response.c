// reponse.c

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

#include "response.h"
#include "request.h"
#include "http-webserver.h"
#include "webserver.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define HTTP_VERSION "HTTP/1.1 "
#define CRLF "\r\n"

struct http_response
{
   struct ws_conn *conn;
   char *msg;
};

static char* http_status_codes_to_str(enum httpws_http_status_code status)
{
#define XX(num, str) if(status == num) {return #str;}
	HTTPWS_HTTP_STATUS_CODE_MAP(XX)
#undef XX
	return NULL;
}

void http_response_destroy(struct http_response *res)
{
   free(res->msg);
   free(res);
}

struct http_response *http_response_create(
      struct http_request *req,
      enum httpws_http_status_code status)
{
   struct http_response *res = NULL;
   int len;

   // Get data
   char *status_str = http_status_codes_to_str(status);

   // Calculate msg length
   len = 1;
   len += strlen(HTTP_VERSION);
   len += strlen(status_str);
   len += strlen(CRLF);
   
   // Allocate space
   res = malloc(sizeof(struct http_response));
   if (res == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory\n");
      return NULL;
   }
   res->msg = malloc(len*sizeof(char));
   if (res == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory\n");
      http_response_destroy(res);
      return NULL;
   }
  
   // Init struct
   res->conn = http_request_get_connection(req);

   // Construct msg
   strcpy(res->msg, HTTP_VERSION);
   strcat(res->msg, status_str);
   strcat(res->msg, CRLF);

   return res;
}

int http_response_add_header(struct http_response *res, const char *field, const char *value)
{
	char *msg;
	int msg_len = strlen(res->msg)+strlen(field)+1+strlen(value)+strlen(CRLF)+1;

	msg = realloc(res->msg, msg_len*sizeof(char));
	 if (msg == NULL) {
      	fprintf(stderr, "ERROR: Cannot allocate memory\n");
      	return 1;
   }
   res->msg = msg;

   strcat(res->msg, field);
   strcat(res->msg, ":");
   strcat(res->msg, value);
   strcat(res->msg, CRLF);

   return 0;
}

void http_response_send(struct http_response *res, const char* body)
{
   fprintf(stderr, "http_response_send is in a very early alpha version!\n");

   if(body != NULL) {
      ws_conn_sendf(res->conn, "%s%s%s", res->msg, CRLF, body);
      ws_conn_close(res->conn);
   } else {
   	ws_conn_sendf(res->conn, "%s%s", res->msg, CRLF);
      ws_conn_close(res->conn);
   }
}
