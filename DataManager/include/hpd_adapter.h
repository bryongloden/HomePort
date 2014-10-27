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

/**
 * @file hpd_adapter.h
 * @brief  Methods for managing the Service structure
 * @author Thibaut Le Guilly
 */

#ifndef ADAPTER_H
#define ADAPTER_H

#include "hpd_device.h"

#define DEVICE_ID_SIZE 4

/**
 * The structure Adapter containing all the Attributes that an Adapter possesses
 */
typedef struct Adapter Adapter;
typedef struct AdapterElement AdapterElement;

struct Adapter
{
  char *id;
  char *network;
  void *data;
  DeviceElement *device_head;
};

Adapter* 	adapterNew( const char *network, void *data );
void 		adapterFree( Adapter *adapter );
int 		adapterAddDevice(Adapter *adapter, Device *device, char *deviceId);
int 		adapterRemoveDevice(Adapter *adapter, Device *device);
Device* 	findDevice(Adapter *adapter, char *device_id);
mxml_node_t* 	adapterToXml(Adapter *adapter, mxml_node_t *parent);
json_t* 	adapterToJson(Adapter *adapter);
void 		adapterSetId( Adapter *adapter, char *id );

struct AdapterElement
{
  Adapter *adapter;
  AdapterElement *next;
  AdapterElement *prev;
};

AdapterElement*	adapterElementNew(Adapter *adapter);
void 		adapterElementFree(AdapterElement *adapterElement);


#endif
