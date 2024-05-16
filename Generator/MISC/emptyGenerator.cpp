#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <random>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;
using std::vector;
using std::ifstream;
using std::ofstream;

const string file_name="empty.txt";

const int
  r_min =40,
  r_max = 80,
  r_step = 20,
  c_min =40,
  c_max = 80,
  c_step = 20;

int main () {
  ofstream file_o(file_name);
  if (!file_o.is_open()) {
    cerr << "Could not open the file " << file_name << endl;
    exit (EXIT_FAILURE);
  }
	    
  for (int r=r_min; r<=r_max; r+=r_step) {
    for (int c=c_min; c<=c_max; c+=c_step) {

      file_o << ".c" << endl;      
      for (int i=0; i<r; i++) {
        for (int j=0; j<c; j++) {
	  file_o << "1";
        }
        file_o << endl;
      }
      
    }
  }

  file_o.close();

  return (1);
}
