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

#define DIST 0

const string file_name="random.txt";

const int
  r_min =100,
  r_max = 100,
  r_step = 5,
  c_min =100,
  c_max = 100,
  c_step = 5;

float my_rand_f (float min, float max) {
  float rn;
  rn = min + (((float) rand ()) / (float) (RAND_MAX)) * ((float) (max-min));
  //cout << "[" << rn << "]";
  return (rn);
}

int main () {
  int mat[r_max][c_max];

#if DIST
  std::default_random_engine generator;
#if 0
  for (int i=0; i<100; i++) {
    double d = distribution(generator);
    cout << d << " ";
  }
  exit (1);
#endif
#endif
  
  ofstream file_o(file_name);
  if (!file_o.is_open()) {
    cerr << "Could not open the file " << file_name << endl;
    exit (EXIT_FAILURE);
  }
	    
  for (int r=r_min; r<=r_max; r+=r_step) {
    for (int c=c_min; c<=c_max; c+=c_step) {

#if DIST
      double media, variance;
      media = my_rand_f (50, 100);
      variance = my_rand_f (1, 50);
      std::normal_distribution<double> distribution (media, variance);
#endif
  
      for (int i=0; i<r; i++) {
        for (int j=0; j<c; j++) {
	  if (i==0 || i==r-1 || j==0 || j==c-1) {
	    mat[i][j] = 1;
	  } else {
	    if (mat[i-1][j-1]!=1 || mat[i-1][j]!=1 ||mat[i-1][j+1]!=1 || mat[i][j-1]!=1) {
	      mat[i][j] = 1;
	    } else {

#if 0
	      float f = my_rand_f (0, 100);
	      double d = distribution(generator);
	      cout << "(" << f << ")" << "[" << d << "]" << endl;
	      if (f>0.8*d && f<1.2*d) {
		mat[i][j] = 1;
	      } else {
	        if (f>0.6*d && f<1.4*d) {
		  mat[i][j] = 2;
		} else {
	          if (f>0.4*d && f> 1.6*d) {
		    mat[i][j] = 3;
		  } else {
		    mat[i][j] = 4;
		  }
		}
	      }
#endif

#if 1
	      float f = my_rand_f (0, (i+j));	      
	      if (f<(0.5*(i+j))) {
		mat[i][j] = 1;
	      } else {
		  if (f<(0.9*(i+j))) {
		  mat[i][j] = 2;
		} else {
		 if (f<(0.95*(i+j))) {
		    mat[i][j] = 3;
		  } else {
		    mat[i][j] = 4;
		  }
		}
	      }
#endif

	      
	    }
	  }
        }
      }

      file_o << ".c" << endl;
      for (int i=0; i<r; i++) {
        for (int j=0; j<c; j++) {
	  file_o << mat[i][j];
	}
	file_o << endl;
      }
      
    }
  }

  file_o.close();

  return (1);
}
