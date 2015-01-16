/*
 * Copyright (c) 2008 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <stdio.h>
#include "sigar.h"

int main(int argc, char **argv) {
    int status;
    unsigned long i;
    sigar_t *sigar;
    sigar_sys_info_t systeminfo;
    //sigar_resource_limit_t resinfo;
    sigar_cpu_info_list_t cpulinfolist;
    sigar_mem_t meminfo;
    sigar_file_system_list_t diskinfolist;
    sigar_file_system_usage_t diskusageinfo;

    sigar_open(&sigar);

    //Operating System Info
    status = sigar_sys_info_get(sigar, &systeminfo);
    if (status != SIGAR_OK) {
        printf("sys_info error: %d (%s)\n",
            status, sigar_strerror(sigar, status));
        return 1;
    }
    printf("OS Info: %s\n", systeminfo.description);
    printf("Program Arch: %s\n", systeminfo.arch);

    //Resource Info
    /*status = sigar_resource_limit_get(sigar, &resinfo);
    if (status != SIGAR_OK) {
        printf("resource_limit error: %d (%s)\n",
            status, sigar_strerror(sigar, status));
        return 1;
    }
    printf("%i %i\n", resinfo.memory_cur, resinfo.memory_max);*/

    //CPU Info
    status = sigar_cpu_info_list_get(sigar, &cpulinfolist);
    if (status != SIGAR_OK) {
        printf("cpu_list error: %d (%s)\n",
            status, sigar_strerror(sigar, status));
        return 1;
    }
    printf("CPU Info\n");
    for (i=0; i<cpulinfolist.number; i++) {
        sigar_cpu_info_t cpu_info = cpulinfolist.data[i];

        printf(" %s %s %i Mhz\n", cpu_info.vendor, cpu_info.model, cpu_info.mhz);
    }
    sigar_cpu_info_list_destroy(sigar, &cpulinfolist);

    //Memory Info
    status = sigar_mem_get(sigar, &meminfo);
    if (status != SIGAR_OK) {
        printf("mem_info error: %d (%s)\n",
            status, sigar_strerror(sigar, status));
        return 1;
    }
#ifdef WIN32
    printf("Memory Info\n RAM: %I64d MB\n Used: %I64d MB %.2f%%\n Free: %I64d MB %.2f%%\n", meminfo.ram, meminfo.used/1000000, meminfo.used_percent, meminfo.free/1000000, meminfo.free_percent);
#else
    printf("Memory Info\n RAM: %lu MB\n Used: %lu MB %.2f%%\n Free: %lu MB %.2f%%\n", meminfo.ram, meminfo.used/1000000, meminfo.used_percent, meminfo.free/1000000, meminfo.free_percent);
#endif
    //Disk Info
    status = sigar_file_system_list_get(sigar, &diskinfolist);
    if (status != SIGAR_OK) {
        printf("file_system_list error: %d (%s)\n",
            status, sigar_strerror(sigar, status));
        return 1;
    }
    printf("Disk Info\n");
    for (i=0; i<diskinfolist.number; i++) {
        sigar_file_system_t disk_info = diskinfolist.data[i];

        status = sigar_file_system_usage_get(sigar, disk_info.dir_name, &diskusageinfo);

        if (status != SIGAR_OK) {
            /*printf("disk_usage error: %d (%s)\n",
                status, sigar_strerror(sigar, status));
            return 1;*/
        }
        else if(disk_info.dev_name[0] != '\\'){
#ifdef WIN32
            printf("%s Total: %I64d MB Used: %I64d MB Free: %I64d MB\n", disk_info.dev_name, diskusageinfo.total/1000, diskusageinfo.used/1000, diskusageinfo.free/1000);
#else
            printf("%s Total: %lu MB Used: %lu MB Free: %lu MB\n", disk_info.dev_name, diskusageinfo.total/1000, diskusageinfo.used/1000, diskusageinfo.free/1000);
#endif
        }
    }
    sigar_file_system_list_destroy(sigar, &diskinfolist);

    sigar_close(sigar);

    return 0;
}
