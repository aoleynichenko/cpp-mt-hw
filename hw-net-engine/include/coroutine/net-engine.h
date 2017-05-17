#ifndef COROUTINE_NET_ENGINE_H
#define COROUTINE_NET_ENGINE_H

#include <map>
#include <vector>

#include <coroutine/channel.h>
#include <coroutine/engine.h>

namespace Coroutine {

class Channel;

class NetEngine : public Engine {
protected:
    std::map<int, Channel> channels;
    std::map<int, std::vector<context*>> locks_table;

    void lock(int chan_descr, context* ctx, LockedState reason);
    void unlock_all(int chan_descr, LockedState reason);
public:
    NetEngine();
    ~NetEngine();

    NetEngine(Engine&&) = delete;
    NetEngine(const Engine&) = delete;

    int create_channel();
    int create_channel(int chan_descr);
    int close_channel(int chan_descr);

    ssize_t send_channel(int chan_descr, void* data, size_t nbytes);
    ssize_t recv_channel(int chan_descr, void* data, size_t nbytes);
};

} // namespace Coroutine

#endif // COROUTINE_NET_ENGINE_H
