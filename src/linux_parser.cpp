#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;


// A helper function to return the value (as a string) specified by @p label
// in a file at @p os_path.
string LinuxParser::GetValueFromKey(string label, string os_path) {
  string line, key, value;
  std::ifstream stream(os_path);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == label) {
        return value;
      }
    }
  }
}

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  int total_memory, free_memory;
  float memory_utilization;
  string line, key, value;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:")
        total_memory = std::stoi(value);
      else if (key == "MemFree:") {
        free_memory =  std::stoi(value);
        break;  // No need to check more lines
      }
    }
  }
  memory_utilization = (total_memory - free_memory) / (float)total_memory;
  return memory_utilization;
}

// Read and return the system uptime
long int LinuxParser::UpTime() {
  string line;
  long int uptime;
  string os_path = kProcDirectory + kUptimeFilename;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

// Returns array of aggregate jiffy values
long int* LinuxParser::GetCPUJiffies(long int* jiffy_array) {
  string line, label;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> label;
    for (int i=0; i < 10; i++)
      linestream >> jiffy_array[i];
  }
  return jiffy_array;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long int jiffy_array[10];
  GetCPUJiffies(jiffy_array);
  return ActiveJiffies(jiffy_array) + IdleJiffies(jiffy_array);
}

// Read and return the number of active seconds for a PID
// https://man7.org/linux/man-pages/man5/proc.5.html
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
// active time = (utime + stime + cutime + cstime) / clock frequency
long LinuxParser::ActiveSeconds(int pid) {
  string line;
  int start_idx = 13;
  int last_idx = 16;
  long temp_time, active_time;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i=0; i <= last_idx; i++) {
      linestream >> temp_time;
      if (i >= start_idx)
        active_time += temp_time;
    }
  }
  return active_time / sysconf(_SC_CLK_TCK);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long int jiffy_array[10];
  GetCPUJiffies(jiffy_array);
  return ActiveJiffies(jiffy_array);
}

long LinuxParser::ActiveJiffies(long int* jiffy_array) {
  // active = user + nice + system + irq + softirq + steal
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  int active_idx[6] = {0, 1, 2, 5, 6, 7};
  long int active_jiffies = 0;
  for (int idx: active_idx)
    active_jiffies += jiffy_array[idx];
  return active_jiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies(long int* jiffy_array) {
  // idle = idle + iowait
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  long idle_jiffies = 0;
  idle_jiffies += jiffy_array[3];
  idle_jiffies += jiffy_array[4];
  return idle_jiffies;
}

// Read and return CPU utilization
float LinuxParser::CpuUtilization() {
  static long prev_total_jiffies = 0;
  static long prev_active_jiffies = 0;
  long total_jiffies = Jiffies();
  long total_active_jiffies = ActiveJiffies();

  long del_total_jiffies = total_jiffies  - prev_total_jiffies;
  long del_active_jiffies = total_active_jiffies - prev_active_jiffies;
  float cpu_utilization = del_active_jiffies / (float) del_total_jiffies;
  prev_total_jiffies = total_jiffies;
  prev_active_jiffies = total_active_jiffies;
  return cpu_utilization;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string os_path = kProcDirectory + kStatFilename;
  string label = "processes";
  int total_procs = std::stoi(GetValueFromKey(label, os_path));
  return total_procs;
}
// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string os_path = kProcDirectory + kStatFilename;
  string label = "procs_running";
  int running_procs = std::stoi(GetValueFromKey(label, os_path));
  return running_procs;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string key = "VmSize:";
  string os_path = kProcDirectory + std::to_string(pid) + kStatusFilename;
  string ram = GetValueFromKey(key, os_path);
  return ram;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string key = "Uid:";
  string os_path = kProcDirectory + std::to_string(pid) + kStatusFilename;
  string uid = GetValueFromKey(key, os_path);
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line, name;
  char x;
  int id;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line, ':')) {
      std::istringstream linestream(line);
      linestream >> name >> x >> id;
      if (id == std::stoi(uid)) {
        return name;
      }
    }
  }
}

// Read and return the uptime of a process
// Calculated as system uptime - the time after startup a process began.
long LinuxParser::UpTime(int pid) {
  string line;
  int time_idx = 22;
  long temp_time;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i=1; i <= time_idx; i++)
      linestream >> temp_time;
  }
  long timestamp_seconds = temp_time /  sysconf(_SC_CLK_TCK);
  long proc_uptime =  UpTime() - timestamp_seconds;
  return proc_uptime;
}
