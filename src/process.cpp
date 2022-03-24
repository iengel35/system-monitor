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
Process::Process(int pid) : pid_(pid) {
  cpu_utilization_ = CpuUtilization();
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() {
  long total_active_seconds = LinuxParser::ActiveSeconds(pid_);
  cpu_utilization_ =  (float) total_active_seconds /  UpTime();
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
  if (cpu_utilization_ > a.GetCpuUtilization())
    return true;
  return false;
}
