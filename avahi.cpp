/*
 *  Copyright 2010 William Tisäter. All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *    1.  Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *    2.  Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *    3.  The name of the copyright holder may not be used to endorse or promote
 *        products derived from this software without specific prior written
 *        permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL WILLIAM TISÄTER BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_LIBAVAHI_CLIENT
# include <iostream>
# include <fstream>
# include <pthread.h>
# include <stdio.h>
# include <avahi-common/error.h>
# include <avahi-common/malloc.h>
# include <avahi-client/client.h>
# include <avahi-client/publish.h>
# include <avahi-common/alternative.h>
# include <avahi-common/simple-watch.h>

# ifdef HAVE_LIBUUID
#  include <uuid/uuid.h>
# endif

# include "avahi.h"

using namespace std;
static pthread_t avahi_thread;
static AvahiSimplePoll *avahi_poll = NULL;
static AvahiEntryGroup *group = NULL;
static char type[] = "_istatserv._tcp";
static char name[100];
static int port;

void get_uuid(char* uuid_str) {

#ifdef HAVE_LIBUUID
	uuid_t uuid;

	uuid_generate(uuid);
	uuid_unparse_upper(uuid,uuid_str);
#else
	ifstream kuuid("/proc/sys/kernel/random/uuid");
	if (kuuid.is_open()) {
		kuuid.getline(uuid_str,37);
	}
#endif

}

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {

	assert(g == group || group == NULL);
	group = g;

	switch (state) {
        	case AVAHI_ENTRY_GROUP_FAILURE :
		case AVAHI_ENTRY_GROUP_COLLISION:
			cerr << "Avahi group failure" << endl;
			break;
		default:
			break;
	}
}

static void create_service(AvahiClient *c) {

	assert(c);

	int ret;

	if (!group) {
		if (!(group = avahi_entry_group_new(c, entry_group_callback, NULL))) {
			cerr << "avahi_entry_group_new() failed: " << avahi_strerror(avahi_client_errno(c)) << endl;
		}
	}

	if (avahi_entry_group_is_empty(group)) {

		char uuid_str[37];
		char hostname[62];

		get_uuid(uuid_str);
		gethostname(hostname,sizeof(hostname));
		sprintf(name,"%s,%s",uuid_str,hostname);

		if ((ret = avahi_entry_group_add_service(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, name, type, NULL, NULL, port, NULL)) < 0) {
			cerr << "Failed to add " << name << "service: " << avahi_strerror(ret) << endl;
		}

		if ((ret = avahi_entry_group_commit(group)) < 0) {
            		cerr <<  "Failed to commit entry group: " << avahi_strerror(ret) << endl;
            	}

	}

}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {

	assert(c);

	switch (state) {
	case AVAHI_CLIENT_S_RUNNING:
		create_service(c);
        	break;
	default:
		break;
	}
}

void *create_new_poll(void *param) {

        AvahiClient *client = NULL;
        int error;
	void *ret = (void *)1;

        if (!(avahi_poll = avahi_simple_poll_new())) {
                cerr << "Failed to create avahi poll object." << endl;
		return ret;
        }

        client = avahi_client_new(avahi_simple_poll_get(avahi_poll), (AvahiClientFlags)0, client_callback, NULL, &error);

        if (!client) {
                cerr << "Failed to create client: " << avahi_strerror(error) << endl;
		return ret;
        }

        avahi_simple_poll_loop(avahi_poll);

	if (client) {
		avahi_client_free(client);
	}

	if (avahi_poll) {
        	avahi_simple_poll_free(avahi_poll);
	}

	ret = (void *)0;

	return ret;
}

void avahi_publish_service(int announce_port) {

	port = announce_port;

	int ret = pthread_create(&avahi_thread,NULL,create_new_poll, (void *)NULL);

	if (ret) {
		cerr << "Unable to create avahi thread" << endl;
	}

}

void avahi_stop() {
	if (avahi_poll) {
		avahi_simple_poll_quit(avahi_poll);
	}
}

#endif
