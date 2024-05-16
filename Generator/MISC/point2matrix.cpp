#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <random>
#include <limits>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;
using std::vector;
using std::ifstream;
using std::ofstream;

const int SCALE = 10;

int **malloc2d (int, int);
void free2d (int ***, int);

int main (int argc, char **argv) {
  int **mat, count;
  int i, j, r, c, mat_dim, mat_init_r, mat_init_c, r_min, r_max, c_min, c_max;
  float r_tot, c_tot, n_num;
  ifstream file_i;
  ofstream file_o;
  
  //
  //
  //

  char *file_in = argv[1];
  char *file_out = argv[2];
  mat_dim = atoi (argv[3]);
  mat_init_r = atoi (argv[4]);
  mat_init_c = atoi (argv[5]);
  mat = malloc2d (mat_dim, mat_dim);

  cout << argv[0] << " " << file_in << " " << file_out << " " <<
    mat_dim << " " << mat_init_r << " " << mat_init_c << endl;
  
  //
  // Compute stats
  //
  
  file_i.open(file_in);
  if (!file_i.is_open()) {
    cerr << "Could not open the file " << argv[1] << endl;
    exit (EXIT_FAILURE);
  }

  n_num = r_tot = c_tot = 0;
  r_min = c_min = std::numeric_limits<int>::max();
  r_max = c_max = std::numeric_limits<int>::min();
  while (file_i >> r >> c) {
    r = (int) (r/SCALE);
    c = (int) (c/SCALE);
    if (r<r_min) r_min = r;
    if (r>r_max) r_max = r;
    if (c<c_min) c_min = c;
    if (c>c_max) c_max = c;
    r_tot = r_tot + r;
    c_tot = c_tot + c;
    n_num++;
    //cout << r << " " << c << endl;
  }

  cout << "Min-max-min-max "
       << r_min << " " << r_max << " " << c_min << " " << c_max << endl;
  cout << "Point_N C_avg R_avg "
       << n_num << " " << c_tot/n_num << " " << r_tot/n_num << " " << endl;  

  file_i.close();

  //
  // Generate Matrix
  //

  for (i=0; i<mat_dim; i++)
    for (j=0; j<mat_dim; j++)
      mat[i][j] = 1;

  file_i.open(file_in);
  if (!file_i.is_open()) {
    cerr << "Could not open the file " << argv[1] << endl;
    exit (EXIT_FAILURE);
  }

  count = 0;
  while (file_i >> r >> c) {
    //cout << "[" << r << "," << c << "]" << "(" << r-mat_init_r << "," << c-mat_init_c << ")" << endl;
    if ((r-mat_init_r)>0 && (r-mat_init_r)<mat_dim && (c-mat_init_c)>0 && (c-mat_init_c)<mat_dim) {
      mat[r-mat_init_r][c-mat_init_c] = 2;
      count++;
    }
  }

  //
  // Store Matrix
  //

  file_o.open(file_out);
  if (!file_o.is_open()) {
    cerr << "Could not open the file " << argv[2] << endl;
    exit (EXIT_FAILURE);
  }

  file_o << ".c" << endl;  
  for (i=0; i<mat_dim; i++) {
    for (j=0; j<mat_dim; j++) {    
      file_o << mat[i][j];
    }
    file_o << endl;
  }

  file_o.close();

  cout << "Mat[" << mat_dim << "][" << mat_dim << "]: " <<
    count << "[" << (((float) count / (float) (mat_dim*mat_dim)) * 100) << "%]" << endl;

  free2d (&mat, mat_dim);
  
  return (1);
}

int **malloc2d(int r, int c) {
  int i;
  int **mat;

  mat = (int **) malloc (r * sizeof(int *));
  if (mat == NULL) {
    fprintf (stderr, "Memory allocation error.\n");
    exit(EXIT_FAILURE);
  }
  for (i=0; i<r; i++) {
    // Must be initialized to zero !!!
    mat[i] = (int *) calloc(c, sizeof (int));
    if (mat[i]==NULL) {
      fprintf (stderr, "Memory allocation error.\n");
      exit(EXIT_FAILURE);
    }
  }

  return mat;
}

void free2d (int ***mat, int r) {
  int **lm;
  int i;

  lm = *mat;
  for (i=0; i<r; i++) {
    free (lm[i]);
  }
  free (lm);
  *mat = NULL;
  
  return;
}
