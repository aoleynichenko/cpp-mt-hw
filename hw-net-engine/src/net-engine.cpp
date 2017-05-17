#include <coroutine/channel.h>
#include <coroutine/net-engine.h>

#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include <map>
#include <vector>

namespace Coroutine {

NetEngine::NetEngine()
{
}

NetEngine::~NetEngine()
{
}

/**
 *  creates channel with unique descriptor and returns descriptor to this channel
 */
int NetEngine::create_channel()
{
    if (channels.empty()) {
        channels[0] = Channel();
        return 0;
    }

    // search for the smallest possible unique number
    // for the new channel descriptor 'chd'

    int chd = 0;
    for(std::map<int,Channel>::iterator it = channels.begin(); it != channels.end(); it++) {
        if (it->first != chd) {
            break;
        }
        chd++;
    }

    channels[chd] = Channel();

    return chd;
}

/**
 *  creates new channel with desciptor chan_descr or returns existing channel
 *  (with the same descriptor).
 *  Returns: channel descriptor
 */
int NetEngine::create_channel(int chan_descr)
{
    std::map<int,Channel>::iterator ch_it = channels.find(chan_descr);

    if (ch_it == channels.end()) {
        channels[chan_descr] = Channel();
    }

    return chan_descr;
}

/**
 *  closes destroys channel only in case there are no locks associated with
 *  chan_descr. If there are such locks, returns -1 (error), else returns 0 (success)
 */
int NetEngine::close_channel(int chan_descr)
{
    std::map<int,std::vector<context*>>::iterator chan_locks = locks_table.find(chan_descr);

    // there are locks associated with this channel
    // we shold not destroy it
    if (chan_locks != locks_table.end() && !chan_locks->second.empty()) {
        return -1;
    }

    channels.erase(chan_descr);
    return 0;
}

/**
 *  blocking send().
 *  Returns: number of bytes written if success or -1 if error
 */
ssize_t NetEngine::send_channel(int chan_descr, void* data, size_t nbytes)
{
    std::map<int,Channel>::iterator ch_it = channels.find(chan_descr);

    // no channel with descriptor chan_descr
    if (ch_it == channels.end()) {
        errno = EBADF;
        return -1;
    }

    int chd = ch_it->first;
    Channel& channel = ch_it->second;

    size_t written = 0;
    while (written != nbytes) {
        int nwrite = channel.write((char*) data + written, nbytes - written);
        written += nwrite;
        // unlock routines which wants to read from this channel
        unlock_all(chan_descr, LockedState::LOCKED_READ);

        if (written != nbytes) {
            lock(chan_descr, cur_routine, LockedState::LOCKED_WRITE);
            yield();
        }
    }

    return written;
}

/**
 *  blocking recv().
 *  Returns: number of bytes read if success or -1 if error
 */
ssize_t NetEngine::recv_channel(int chan_descr, void* data, size_t nbytes)
{
    std::map<int,Channel>::iterator ch_it = channels.find(chan_descr);

    // no channel with descriptor chan_descr
    if (ch_it == channels.end()) {
        errno = EBADF;
        return -1;
    }

    int chd = ch_it->first;
    Channel& channel = ch_it->second;

    size_t n_bytes_read = 0;
    while (n_bytes_read != nbytes) {
        int nread = channel.read((char*) data + n_bytes_read, nbytes - n_bytes_read);
        n_bytes_read += nread;
        // unlock routines which wants to write to this channel
        unlock_all(chan_descr, LockedState::LOCKED_WRITE);

        if (n_bytes_read != nbytes) {
            lock(chan_descr, cur_routine, LockedState::LOCKED_READ);
            yield();
        }
    }

    return n_bytes_read;
}

// protected:

/**
 *  adds ctx to the locks_table and sets ctx->locked to reason
 */
void NetEngine::lock(int chan_descr, context* ctx, LockedState reason)
{
    if (locks_table.find(chan_descr) == locks_table.end()) {
        locks_table[chan_descr] = std::vector<context*>();
    }

    locks_table[chan_descr].push_back(ctx);
    ctx->locked = reason;
}

/**
 *  unlocks all coroutines trying to read or write (determined by the 'reason' argument)
 *  to channel chan_descr. Erases them from locks_table
 */
void NetEngine::unlock_all(int chan_descr, LockedState reason)
{
    if (locks_table.find(chan_descr) != locks_table.end()) {
        std::vector<context*>& locked_contexts = locks_table[chan_descr];
        for (std::vector<context*>::iterator i = locked_contexts.begin(); i != locked_contexts.end(); i++) {
            if ((*i)->locked == reason) {
                (*i)->locked = LockedState::NOT_LOCKED;
                locked_contexts.erase(i);
            }
        }
    }
}

} // namespace Coroutine
