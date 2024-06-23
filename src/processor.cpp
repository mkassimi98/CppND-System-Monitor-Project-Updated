#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
	float activeJiff = LinuxParser::ActiveJiffies();
    float iddleJiff = LinuxParser::IdleJiffies();
    float util {0.0f};
  
    if (activeJiff + iddleJiff > 0.0f)
    {
        return activeJiff / (activeJiff + iddleJiff);
    }
    return util;
}