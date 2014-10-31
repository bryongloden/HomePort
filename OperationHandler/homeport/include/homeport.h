/*Copyright 2011 Aalborg University. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed*/

#ifndef HOMEPORT_H
#define HOMEPORT_H

#include <stddef.h>

typedef struct HomePort HomePort;
typedef struct Adapter Adapter;
typedef struct Device Device;
typedef struct Service Service;
typedef struct Configuration Configuration;

typedef int (*init_f)(HomePort *homeport, void *data);
typedef void (*deinit_f)(HomePort *homeport, void *data);
typedef void   (*serviceGetFunction) (Service* service, void *request);
typedef size_t (*servicePutFunction) (Service* service, char *buffer, size_t max_buffer_size, char *put_value);

struct ev_loop;
struct lr;

struct HomePort
{
  struct lr *rest_interface;
  Configuration *configuration;
  struct ev_loop *loop;
};

// Homeport Service Control
HomePort *homePortNew( struct ev_loop *loop, int port );
void      homePortFree(HomePort *homeport);
int       homePortStart(HomePort *homeport);
void      homePortStop(HomePort *homeport);
int       homePortEasy(init_f init, deinit_f deinit, void *data, int port );

// Configurator Interface
int  homePortAddAdapter( HomePort *homeport, Adapter *adapter );
int  homePortRemoveAdapter( HomePort *homeport, Adapter *adapter );
int  homePortAttachDevice( HomePort *homeport, Adapter *adapter, Device *device );
int  homePortDetachDevice( HomePort *homeport, Device *device );

// Communication interface
void homePortSendState(Service *service, void *req_in, const char *val, size_t len);

#endif
