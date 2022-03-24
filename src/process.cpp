#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// Constructor to populate process pid
Process::Process(int pid) : pid_(pid) {}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() {
  static long prev_total_uptime = 0;
  static long prev_active_seconds = 0;
  long total_active_seconds = LinuxParser::ActiveSeconds(pid_);
  long uptime = UpTime();

  long del_total_uptime = uptime  - prev_total_uptime;
  long del_active_seconds = total_active_seconds - prev_active_seconds;
  if ( del_active_seconds > 0)
    cpu_utilization_ = 100 * (del_total_uptime / (float) del_active_seconds);
  else
    cpu_utilization_ = 0;
  prev_total_uptime = uptime_;
  prev_active_seconds = total_active_seconds;
  return cpu_utilization_;
}

// A const getter function for CPU utilization
float Process::GetCpuUtilization() const {
  return cpu_utilization_;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_);}

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// Overload the "less than" comparison operator for Process objects
// Return true if this process' CPU Utilization is < the other's.
bool Process::operator<(Process const& a) const {
  float other_cpu_util = a.GetCpuUtilization();
  if (cpu_utilization_ < other_cpu_util)
    return true;
  return false;
}
