#include <string>

#include "format.h"

using std::string;
using std::to_string;
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  int just_seconds = seconds % 60;
  seconds /= 60;
  int minutes = seconds % 60;
  int hours = seconds / 60;
  string time_string = to_string(hours) + ":"
    + to_string(minutes) + ":" + to_string(just_seconds);
  return time_string;
}
