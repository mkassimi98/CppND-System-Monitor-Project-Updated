#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Key-Words
const std::string filterProcesses("processes");
const std::string filterRunningProcesses("procs_running");
const std::string filterUID("Uid:");
const std::string filterProcMem("VmSize:");
const std::string filterOperatingSystem("PRETTY_NAME");

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  string line;
  string name, memory, units;
  float memoryTotal, memoryFree;
  float memoryUtilization = 0;
  if (filestream.is_open()) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> name >> memory >> units;
      memoryTotal = std::stoi(memory);
      std::getline(filestream, line);
      linestream.clear();
      linestream.str(line);
      linestream >> name >> memory >> units;
      memoryFree = std::stoi(memory);

      memoryUtilization = (memoryTotal - memoryFree) / memoryTotal;

      filestream.close();
  }

  return memoryUtilization;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
	string line;
    string key;
    string time {""};
    std::ifstream filestream(kProcDirectory + kUptimeFilename);
    if (filestream.is_open()) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> time;

      filestream.close();
    }
    return std::stol(time);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return  ActiveJiffies() + IdleJiffies(); }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  long int utime, stime, cutime, cstime;
  long activeJiffies {-1};
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(filestream.is_open())
  {
    string line, useless_token;
    std::getline(filestream, line);
    std::istringstream linestream(line);

    for(int i = 0; i < 13; ++i)
    {
      linestream >> useless_token;
    }
    linestream >> utime >> stime >> cutime >> cstime;
    activeJiffies = (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
    filestream.close();
  }

  return activeJiffies; 
}
// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
    auto jiffies = CpuUtilization();

    return std::stol(jiffies[CPUStates::kUser_]) +
           std::stol(jiffies[CPUStates::kNice_]) +
           std::stol(jiffies[CPUStates::kSystem_]) +
           std::stol(jiffies[CPUStates::kIRQ_]) +
           std::stol(jiffies[CPUStates::kSoftIRQ_]) +
           std::stol(jiffies[CPUStates::kSteal_]);
}

string findValueInFileByKey(string path, string formatedKey) {
  string line;
  string key;
  string value;
  std::ifstream filestream(path);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == formatedKey) {
          return value;
        }
      }
    }
    filestream.close();
  }
  return "";
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
    auto jiffies = CpuUtilization();

    return std::stol(jiffies[CPUStates::kIdle_]) +
           std::stol(jiffies[CPUStates::kIOwait_]);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
	string line, cpu, value;
    vector<string> jiffies;
    std::ifstream stream(kProcDirectory + kStatFilename);

    if (stream.is_open()) {
      std::getline(stream, line);
      std::istringstream linestream(line);

      linestream >> cpu;

      while (linestream >> value) {
        jiffies.push_back(value);
      }
      stream.close();
    }
    return jiffies;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
    string value = findValueInFileByKey(kProcDirectory + kStatFilename, filterProcesses);
    if (!value.empty()) {
      return std::stoi(value);
    } else {
      return 0;
    }
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
    string value =
        findValueInFileByKey(kProcDirectory + kStatFilename, filterRunningProcesses);
    if (!value.empty()) {
      return std::stoi(value);
    } else {
      return 0;
    }
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
    string command {""};
    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
    if(filestream.is_open())
    {
      string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> command;

      filestream.close();
    }

    return command;  
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
    string vmSize = findValueInFileByKey(kProcDirectory + std::to_string(pid) + kStatusFilename, filterProcMem);
    int ram = 0;
    if(!vmSize.empty())
    {
      ram = std::stof(vmSize)/ 1024;
    }


    return std::to_string(ram);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
    string uid = findValueInFileByKey(kProcDirectory + std::to_string(pid) + kStatusFilename, filterUID);
    return uid;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
    string user_uid = Uid(pid);
    string name {"Unkonwn"};
    if(user_uid.empty())
    {
      return name;
    }

    std::ifstream filestream(kPasswordPath);

    if (filestream.is_open()) {
      string line, pass, uid;
      while (std::getline(filestream, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        while (linestream >> name >> pass >> uid) {
          if (uid == user_uid) {
            return name;
          }
        }
      }
      filestream.close();
    }

    return name;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
    long int uptime {0};
    std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
    if(filestream.is_open())
    {
      string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string field;
      for (int i = 1; i <= 22; ++i) {
        linestream >> field;
      }

      uptime = std::stol(field) / sysconf(_SC_CLK_TCK);

      filestream.close();
    }

    return uptime; 
}
