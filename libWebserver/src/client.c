// client.c

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

#include "client.h"
#include "instance.h"
#include "request.h"
#include "response.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <ev.h>

/// The maximum data size we can recieve or send
#define MAXDATASIZE 1024

/// The amount of time a client may be inactive, in seconds
#define TIMEOUT 15

/// All data to represent a client
struct ws_client {
   struct ws *instance;             ///< Webserver instance
   struct ws_settings *settings;    ///< Webserver settings
   char ip[INET6_ADDRSTRLEN];       ///< IP address of the client
   struct ev_loop *loop;            ///< The event loop
   struct ev_timer timeout_watcher; ///< Timeout watcher
   struct ev_io recv_watcher;       ///< Recieve watcher
   struct ev_io send_watcher;       ///< Send watcher
   struct ws_request *request;      ///< Current request in progress
   char send_msg[MAXDATASIZE];      ///< Data to send
};

/// Get the in_addr from a sockaddr (IPv4 or IPv6)
/**
 *  Get the in_addr for either IPv4 or IPv6. The type depends on the
 *  protocal, which is why this returns a void pointer. It works nicely
 *  to pass the result of this function as the second argument to
 *  inet_ntop().
 *
 *  \param sa The sockaddr
 *
 *  \return An in_addr or in6_addr depending on the protocol.
 */
static void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET) {
      // IPv4
      return &(((struct sockaddr_in*)sa)->sin_addr);
   } else {
      // IPv6
      return &(((struct sockaddr_in6*)sa)->sin6_addr);
   }
}

static void client_recv_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
   ssize_t recieved;
   size_t parsed;
   char buffer[MAXDATASIZE];
   struct ws_client *client = watcher->data;

   printf("recieving data from %s\n", client->ip);

   // Receive some data
   if ((recieved = recv(watcher->fd, buffer, MAXDATASIZE-1, 0)) < 0) {
      if (recieved == -1 && errno == EWOULDBLOCK) {
         fprintf(stderr, "libev callbacked called without data to " \
                         "recieve (client: %s)", client->ip);
         return;
      }
      perror("recv");
      // TODO Handle errors better - look up error# etc.
      ws_client_kill(client);
      return;
   } else if (recieved == 0) {
      printf("connection closed by %s\n", client->ip);
      ws_client_kill(client);
      return;
   }

   if (client->request == NULL) {
      client->request = ws_request_create(client, client->settings);
   }

   // Parse recieved data
   parsed = ws_request_parse(client->request, buffer, recieved);
   if (parsed != recieved) {
      perror("parse");
      ws_client_kill(client);
      return;
   }

   // Reset timeout
   client->timeout_watcher.repeat = TIMEOUT;
   ev_timer_again(loop, &client->timeout_watcher);
}

static void client_send_cb(struct ev_loop *loop, struct ev_io *watcher,
      int revents)
{
   struct ws_client *client = watcher->data;

   printf("sending response to %s\n", client->ip);
   if (send(watcher->fd, client->send_msg, strlen(client->send_msg), 0) == -1)
      perror("send");
   ev_io_stop(client->loop, &client->send_watcher);

   ws_client_kill(client);
}

static void client_timeout_cb(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
   struct ws_client *client = watcher->data;
   printf("timeout on %s\n", client->ip);
   ws_client_kill(client);
}

void ws_client_sendf(struct ws_client *client, char *fmt, ...) {
   int status;
   va_list arg;

   va_start(arg, fmt);
   status = vsnprintf(client->send_msg, MAXDATASIZE, fmt, arg);
   va_end(arg);

   if (status >= MAXDATASIZE) {
      fprintf(stderr, "Data is too large to send!");
      ws_client_kill(client);
      return;
   }

   ev_io_start(client->loop, &client->send_watcher);
}

/// Initialise and accept client
/**
 *  This function is designed to be used as a callback function within
 *  LibEV. It will accept the conncetion as described inside the file
 *  descripter within the watcher. It will also add timeout and io
 *  watchers to the loop, which will handle the further communication
 *  with the client.
 *
 *  \param loop The running event loop.
 *  \param watcher The watcher that was tiggered on the connection.
 *  \param revents Not used.
 */
void ws_client_accept(
      struct ev_loop *loop,
      struct ev_io *watcher,
      int revents)
{
   char ip_string[INET6_ADDRSTRLEN];
   int in_fd;
   socklen_t in_size;
   struct sockaddr_storage in_addr;
   struct ws_client *client;
   
   // Accept connection
   in_size = sizeof in_addr;
   if ((in_fd = accept(watcher->fd, (struct sockaddr *)&in_addr, &in_size)) < 0) {
      perror("accept");
      return;
   }

   // Print a nice message
   inet_ntop(in_addr.ss_family,
         get_in_addr((struct sockaddr *)&in_addr),
         ip_string,
         sizeof ip_string);
   printf("got connection from %s\n", ip_string);

   // Create client and parser
   client = malloc(sizeof(struct ws_client));
   if (client == NULL) {
      fprintf(stderr, "ERROR: Cannot accept client (malloc return NULL)\n");
      return;
   }
   client->instance = watcher->data;
   client->settings = ws_instance_get_settings(client->instance); 
   strcpy(client->ip, ip_string);
   client->loop = loop;
   client->timeout_watcher.data = client;
   client->recv_watcher.data = client;
   client->send_watcher.data = client;
   client->request = NULL;

   // Set up list
   ws_instance_add_client(client->instance, client);

   // Start timeout and io watcher
   ev_io_init(&client->recv_watcher, client_recv_cb, in_fd, EV_READ);
   ev_io_init(&client->send_watcher, client_send_cb, in_fd, EV_WRITE);
   ev_io_start(loop, &client->recv_watcher);
   ev_init(&client->timeout_watcher, client_timeout_cb);
   client->timeout_watcher.repeat = TIMEOUT;
   ev_timer_again(loop, &client->timeout_watcher);
}

/// Kill and clean up after a client
/**
 *  This function stops the LibEV watchers, closes the socket, and frees
 *  the data structures used be a client.
 *
 *  \param client The client to kill.
 */
void ws_client_kill(struct ws_client *client) {
   // Stop watchers
   int sockfd = client->recv_watcher.fd;
   ev_io_stop(client->loop, &client->recv_watcher);
   ev_io_stop(client->loop, &client->send_watcher);
   ev_timer_stop(client->loop, &client->timeout_watcher);

   // Close socket
   if (close(sockfd) != 0) {
      perror("close");
   }

   // Remove from list
   ws_instance_rm_client(client->instance, client);

   // Cleanup
   free(client);
}

