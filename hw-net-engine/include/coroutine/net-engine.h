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
    /**
     *  mapping: channel_descriptor -> Channel instance
     */
    std::map<int, Channel> channels;

    /**
     *  map: channel_descriptor -> all coroutines, blocked by I/O operations
     *  with this channel
     */
    std::map<int, std::vector<context*>> locks_table;

    /**
     *  adds ctx to the locks_table and sets ctx->locked to reason
     */
    void lock(int chan_descr, context* ctx, LockedState reason);

    /**
     *  unlocks all coroutines trying to read or write (determined by the 'reason' argument)
     *  to channel chan_descr. Erases them from locks_table
     */
    void unlock_all(int chan_descr, LockedState reason);
public:
    NetEngine();
    ~NetEngine();

    NetEngine(Engine&&) = delete;
    NetEngine(const Engine&) = delete;

    /**
     *  creates channel with unique descriptor and returns descriptor to this channel
     */
    int create_channel();

    /**
     *  creates new channel with desciptor chan_descr or returns existing channel
     *  (with the same descriptor).
     *  Returns: channel descriptor
     */
    int create_channel(int chan_descr);

    /**
     *  closes destroys channel only in case there are no locks associated with
     *  chan_descr. If there are such locks, returns -1 (error), else returns 0 (success)
     */
    int close_channel(int chan_descr);

    /**
     *  blocking send().
     *  Returns: number of bytes written if success or -1 if error
     */
    ssize_t send_channel(int chan_descr, void* data, size_t nbytes);

    /**
     *  blocking recv().
     *  Returns: number of bytes read if success or -1 if error
     */
    ssize_t recv_channel(int chan_descr, void* data, size_t nbytes);
};

} // namespace Coroutine

#endif // COROUTINE_NET_ENGINE_H
