#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {
    long h = seconds / 3600;
    seconds = seconds % 3600;
    long m = seconds / 60;
    seconds = seconds % 60;
    long s = seconds;

    return std::to_string(h) + ": " +  std::to_string(m) + ": " + std::to_string(s);
}