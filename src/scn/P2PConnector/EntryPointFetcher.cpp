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

#include "EntryPointFetcher.h"
#include <curl/curl.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace scn;


const std::string EntryPointFetcher::entry_points_url_ = "https://www.swabiancoin.com/scn_entry_point_list.txt";

const std::list<std::pair<std::string, std::uint16_t>> EntryPointFetcher::entry_points_fallback_{{"56bd5cf.online-server.cloud", 13286},
                                                                                                 {"f11b639.online-server.cloud", 13288},
                                                                                                 {"e2f7bf6.online-server.cloud", 13290},
                                                                                                 {"df564a5.online-server.cloud", 13292},
                                                                                                 {"v33200.1blu.de", 13294},
                                                                                                 {"v36919.1blu.de", 13296},
                                                                                                 {"v41253.1blu.de", 13298},
                                                                                                 {"v26254.1blu.de", 13300}};


static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


const std::list<std::pair<std::string, std::uint16_t>> EntryPointFetcher::fetch() {
    std::list<std::pair<std::string, std::uint16_t>> entry_points = entry_points_fallback_;

    LOG(INFO) << "Fetching entry point list";

    std::string read_buffer;
    CURL *curl = curl_easy_init();
    try {
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, entry_points_url_.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res == CURLE_OK) {
                LOG(INFO) << "Received entry point list content: " << read_buffer;
                std::vector<std::string> lines;
                boost::split(lines, read_buffer, boost::is_any_of("\n"));
                for (auto &line : lines) {
                    std::vector<std::string> items;
                    boost::split(items, line, boost::is_any_of(":"));
                    if (items.size() == 2) {
                        entry_points.push_front(std::pair<std::string, std::uint16_t>(items[0], std::stoi(items[1])));
                    }
                }
            } else {
                LOG(ERROR) << "Failed to fetch entry point list, using fallback entry point list. CURL return value: "
                           << res << " CURL error description: " << curl_easy_strerror(res);
            }
        } else {
            LOG(ERROR) << "Failed to initialize curl - using fallback entry point list";
        }
    } catch(const std::exception& e) {
        LOG(ERROR) << "Exception occured when fetching entry point list - using fallback entry point list. Exception: " << e.what();
    }

    return entry_points;
}