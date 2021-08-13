#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() { return std::stof(LinuxParser::CpuUtilization()[0]); }