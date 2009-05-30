/*
 *  Copyright 2009 William Tisäter. All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *	1.  Redistributions of source code must retain the above copyright
 *		notice, this list of conditions and the following disclaimer.
 *
 *	2.  Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in the
 *		documentation and/or other materials provided with the distribution.
 *
 *	3.  The name of the copyright holder may not be used to endorse or promote
 *		products derived from this software without specific prior written
 *		permission.
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

#include <fstream>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "isr.h"
#include "stats.h"
#include "socket.h"
#include "utility.h"
#include "argument.h"
#include "socketset.h"
#include "clientset.h"
#include "switchboard.h"

using namespace std;

void Switchboard::parse(SocketSet * _sockets, ClientSet * _clients, Socket * _active_socket, Stats * _stats, ArgumentSet * arguments, const string & _data, const string & _prop_code)
{
	int rid;
	Client client;
	string content;
	int element_code;
	stringstream temp;
	string element_name;
	xmlChar * element_content;
	int element_content_int = 0;
	vector<sys_info> data_history;
	vector<net_info> data_net_history;
	vector<disk_info> data_disk_history;
	
	if (_data.substr(0, 1) != "<")
	{
		if (_data == _prop_code)
		{
			_clients->authenticate(_active_socket->get_id());
			_active_socket->send(isr_accept_code());
		}
		else
			_active_socket->send(isr_reject_code());
	}
	else
	{
		xmlDocPtr doc;
		xmlNodePtr cur;
		xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
		
		doc = xmlCtxtReadMemory(ctxt, _data.c_str(), _data.length(), NULL, NULL, 0);
		
		if ((cur = xmlDocGetRootElement(doc)) != NULL)
		{
			if (xmlStrEqual(cur->name, BAD_CAST "isr"))
			{
				if ((cur = cur->xmlChildrenNode) != NULL)
				{
					element_name = (const char *) cur->name;
					
					// This is a connection test by the client
					// Answer with an empty element as heartbeat
					
					if (element_name == "conntest")
					{
						_active_socket->send(isr_conntest());
					}
					
					// First element contains the host, we have a client connecting
					// Check if this user already is in the client list
					
					if (element_name == "h")
					{
						client.ready = 0;
						client.socket = _active_socket->get_id();
						client.name = (const char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
						cur = cur->next;
						client.duuid = (const char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
					
						// Clean old sessions and update socket id for current duuid
						_clients->update_sessions(client.duuid, _active_socket->get_id(), _sockets);
					
						// Check if this user is already authenticated
						if (_clients->is_authenticated(client.duuid))
						{
							_active_socket->send(isr_accept_connection(0, 6, _stats->get_stats().upt + 1, _stats->get_stats().upt));
						}
						else
						{
							(*_clients) += client;
							_active_socket->send(isr_accept_connection(1, 6, _stats->get_stats().upt + 1, _stats->get_stats().upt));
						}
						
						return;
					}
					
					// Everything below requires a authenticated user
					// Check if we have any - if so - get the client data
					
					if (_clients->length() < 1) return;
					
					// Some connection is requesting data.
					// Check if the user is authenticated and then respond to the question.
					
					if (element_name == "rid")
					{
						if (_clients->is_authenticated(_clients->get_client(_active_socket->get_id()).duuid))
						{
							rid = to_int((const char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
						
							// TODO: Let the client know if there have been changes to disks, temps or fans.
							temp << isr_create_header();
							temp << isr_create_session(rid, 0, 0, 0);
							
							if ((cur = cur->next) == NULL)
							{
								xmlFreeDoc(doc);
								xmlFreeParserCtxt(ctxt);
								return;
							}
						
							while (cur)
							{
								element_name = (const char *) cur->name;
								element_code = to_int(to_ascii(element_name));
								element_content = xmlNodeListGetString(doc->doc, cur->xmlChildrenNode, 1);
								
								if (element_content) element_content_int = to_int(string((const char *) element_content));
								
								switch (element_code)
								{
									case CPU:
										data_history = _stats->get_history(element_content_int);
										temp << isr_cpu_data(&data_history, element_content_int);
										break;
									
									case NETWORK:
										data_net_history = _stats->get_net_history(element_content_int);
										temp << isr_network_data(&data_net_history, element_content_int);
										break;
									
									case MEMORY:
										data_history = _stats->get_history(element_content_int);
										temp << isr_memory_data(&data_history);
										break;
									
									case LOAD:
										data_history = _stats->get_history(element_content_int);
										temp << isr_loadavg_data(&data_history);
										break;
									
									case TEMP:
										// Not implemented yet.
										break;
									
									case FAN:
										// Not implemented yet.
										break;
									
									case UPTIME:
										data_history = _stats->get_history(element_content_int);
										temp << isr_uptime_data(&data_history);
										break;
									
									case DISK:
										data_disk_history = _stats->get_disk_history();
										temp << isr_disk_data(&data_disk_history, element_content_int);
										break;
								}
							
								cur = cur->next;
							}
							
							temp << "</isr>";
					
							_active_socket->send(temp.str());
						}
					}
				} // Failed to read next node
			} // Unknown element recived after header
		} // Failed to read header
	
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
	}
}
