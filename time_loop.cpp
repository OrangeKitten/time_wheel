#include "time_wheel.h"
#include <thread>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
const int kMaxEvents = 100;
Timer_Loop::Timer_Loop(itimerspec time) : time_value_(time), quit_(false), events_(kInitEventListSize)
{

    // 创建定时器文件描述符
    time_fd_ = timerfd_create(CLOCK_REALTIME, 0);
    if (time_fd_ == -1)
    {
        throw std::runtime_error("timerfd_create failed");
    }
    // 创建 epoll 实例
    epoll_fd_ = epoll_create(kMaxEvents);
    if (epoll_fd_ == -1)
    {
        throw std::runtime_error("epoll_create1 failed");
    }
    // 将定时器文件描述符添加到 epoll 监听事件
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = time_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, time_fd_, &event) == -1)
    {
        throw std::runtime_error("epoll_ctl failed");
    }
}

struct epoll_event events[5];
void Timer_Loop::Loop()
{
    while (!quit_)
    {
        int num_events = epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), -1);
        if (num_events == -1)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < num_events; ++i)
        {
            if (events_[i].data.fd == time_fd_)
            {
                uint64_t expirations;
                read(time_fd_, &expirations, sizeof(expirations));
                timer_wheel_.Tick();
                timerfd_settime(time_fd_, 0, &time_value_, nullptr);
            }
        }
    }
}

void Timer_Loop::AddTimer(TimerNode *current_timer)
{
    // 设置定时器
    timer_wheel_.AddTimer(current_timer);
    std::cout << "timeout" << current_timer->timeout_ << std::endl;
    timerfd_settime(time_fd_, 0, &time_value_, nullptr);
}
void Timer_Loop::DeleteTimer() {}
void Timer_Loop::show()
{
    timer_wheel_.Show();
}
