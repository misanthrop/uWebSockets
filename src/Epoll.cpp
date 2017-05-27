#include "Backend.h"

#ifdef USE_EPOLL

// todo: remove this mutex, have callbacks set at program start
std::recursive_mutex cbMutex;
void (*callbacks[16])(Poll *, int, int);
int cbHead = 0;

void Loop::run() {
    timepoint = std::chrono::high_resolution_clock::now();
    while (numPolls) {
        for (std::pair<Poll *, void (*)(Poll *)> c : closing) {
            numPolls--;

            c.second(c.first);

            if (!numPolls) {
                closing.clear();
                return;
            }
        }
        closing.clear();

        int timeout = timers.empty() ? -1 : std::max<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timers[0].timepoint - std::chrono::high_resolution_clock::now()).count(), 0);
        int numFdReady = epoll_wait(epfd, readyEvents, 1024, timeout);

        if (preCb) {
            preCb(preCbData);
        }

        for (int i = 0; i < numFdReady; i++) {
            Poll *poll = (Poll *) readyEvents[i].data.ptr;
            int status = -bool(readyEvents[i].events & EPOLLERR);
            callbacks[poll->state.cbIndex](poll, status, readyEvents[i].events);
        }

        timepoint = std::chrono::high_resolution_clock::now();
        while (timers.size() && timers[0].timepoint <= timepoint) {
            Timer *timer = timers[0].timer;
            cancelledLastTimer = false;
            timers[0].cb(timers[0].timer);

            if (cancelledLastTimer) {
                continue;
            }

            auto tp = timers[0].timepoint;
            int repeat = timers[0].nextDelay;
            auto cb = timers[0].cb;
            timers.erase(timers.begin());
            if (repeat) {
                timer->startAt(cb, tp + std::chrono::milliseconds(repeat), repeat);
            }
        }

        if (postCb) {
            postCb(postCbData);
        }
    }
}
#endif
