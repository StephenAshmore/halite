#include <fstream>

using std::ofstream;
using std::endl;

void log(char const* str) {
  static ofstream out;
  if ( !out )
    out.open("log.txt");
  out << str << endl;
}
