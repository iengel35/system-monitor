#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid();                               // TODO: See src/process.cpp
  std::string User();                      // TODO: See src/process.cpp
  std::string Command();
  float CpuUtilization();
  float GetCpuUtilization() const;
  std::string Ram();
  long int UpTime();
  bool operator<(Process const& a) const;

 private:
  int pid_;
  std::string user_, command_, ram_;
  float cpu_utilization_;
  long int uptime_, active_time_;
};

#endif
