//
// Created by 陈鹏飞 on 2020/2/23.
//

#ifndef SCUTTLEBUTT_EVENT_LISTENER_H
#define SCUTTLEBUTT_EVENT_LISTENER_H

#include <functional>
#include "duplex.h"

using update_sent_listener = std::function<void(dp::duplex_pull *duplex, const nonstd::any &update,
                                                int &sent_counter, std::string id_name)>;

using update_received_listener = std::function<void(dp::duplex_pull *duplex,
                                                    const nonstd::any &update,
                                                    int &sent_counter, std::string id_name)>;

using unstream_listener = std::function<void(int &count)>;

using changed_with_kv_listener = std::function<void(std::string &key, nonstd::any &value)>;
using changed_with_v_listener = std::function<void(nonstd::any &value)>;

using changed_by_peer_listener = std::function<void(std::string &key, nonstd::any &value,
                                                    const sb::from &from)>;

#endif //SCUTTLEBUTT_EVENT_LISTENER_H
