/*
 *  Copyright 2009 William Tisäter. All rights reserved.
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

#ifndef _ISR_H
#define _ISR_H

#include <vector>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "stats.h"
#include "system.h"

std::string isr_create_header();
std::string isr_create_session(int _rid, int _ds, int _ts, int _fs);
std::string isr_accept_code();
std::string isr_reject_code();
std::string isr_conntest();
std::string isr_accept_connection(int _ath, int _ss, int _c, int _n);

std::string isr_cpu_data(std::vector<sys_info> * _history, int _init);
std::string isr_network_data(std::vector<net_info> * _history, int _init);
std::string isr_disk_data(std::vector<disk_info> * _disks, int _init, const std::string cf_disk_mount_path_label, const std::string cf_disk_filesystem_label, std::vector<std::string> cf_disk_rename_label, int _temp_hack);
std::string isr_uptime_data(std::vector<sys_info> * _history);
std::string isr_loadavg_data(std::vector<sys_info> * _history);
std::string isr_memory_data(std::vector<sys_info> * _history);

#endif
