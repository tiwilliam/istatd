/*
 *  Copyright 2009 William Tisäter. All rights reserved.
 *  Copyright 2009 Mo McRoberts.
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

#include <stdio.h>
#include <string.h>

#ifdef HAVE_MNTENT_H
# include <mntent.h>
#endif

#ifdef HAVE_PATHS_H
# include <paths.h>
#endif

#ifdef HAVE_SYS_MNTTAB_H
# include <sys/mnttab.h>
#elif defined(HAVE_MNTENT_H)
# include <mntent.h>
#endif

#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#elif defined(HAVE_SYS_STATFS_H)
# include <sys/statfs.h>
#endif

#include "system.h"

#ifndef _PATH_MOUNTED
# define _PATH_MOUNTED                 "/etc/mtab"
#endif

#ifdef USE_DISK_STATFS
int get_disk_info(const char * _dev, struct disk_info * _disk)
{
    FILE * table;
    unsigned long long bsize;
# ifdef HAVE_STATVFS
    struct statvfs space;
# else
    struct statfs space;
# endif
    int get_size = 0;
# ifdef USE_STRUCT_MNTENT
    struct mntent *entry;
# elif defined(USE_STRUCT_MNTTAB)
    struct mnttab *entry, ebuf;
# endif
    
# ifdef HAVE_SETMNTENT
    if (!(table = setmntent(_PATH_MOUNTED, "r"))) return -1;
# else
    if (!(table = fopen(_PATH_MOUNTED, "r"))) return -1;
    resetmnttab(table);
# endif
    
# ifdef USE_STRUCT_MNTENT
    while ((entry = getmntent(table)) != 0)
# elif defined(USE_STRUCT_MNTTAB)
    entry = &ebuf;
    while (!getmntent(table, entry))
#  define mnt_fsname mnt_special
#  define mnt_dir mnt_mountp 
# endif
    {
        if (strcmp(entry->mnt_fsname, _dev) == 0 || strcmp(entry->mnt_dir, _dev) == 0)
        {
            get_size = 1;
            
            _disk->name = entry->mnt_dir;
            _disk->device = entry->mnt_fsname;
            
            break;
        }
    }

# ifdef HAVE_SETMNTENT
    endmntent(table);
# else
    fclose(table);
# endif

    if (get_size)
    {
# ifdef HAVE_STATVFS
        if (statvfs(_disk->name, &space) == 0)
# else
        if (statfs(_disk->name, &space) == 0)
# endif
        {
# ifdef HAVE_STATVFS_FRSIZE
            bsize = space.f_frsize;
# else
            bsize = space.f_bsize;
# endif
            _disk->t = ((unsigned long long) space.f_blocks * bsize) / 1024;
            _disk->u = ((unsigned long long) (space.f_blocks - space.f_bavail) * bsize) / 1024;
            _disk->f = _disk->t - _disk->u;
            _disk->p = ((float) _disk->u / _disk->t) * 100;
            
            return 0;
        }
    }
    
    return -1;
}
#endif /*USE_DISK_STATFS*/
