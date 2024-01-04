#include "time_wheel.h"
#include <functional>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>

void printTime(time_t t)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm *tm_info;
  char buffer[26];
  tm_info = localtime(&(tv.tv_sec));
  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  printf("%s.%03ld\n", buffer, tv.tv_usec / 1000);
}
void print_time(int timeout)
{
  std::cout << "****************print_time************" << std::endl;
  std::cout << "timeout " << timeout << std::endl;
  time_t now_time = time(nullptr);
  printTime(now_time);
}
int main()
{
  TimerNode *tm_node1 = new TimerNode(1);
  tm_node1->cb_func_ = std::bind(print_time, tm_node1->timeout_);
  TimerNode *tm_node2 = new TimerNode(2);
  tm_node2->cb_func_ = std::bind(print_time, tm_node2->timeout_);

  TimerNode *tm_node3 = new TimerNode(3);
  tm_node3->cb_func_ = std::bind(print_time, tm_node3->timeout_);

  TimerNode *tm_node4 = new TimerNode(3);
  tm_node4->cb_func_ = std::bind(print_time, tm_node4->timeout_);
  TimerNode *tm_node5 = new TimerNode(3);
  tm_node5->cb_func_ = std::bind(print_time, tm_node5->timeout_);
  TimerNode *tm_node6 = new TimerNode(3);
  tm_node6->cb_func_ = std::bind(print_time, tm_node6->timeout_);

  itimerspec time_value_;
  time_value_.it_value.tv_sec = 0;
  time_value_.it_value.tv_nsec = 1000 * 1000; // 毫秒
                                              // time_value_.it_value.tv_nsec = 1000; //微秒  达不到微妙级别
  time_value_.it_interval.tv_sec = 0;
  time_value_.it_interval.tv_nsec = 0;
  Timer_Loop loop(time_value_);
  loop.AddTimer(tm_node1);
  loop.AddTimer(tm_node2);
  loop.AddTimer(tm_node3);
  loop.AddTimer(tm_node4);
  loop.AddTimer(tm_node5);
  loop.AddTimer(tm_node6);
  loop.show();
  loop.Loop();
  sleep(3);
}