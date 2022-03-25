#include <string>
#include <iomanip>

#include "format.h"

using std::string;
using std::to_string;
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
using std::setw;
using std::setfill;
string Format::ElapsedTime(long seconds) {
  int just_seconds = seconds % 60;
  seconds /= 60;
  int minutes = seconds % 60;
  int hours = seconds / 60;
  std::stringstream ss;
  ss << setw(2) << setfill('0') << hours << ':' << setw(2) << setfill('0') << minutes
     << ':' << setw(2) << setfill('0') << just_seconds;
  string time_string = ss.str();
  return time_string;
}
