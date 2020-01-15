/*
 * This file is part of SwabianCoin.
 *
 * SwabianCoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * SwabianCoin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SwabianCoin.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "SystemMonitor.h"
#ifdef _WIN32
#else
#include "sys/types.h"
#include "sys/sysinfo.h"
#endif

using namespace scn;

SystemMonitor::SystemMonitor()
: running_(true)
, system_monitor_thread_(&SystemMonitor::systemMonitorThread, this) {

}


SystemMonitor::~SystemMonitor() {
    running_ = false;
    system_monitor_thread_.join();
}


void SystemMonitor::systemMonitorThread() {
    while(running_) {

        uint64_t total_ram = 0, total_swap = 0, free_ram = 0, free_swap = 0;
#ifdef _WIN32
        //todo: find something platform independent or implement for windows
#else
        //todo: find better library
        struct sysinfo mem_info;
        sysinfo(&mem_info);
        total_ram = mem_info.totalram;
        total_swap = mem_info.totalswap;
        free_ram = mem_info.freeram;
        free_swap = mem_info.freeswap;
#endif

        LOG(INFO) << "Memory Overview -  Total RAM: " << total_ram
                  << " Total Swap: " << total_swap
                  << " Free RAM: " << free_ram << " (" << 100.0 * static_cast<double>(free_ram)/static_cast<double>(total_ram) << "%)"
                  << " Free Swap: " << free_swap << " (" << 100.0 * static_cast<double>(free_swap)/static_cast<double>(total_swap) << "%)";

        google::FlushLogFiles(google::GLOG_INFO);

        for(uint32_t i=0;i<60*2;i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if(!running_) {
                break;
            }
        }
    }
}