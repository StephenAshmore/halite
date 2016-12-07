#include <fstream>
#include <string>

using std::ofstream;
using std::endl;
using std::string;

void log(string str) {
  static ofstream out;
  if ( !out )
    out.open("log.txt");
  out << str;
  out.flush(); out.flush();
}
