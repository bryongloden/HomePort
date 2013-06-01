// instance.c

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

#include "instance.h"
#include "libREST.h"
#include "trie.h"

#include <stdlib.h>
#include <stdio.h>

struct lr {
   struct lr_service *services;
   struct TrieNode *trie;
};

struct lr_service {
   lr_cb on_get;
   lr_cb on_post;
   lr_cb on_put;
   lr_cb on_delete;
   struct lr_service *next;
};

struct lr *lr_create()
{
   struct lr *ins = malloc(sizeof(struct lr));
   if (ins == NULL) {
      fprintf(stderr, "Cannot allocate lr instance\n");
      return NULL;
   }

   ins->trie = trie_create();

   ins->services = NULL;

   return ins;
}

void lr_destroy(struct lr *ins)
{
   struct lr_service *next;
   struct lr_service *service = ins->services;

   while (service != NULL) {
      next = service->next;
      free(service);
      service = next;
   }

   trie_destroy(ins->trie);
   free(ins);
}

void lr_register_service(struct lr *ins,
                         char *url,
                         lr_cb on_get,
                         lr_cb on_post,
                         lr_cb on_put,
                         lr_cb on_delete)
{
   struct lr_service *service = ins->services;

   if (service == NULL) {
      ins->services = malloc(sizeof(struct lr_service));
      service = ins->services;
   } else {
      while (service->next != NULL) service = service->next;
      service->next = malloc(sizeof(struct lr_service));
      service = service->next;
   }
   
   if (service == NULL) {
      fprintf(stderr, "Not enough memory to allocate service\n");
      return;
   }

   service->on_get = on_get;
   service->on_post = on_post;
   service->on_put = on_put;
   service->on_delete = on_delete;
   service->next = NULL;

   struct ListElement* element = trie_insert(ins->trie, url);
   set_listElement_value(element, service);
}

// TODO: Make lookup_service private and make a function which actually calls the callback
//       int lr_call(struct lr *ins, url, method)?
lr_cb lr_lookup_service(struct lr *ins, char *url, enum lr_method method)
{
  struct ListElement* element = trie_lookup_node(ins->trie, url);
  if(element != NULL) {
    struct lr_service *service = get_ListElement_value(element);
    if(service != NULL) {
      switch(method) {
        case GET:
          return service->on_get;
        break;
        case POST:
          return service->on_post;
        break;
        case PUT:
          return service->on_put;
        break;
        case DELETE:
          return service->on_delete;
        break;
      }
    }
  }
  return NULL;
}