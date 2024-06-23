#include <string>
#include <iomanip>

#include "format.h"

using std::string;


// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
  	long s = seconds % 3600 % 60;  
  	long minutes = (seconds % 3600) / 60;
  	long hours = seconds / 3600;
  	std::ostringstream stream;
    stream << std::setw(2) << std::setfill('0') << hours << ":" 
     << std::setw(2) << std::setfill('0') << minutes << ":"
     << std::setw(2) << std::setfill('0') << s;

   return  stream.str();
}