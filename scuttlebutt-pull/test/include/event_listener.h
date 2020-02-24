//
// Created by 陈鹏飞 on 2020/2/23.
//

#ifndef SCUTTLEBUTT_EVENT_LISTENER_H
#define SCUTTLEBUTT_EVENT_LISTENER_H

#include <functional>
#include "duplex.h"

using update_sent_listener = std::function<void(dp::duplex_base *duplex, const nonstd::any &update,
                                                int &sent_counter, std::string id_name)>;
//using update_received_listener = std::function<void(dp::duplex_base * duplex, const nonstd::any & update, int &sent_counter, std::string id_name)>;
using update_received_listener = std::function<void(dp::duplex_base *duplex,
                                                    const nonstd::any &update,
                                                    int &sent_counter, std::string id_name)>;

#endif //SCUTTLEBUTT_EVENT_LISTENER_H
