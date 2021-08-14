#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

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

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    if (file->d_type == DT_DIR) {
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

float LinuxParser::MemoryUtilization() { 
  float mem_total, mem_free;
  string key, value, kb, line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)){
      	std::replace(line.begin(), line.end(), ':', ' ');
  		std::istringstream linestream(line);
    	while (linestream >> key >> value >> kb) {
        	if (key == "MemTotal") { mem_total = std::stof(value);}
            if (key == "MemFree") { mem_free = std::stof(value);}
        	}
    	}
  }
  return (mem_total - mem_free)/mem_total;
}

long LinuxParser::UpTime() { 
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
   	std::istringstream linestream(line);
    linestream >> uptime;
  }
  return std::stol(uptime);
}

vector<string> LinuxParser::CpuUtilization() { 
  string line;
  string key;
  int idle;
  int active;
  float util;
  vector<string> jiffies{};
  vector<int> params;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> key;
    while (linestream >> key) {
      params.emplace_back(std::stoi(key));
      }
    idle = params[3] + params[4];
    active = params[0] + params[1] + params[2] + params[4] + params[6] + params[7];
    util = active/(float)(idle + active);
    jiffies.emplace_back(std::to_string(util));
  }
  return jiffies;
}
float LinuxParser::CpuUtilization(int pid) {
  string line;
  vector<string> columns;
  string column;
  float util{0.0};
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    getline(stream, line);
    std::istringstream linestream(line);
    while(linestream.good()) {
      getline(linestream, column, ' ');
      columns.push_back(column);
    }
    int totalProcessTicks = stoi(columns[13]) + stoi(columns[14]) + stoi(columns[15]) + stoi(columns[16]);
    float totalProcessTime = totalProcessTicks / (float)sysconf(_SC_CLK_TCK);
    long totalSeconds = UpTime(pid);
    util = totalSeconds != 0 ? (totalProcessTime/(float)totalSeconds) : 0.0;
  }
  return util;
}

int LinuxParser::TotalProcesses() { 
  string key;
  string value;
  string line;
  int processes;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "processes") {
          linestream >> processes;
          break;
        }
      }
    }
  }
  return processes;  
}

int LinuxParser::RunningProcesses() { 
  string line;
  string key;
  int running_processes = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "procs_running") {
          linestream >> running_processes;
          break;
        }
      }
    }
  }
  return running_processes;
}

string LinuxParser::Command(int pid) { 
  string cmd;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, cmd);
  }
  return cmd;
}

string LinuxParser::Ram(int pid) { 
  string line;
  string key;
  long ram = -1;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      /*       
      replacing VmSize regarding to previous review 
      
      Whereas when you use VmRSS then it gives the exact physical memory being used as a part of Physical RAM. So it is recommended to replace the string VmSize with VmRSS as people who will be looking at your 		GitHub might not have any idea of Virtual memory and so they will think you have done something wrong!  
      PS - Moreover when you replace then please put a comment stating that you have used VmRSS instead of 	   VmSize because it might happen that another reviewer is following the Udacity guideline and so 		  he/she might make it a required change but once you put the comment with the link to the resources then he 	will surely understand that!
      */
      if (key == "VmRSS:") {
        linestream >> ram;
        break;
      }
    }
  }
  return std::to_string(ram/ 1024);
}

string LinuxParser::Uid(int pid) { 
  string line;
  string key;
  string uid;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "Uid:") {
          linestream >> uid;
          break;
        }
      }
    }
  }
  return uid;  
}

string LinuxParser::User(int pid) { 
  string uid = Uid(pid);
  string id, x, line;
  string usr = "DEFAULT";
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> usr >> x >> id;
      if (id == uid) {
        break;
      }
    }
  }
  return usr;
}

long LinuxParser::UpTime(int pid) { 
  string value;
  string line;
  vector<string> states;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      states.emplace_back(value);
    }
  }
  return LinuxParser::UpTime() - std::stol(states[21])/sysconf(_SC_CLK_TCK);
}