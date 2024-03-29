#ifndef __TIME_WHEEL__
#define __TIME_WHEEL__
#include <cstring>
#include <functional>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <sys/epoll.h>



using Func = std::function<void()>;
class TimerNode {
public:
  TimerNode(int timeout)
      : cb_func_(nullptr), rotation_(0), time_slot_(0), next_(nullptr),
        pre_(nullptr), timeout_(timeout) {}
  TimerNode(const TimerNode &) = delete;
  TimerNode &operator=(const TimerNode &) = delete;

public:
  Func cb_func_;
  int rotation_;
  int time_slot_;
  TimerNode *next_;
  TimerNode *pre_;
  int timeout_; // s
};

class TimeWheel {
public:
  TimeWheel() : cur_slot(0) {
    for (int i = 0; i < kN; i++) {
      slot[i] = nullptr;
    }
  }
  void AddTimer(TimerNode *current_timer) {
    if (current_timer->timeout_ < 0) {
      return;
    }
    int tick = 0;

    if (current_timer->timeout_ < kSi) {
      tick = 0;
    } else {
      tick = (current_timer->timeout_ / kSi);
    }

    int time_slot = (cur_slot + tick) % kN;
    int rotation = tick / kN;
    current_timer->rotation_ = rotation;
    current_timer->time_slot_ = time_slot;
    if (slot[time_slot] == NULL) {
      slot[time_slot] = current_timer;

    } else {
      current_timer->next_ = slot[time_slot];
      slot[time_slot]->pre_ = current_timer;
      slot[time_slot] = current_timer;
    }
  }
  void DelTimer(TimerNode *current_timer) {
    DelTimerFromList(current_timer);
    delete(current_timer);
  }
  void ModTimer(TimerNode *current_timer, int new_timeout) {

    DelTimerFromList(current_timer);
    current_timer->timeout_ = new_timeout;
    AddTimer(current_timer);
  }
  void Tick() {
    TimerNode *current_timer = slot[cur_slot];
    TimerNode * next_timer = nullptr;
    while (current_timer) {
        next_timer = current_timer->next_;
      if (!current_timer->rotation_) {
        current_timer->cb_func_();
        DelTimer(current_timer);
      } else {
        current_timer->rotation_--;
      }
      current_timer = next_timer;
    }
    cur_slot = ++cur_slot % kN;
  }
  void Show() {
    for (int i = 0; i < kN; i++) {
      std::cout << "第" << i << "个时间槽" << std::endl;
      TimerNode *tmp = slot[i];
      while (tmp) {
        std::cout << "tmp time_slot " << tmp->time_slot_ << std::endl;
        std::cout << "tmp rotation_ " << tmp->rotation_ << std::endl;
        std::cout << "tmp timeout " << tmp->timeout_ << std::endl;
        tmp = tmp->next_;
      }
    }
  }

private:
  void DelTimerFromList(TimerNode *current_timer) {
    int time_slot = current_timer->time_slot_;
    int time_out = current_timer->timeout_;
    std::cout << "DelTimerFromList"
              << "time_slot" << time_slot << std::endl;
    std::cout << "DelTimerFromList"
              << "time_out" << time_out << std::endl;
    if (slot[time_slot] == current_timer) {
      std::cout << "slot[time_slot] == current_timer" << std::endl;
      slot[time_slot] = current_timer->next_;
      if (slot[time_slot])
        slot[time_slot]->pre_ = nullptr;
    } else {
      std::cout << "slot[time_slot] != current_timer" << std::endl;
      current_timer->pre_->next_ = current_timer->next_;
      if (current_timer->next_)
        current_timer->next_->pre_ = current_timer->pre_;
    }
    current_timer->pre_ = nullptr;
    current_timer->next_ = nullptr;
  }

public:
  //时间轮槽的个数
  static const int kN = 20;
  //每个槽的时间区间1s
  static const int kSi = 1;
  //时间轮的槽， 其中每个元素指向一个定时器链表， 链表无序
  TimerNode *slot[kN];
  int cur_slot;
};

class Timer_Loop{
  public:
   Timer_Loop(itimerspec time);
  void AddTimer(TimerNode *current_timer);
  void DeleteTimer();
    void Loop();
    void show();
  private:
  using EventVec = std::vector<struct epoll_event>;
  static const int kInitEventListSize = 16;
  const int MAX_EVENT  =5;
  EventVec events_;
  private:
  bool quit_;
  TimeWheel timer_wheel_;
  int epoll_fd_;
  int time_fd_;
  itimerspec time_value_;
 
};
#endif