//
// Created by 陈鹏飞 on 2020/3/6.
//

#ifndef SCUTTLEBUTT_SOCKET_STREAM_H
#define SCUTTLEBUTT_SOCKET_STREAM_H

#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <future>
#include <poll.h>
#include "spdlog/spdlog.h"
#include "duplex-stream/include/duplex-stream.h"
#include "circular-buffer/include/circular-buffer.h"
#include "duplex-pull/include/duplex-pull.h"
#include "event-loop/include/event-loop.h"

class socket_stream final : public duplex_stream {
public:
    socket_stream(size_t high_water_mark)
            : duplex_stream(high_water_mark),
              r_circle_(high_water_mark),
              w_circle_(high_water_mark),
              end_(false),
              waiting_(true),
              draining_(true),
              closed_(false) {

    }

    bool write(const std::string &data) override {
        if (end_) {
            throw "end";
        }

        // abort, not override exist data
        if (w_circle_.full()) {
            draining_ = false;
            return false;
        }

        // todo.optimize
        {
            std::lock_guard<std::mutex> lk(w_mutex_);
            w_circle_.put(data);
            w_cv_.notify_one();
        }

        if (w_circle_.full()) {
            draining_ = false;
            return false;
        }

        return true;
    }

    std::string read() override {
        if (end_) {
            return std::string();
        }

        if (r_circle_.empty()) {
            waiting_ = true;
            return std::string();
        }

        //todo. optimize
        std::lock_guard<std::mutex> lk(r_mutex_);
        auto data = r_circle_.get();
        r_cv_.notify_one();

        return data;
    }

    void end() override {
        _close();
    }

    void _destroy() override {
        _close();
    }

    /**
     * It loops through all the addresses available,
     * regardless of address family.
     * If the destination resolves to an IPv4 address, it will use an PF_INET socket.
     * Similarly, if it resolves to IPv6, an PF_INET6 socket is used.
     *
     * Observe that there is no hardcoded reference to a particular address family.
     * The code works even if getaddrinfo() returns addresses that are not IPv4/v6
     */
    int connect(const std::string &host, const std::string &port) {
        struct addrinfo hints{}, *res, *server_info;
        int error;
        const char *cause = nullptr;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        error = getaddrinfo(host.c_str(), port.c_str(), &hints, &server_info);
        if (error) {
            spdlog::error("getaddrinfo: {}", gai_strerror(error));
            return 1;
        }

        socket_fd_ = -1;
        for (res = server_info; res; res = res->ai_next) {
            socket_fd_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if (socket_fd_ < 0) {
                cause = "socket";
                continue;
            }

            if (::connect(socket_fd_, res->ai_addr, res->ai_addrlen) < 0) {
                cause = "connect";
                close(socket_fd_);
                socket_fd_ = -1;
                continue;
            }

            break;  /* okay we got one */
        }
        if (socket_fd_ < 0) {
            spdlog::error("client: {}", cause);
            return 2;
        }

        char s[INET6_ADDRSTRLEN];
        inet_ntop(res->ai_family, get_in_addr((struct sockaddr *) res->ai_addr), s, sizeof s);
        spdlog::info("client: connecting to {}", s);

        freeaddrinfo(server_info);

        r_thread_ = std::make_unique<std::thread>(&socket_stream::read_thread, this);
        w_thread_ = std::make_unique<std::thread>(&socket_stream::write_thread, this);

        return 0;
    }

private:
    void read_thread() {
        struct pollfd poll_fd{};
        poll_fd.fd = socket_fd_;
        poll_fd.events = POLLIN; // Report ready to read on incoming connection

        while (!end_) {
            if (r_circle_.full()) {
                std::unique_lock<std::mutex> lk(r_mutex_);
                r_cv_.wait(lk, [this] { return !r_circle_.full(); });
            }

            int poll_count = poll(&poll_fd, 1, -1);

            if (poll_count == -1) {
                spdlog::error("poll");
                break;
            }

            // We got one
            if (poll_fd.revents & POLLIN) {
                int bytes = recv(poll_fd.fd, r_buffer_, sizeof r_buffer_, 0);
                if (bytes <= 0) {
                    // Got error or connection closed by client
                    if (bytes == 0) {
                        // Connection closed
                        spdlog::error("socket({}) hung up", poll_fd.fd);
                    } else {
                        spdlog::error("socket recv");
                    }
                    break;
                } else {
                    auto data = std::string(r_buffer_, bytes);
                    r_circle_.put(data);
                    //todo
                    if (waiting_) {
                        waiting_ = false;
                        el::PUSH(el::handler([this](){
                            emit("readable");
                        }));
                    }
                }
            }
        }

        _close();
    }

    void write_thread() {
        while (!end_) {
            if (w_circle_.empty()) {
                el::PUSH(el::handler([this](){
                    if (!draining_ && w_circle_.empty()) {
                        draining_ = true;
                        emit("drain");
                    }
                }));
            }

            {
                std::unique_lock<std::mutex> lk(w_mutex_);
                w_cv_.wait(lk, [this] { return !w_circle_.empty(); });
            }

            auto chunk = w_circle_.get();
            if (send_all(socket_fd_, chunk.c_str(), chunk.length()) == -1) {
                spdlog::error("socket send");
                break;
            }
        }

        _close();
    }

    void _close() {
        if (closed_) {
            return;
        }
        closed_ = true;

        end_ = true;
        if (r_thread_) {
            r_thread_->join();
        }
        if (w_thread_) {
            w_thread_->join();
        }
        close(socket_fd_);
        emit("close");
    }

    int send_all(int s, const char *buf, int len) {
        int total = 0;        // how many bytes we've sent
        int bytes_left = len; // how many we have left to send
        int n = 0;

        while(total < len) {
            n = send(s, buf+total, bytes_left, 0);
            if (n == -1) { break; }
            total += n;
            bytes_left -= n;
        }

        return n == -1? -1 : 0; // return -1 on failure, 0 on success
    }

    // get sockaddr, IPv4 or IPv6:
    static void *get_in_addr(struct sockaddr *sa) {
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in *) sa)->sin_addr);
        }

        return &(((struct sockaddr_in6 *) sa)->sin6_addr);
    }

private:
    int socket_fd_;

    std::atomic<bool> end_;

    static const size_t BUFFER_SIZE = 4 * 1024;
    char r_buffer_[BUFFER_SIZE];
    char w_buffer_[BUFFER_SIZE];

    std::unique_ptr<std::thread> r_thread_ = nullptr;
    circular_buffer<std::string> r_circle_;
    std::atomic<bool> waiting_;
    std::mutex r_mutex_;
    std::condition_variable r_cv_;

    std::unique_ptr<std::thread> w_thread_ = nullptr;
    circular_buffer<std::string> w_circle_;
    std::mutex w_mutex_;
    std::condition_variable w_cv_;
    std::atomic<bool> draining_;


    std::atomic<bool> closed_;
};

#endif //SCUTTLEBUTT_SOCKET_STREAM_H
