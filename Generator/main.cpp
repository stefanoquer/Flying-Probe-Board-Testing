/*
 *  This work is lecende mnder
 *  CC BY-NC-ND 4.0 Deed
 *  Attribution-NonCommercial-NoDerivs 4.0 International 
 *  Canonical URL https://creativecommons.org/licenses/by-nc-nd/4.0/
 *
 *  StQ 20.09.2023
 *  Board Generator
 *  - No parameters
 *  - It creates ../../Flying-Probe-Board/dir-board-with-date-of-today
 *
 *
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::to_string;
using std::vector;
using std::ifstream;
using std::ofstream;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
uint32_t rmask = 0xff000000;
uint32_t gmask = 0x00ff0000;
uint32_t bmask = 0x0000ff00;
uint32_t amask = 0x000000ff;
#else
uint32_t rmask = 0x000000ff;
uint32_t gmask = 0x0000ff00;
uint32_t bmask = 0x00ff0000;
uint32_t amask = 0xff000000;
#endif

// Generic Constant
const bool
  DEBUG_L1 = true,
  DEBUG_L2 = false;

const int
  LENGTH1 = 100,
  LENGTH2 = 200,
// Ignore no_fly and no_touch
  IGNORE = 0,
  // Check components (if 1 use all component from 0 on circularly)
  CHECK_COMPONENT = 0,
  // Check components (if 1 use all component from 0 on circularly) AND EXIT
  CHECK_COMPONENT_AND_EXIT = 0;

// SCALE = 1 matrix element == SCALE mm
// SCALE=0.25: 400=10cm, 2000=50cm
// DENSITY == 1 -> insert a new component for each free element
const float
  SCALE = 0.25,
  DENSITY = 0.01,
  // Component Direction: Alwasy  vertical 0; Always horizontal 1; 50-50 = 0.5
  DIRECTION = 0.5,
  // Repeat previous component with this probability (1==always the first one)
  REPETITION = 0.2;

// Board Size (x,y,min,max,delta)
const int
  R_MIN = 200,
  R_MAX = 1600,
  //R_MAX = 300,
  R_DELTA = 200,
  C_MIN = 200,
  C_MAX = 1600,
  //C_MAX = 300,
  C_DELTA = 200;

// Points values
const int
    BACKGROUND = 0,
    COMPONENT = 1,
    PIN = 2,
    GND = 3,
    VDD = 4,
    CLOCK = 5,
    NET = 6,
    NOFLY = 7,
    NOTOUCH = 8,
    POINT = 9;

// Points colors
std::tuple<int,int,int> pin_color = std::make_tuple(0,0,0);
std::tuple<int,int,int> gnd_color = std::make_tuple(0,255,0);
std::tuple<int,int,int> vdd_color = std::make_tuple(255,0,0);
std::tuple<int,int,int> clock_color = std::make_tuple(0,0,255);
std::tuple<int,int,int> nofly_color = std::make_tuple(143,0,255);
std::tuple<int,int,int> notouch_color = std::make_tuple(66,49,137);

// Point names
vector<string> point_name_v = {
    "background",
    "component",
    "pin",
    "vdd",
    "gnd",
    "clock",
    "net",
    "nofly", 
    "notouch",
    "point"
    };

// Input File Names
class input_file {
    public:
        string file_name;
        int count_max;
};

// Name; frequence = -1 do not use, 0 no limit in use, n limit to that max count (FOR ENTIRE FILE NOT SINGLE COMPONENT)
vector<input_file> file_name_input {
   {"COMPONENTS/chip.txt", 0},
   {"COMPONENTS/empty.txt", 0},
   {"COMPONENTS/nofly.txt", 2},
   {"COMPONENTS/notouch.txt", 2},
   {"COMPONENTS/random.txt", 0},
   {"COMPONENTS/resistor.txt",0},
   {"COMPONENTS/small.txt", 0},
   {"COMPONENTS/big.txt", 0}
};

// Output File  names
const string
    BOARD_DIR = "./../Flying-Probe-Board",
    BOARD_NAME = "board_",
    BOARD_MATRIX_A = "matrix_side1.txt",
    BOARD_MATRIX_B= "matrix_side2.txt",
    BOARD_POINT = "board.txt",
    BOARD_POINT_JSON = "board.json",
    BOARD_TEST_S = "test_s.txt",
    BOARD_TEST_S_JSON = "test_s.json",
    BOARD_TEST_M = "test_m.txt",
    BOARD_TEST_M_JSON = "test_m.json",
    BOARD_TEST_L = "test_l.txt",
    BOARD_TEST_L_JSON = "test_l.json",
    BOARD_STAT = "stat.txt",
    BOARD_IMG_A = "img_side1.bmp",
    BOARD_IMG_B = "img_side2.bmp";

// A point is a pin, gnd, vdd, etc.
class point {
    public:
        // Side = 1 OR 2
        char side;
        // Position
        int r;
        int c;
        // pin, gnd, vdd, etc.
        int type;
        // Corresponding net
        int net;

        point (char lside, int lr, int lc, int ltype, int lnet) {
            side = lside;
            r = lr;
            c = lc;
            type = ltype;
            net = lnet;
            return;
        }
};

class component {
    public:
        int r, c, type, count, count_max;
        int pin_n, gnd_n, vdd_n, clock_n, nofly_n, notouch_n;
        char **side;

        component (int r_size, int c_size) {
            r = r_size;
            c = c_size;
            pin_n = gnd_n = vdd_n = clock_n = nofly_n = notouch_n = 0;
            type = count = count_max = 0;

            side = (char **) calloc (r, sizeof (char *));
            if (side == nullptr) {
                cout << "Allocation Error: Component" << endl;
                exit (1);
            }
            for (int i=0; i<r; i++) {
                side[i] = (char *) calloc (c, sizeof (char));
                if (side[i] == nullptr) {
                    cout << "Allocation Error: Board size 2" << endl;
                    exit (1);
                }
            }
            return;
        }

        void display () {
            cout << "Component [r=" << r << ",c=" << c << "]";
            cout << "[type=" << point_name_v[type] << "]";
            cout << "[count=" << count << ",count_max=" << count_max << "]";
            cout 
              << "[pin_n=" << pin_n << ",gnd_n=" << gnd_n << ",vdd_n=" << vdd_n
              << ",clock_n=" << clock_n << ",nofly_n=" << nofly_n
              << ",notouch_n=" << notouch_n << "]" << endl;
            if (DEBUG_L2) {
	      for (int i=0; i<r; i++) {
                for (int j=0; j<c; j++) {
		  cout << int(side[i][j]);
                }
                cout << endl;
	      }
	    }
            return;   
        }

};

class board {

    public:
        int r, c;
        vector<int> component_background_n = {0, 0, 0};
        vector<int> component_point_n = {0, 0, 0};
        vector<int> component_nofly_n = {0, 0, 0};
        vector<int> component_notouch_n = {0, 0, 0};
        vector<int> pin_n = {0, 0, 0};
        vector<int> gnd_n = {0, 0, 0};
        vector<int> vdd_n = {0, 0, 0};
        vector<int> clock_n = {0, 0, 0};
        vector<int> net_n = {0, 0, 0};
        vector<int> nofly_n = {0, 0, 0};
        vector<int> notouch_n = {0, 0, 0};
        vector<int> point_n = {0, 0, 0};
        vector<int> test_n = {0, 0, 0};
        vector<vector<point>> point_v;
        vector<vector<point>> net_v;
        vector<vector<point>> test_v;
        char ***side = nullptr;
      
        board (int r_size_max, int c_size_max) {
            side = (char ***) calloc (2, sizeof (char **));
            if (side == nullptr) {
                cout << "Allocation Error: Board (dim 1)" << endl;
                exit (1);
            }
            for (int i=0; i<2; i++) {
                side[i] = (char **) calloc (r_size_max, sizeof (char *));
                if (side[i] == nullptr) {
                    cout << "Allocation Error: Board (dim 2)" << endl;
                    exit (1);
                }
            }
            for (int i=0; i<2; i++) {
                for (int j=0; j<r_size_max; j++) {
                    side[i][j] = (char *) calloc (c_size_max, sizeof (char));
                    if (side[i][j] == nullptr) {
                        cout << "Allocation Error: Board (dim 3)" << endl;
                        exit (1);
                    }
                }
            }

            r = r_size_max;
            c = c_size_max;

            return;
        }

        int my_rand_i (int min, int max) {
            int rn;
            rn = min + (((float) rand ()) / (float) (RAND_MAX)) * ((float) (max-min+1));
            //cout << "[" << rn << "]";
            return (rn);
        }

       float my_rand_f (float min, float max) {
            float rn;
            rn = min + (((float) rand ()) / (float) (RAND_MAX)) * ((float) (max-min));
            //cout << "[" << rn << "]";
            return (rn);
        }

        int my_rand_i_rep (int rn, int min, int max) {
            if (my_rand_f(0,1)>REPETITION) {
                rn = min + (((float) rand ()) / (float) (RAND_MAX)) * ((float) (max-min+1));
                //cout << "[" << rn << "]";
            }
            return (rn);
        }
 
        void reset () {
            reset (r, c);
            return;
        }

        void reset (int rr, int cc) {
            for (int i=0; i<2; i++) {
                for (int j=0; j<rr; j++) {
                    for (int k=0; k<cc; k++) {
                        side[i][j][k] = BACKGROUND;
                    }
                }
            }

            for (int i=0; i<3; i++) {
                component_background_n[i] = 0;
                component_point_n[i] = 0;
                component_nofly_n[i] = 0;
                component_notouch_n[i] = 0;
                pin_n[i] = 0;
                gnd_n[i] = 0;
                vdd_n[i] = 0;
                clock_n[i] = 0;
                net_n[i] = 0;
                nofly_n[i] = 0;
                notouch_n[i] = 0;
                point_n[i] = 0;
                test_n[i] = 0;
            }

            point_v.clear();
            net_v.clear();
            test_v.clear();

            return;
        }

        void mat2point (int rr, int cc) {
            for (int i=0; i<2; i++) {
                vector<point> tmp;
                for (int j=0; j<rr; j++) {
                    for (int k=0; k<cc; k++) {
                        if (side[i][j][k]==PIN || side[i][j][k]==GND ||
                          side[i][j][k]==VDD || side[i][j][k]==CLOCK ||
                            (!IGNORE & (side[i][j][k]==NOFLY || side[i][j][k]==NOTOUCH))) {
                            if (i==0) {
                                point p('T',j,k,side[i][j][k],0);
                                tmp.push_back(p);
                            } else {
                                point p('B',j,k,side[i][j][k],0);
                                tmp.push_back(p);
                            }
                        }
                    }
                }
                point_v.push_back(tmp);
            }
            return;
        }

        void net_create () {
            int i, n;
            float min = 0.1, max = 0.2;

            i = point_n[2]/2;
            n = my_rand_i (min*i, max*i);
            if (n<3) {
                cerr << "Number of nets less than three (#points=" << point_n[2] << ")" << endl;
                exit (1);
            }
            for (i=0; i<n; i++) {
                net_v.push_back(vector<point>());
            }

            for (auto &p: point_v) {
                for (auto &pp: p) {
                    if (pp.type==GND) {
                        pp.net = GND;
                        net_v[0].push_back(pp);
                    } else if (pp.type==VDD) {
                        pp.net = VDD;
                        net_v[1].push_back(pp);
                    } else if (pp.type==CLOCK) {
                        pp.net = CLOCK;
                        net_v[2].push_back(pp);
                    } else if (pp.type==PIN) {
                        float f;
                        f = my_rand_f (0.0, 1.0);
                        // POSSONO RIMANERE NET VUOTE
                        if (f>=min && f<=max) {
                            int v = my_rand_i (3, n-1);
                            if (v>=n) cout << "XXX" << endl;
                            pp.net = PIN;
                            net_v[v].push_back(pp);
                        }
                    }
                }
            }    
 
            return;
        }

        void test_create (int sml, float min, float max) {
            // test number, point number, point position
            int tn, pn, pp;
            int gnd_flag, vdd_flag, clock_flag;

            if (point_n[2]==0) {
                cout << "No nets (as there are no points)." << endl;
                return;
            }

            // Reset test array (becasue I create small, medium, large test sets)
            test_v.clear();

            // Number of test 
            tn = my_rand_i (min*(point_n[2]/2), max*(point_n[2]/2));
 
            for (test_n[sml]=0; test_n[sml]<tn; test_n[sml]++) {
                vector<point> tmp;
                vector<int> tmpi;
                
                gnd_flag = vdd_flag = clock_flag = 0;
                for (int i=0; i<2; i++) {
                    // Number of TOP/BOTTOM points in this test
                    pn = my_rand_i (1, 4);
                    // For each point
                    int j=0;
                    while (j<pn) {
                        // Select the test point
                        pp = my_rand_i (0, ((point_v[i].size())-1));
                        // Avopdi duplicated points in one test
                        if (std::find(tmpi.begin(), tmpi.end(), pp) == tmpi.end()) {
                            if (point_v[i][pp].type==PIN) {
                                tmpi.push_back(pp);                   
                                tmp.push_back(point_v[i][pp]);
                                j++;
                            } else
                            if (point_v[i][pp].type==GND && gnd_flag==0) {
                                gnd_flag = 1;
                                tmpi.push_back(pp);                   
                                tmp.push_back(point_v[i][pp]);
                                j++;
                            } else
                            if (point_v[i][pp].type==VDD && vdd_flag==0) {
                                vdd_flag = 1;
                                tmpi.push_back(pp);                   
                                tmp.push_back(point_v[i][pp]);
                                j++;
                            } else
                            if (point_v[i][pp].type==CLOCK && clock_flag==0) {
                                clock_flag = 1;
                                tmpi.push_back(pp);                   
                                tmp.push_back(point_v[i][pp]);
                                j++;
                            }
                        }
                    }
                }
                test_v.push_back(tmp);
            }
  
            return;
        }

        void mat_save (string name_txt, int side_n, int rr, int cc) {
            ofstream file_o(name_txt);
            if (!file_o.is_open()) {
                cerr << "Could not open the file " << name_txt << endl;
                exit (EXIT_FAILURE);
            }
            
            for (int j=0; j<rr; j++) {
                for (int k=0; k<cc; k++) {
                    file_o << int(side[side_n][j][k]);
                }
                file_o << endl;
            }
            file_o << endl;

            file_o.close();
            return;   
        }

        void board_save (string name_board, string name_txt, string name_json, int rr, int cc) {
            bool firstA, firstB;

            ofstream file_o(name_txt);
            ofstream file_o_json(name_json);
            if (!file_o.is_open() || !file_o_json.is_open()) {
                cerr << "Could not open the file " << name_txt << " or " << name_json << endl;
                exit (EXIT_FAILURE);
            }

            file_o << ".width " << ((float) rr*SCALE) << endl;
            file_o << ".height " << ((float) cc*SCALE) << endl;

            net_n[0] = 0;
            for (auto v: net_v) {
                if (v.size()==0)
                    continue;
                net_n[0]++;
            }
            for (auto tmp:point_v) {
                for (auto e:tmp) {
                    if (e.net==0) {
                        net_n[0]++;
                    }
                }
            }

            file_o_json << "{" << endl;
            file_o_json << "\t\"name\": \"" << name_board << "\"," << endl;
            file_o_json << "\t\"size\": {" << endl;
            file_o_json << "\t\t\"width\": " << ((float) rr*SCALE) << "," << endl;
            file_o_json << "\t\t\"height\": " << ((float) cc*SCALE) << "," << endl;
            file_o_json << "\t\t\"test_points\": " << point_n[2] << "," << endl;
            file_o_json << "\t\t\"nets\": " << net_n[0] << endl;
            file_o_json << "\t}," << endl;

            file_o_json << "\t\"test_points\": {"  << endl;
            firstA = true;
            for (auto tmp:point_v) {
                if (firstA==true) {
                    firstA = false;
                    file_o_json << "\t\t\"top\": {" << endl;
                } else {
                    file_o_json << "," << endl;
                    file_o_json << "\t\t\"bottom\": {" << endl;
                }

                firstB = true;
                for (auto e:tmp) {
                    file_o << ".point " << point_name_v[e.type] << "_" << e.side << "_" <<
                      e.r << "_" << e.c << " " << e.side << " " << ((float) e.r*SCALE) << " " << ((float) e.c*SCALE) << endl;

                    if (firstB==true) {
                        firstB = false;
                    } else {
                        file_o_json << "\t\t\t]," << endl;
                    }
                    file_o_json << "\t\t\t\"" << point_name_v[e.type] << "_" << e.side << "_" << e.r << "_" << e.c << "\": [" << endl;
                    file_o_json << "\t\t\t\t" << ((float) e.r*SCALE) << "," << endl;
                    file_o_json << "\t\t\t\t" << ((float) e.c*SCALE) << endl;
                }

                file_o_json << "\t\t\t]" << endl;
                file_o_json << "\t\t}";
            }

            //
            // net save
            //

            file_o_json << endl;
            file_o_json << "\t}," << endl;
            file_o_json << "\t\"nets\": {" << endl;
            int i = 0;
            //net_n[0] = 0;
            for (auto v: net_v) {
                if (v.size()==0)
                    continue;
                //net_n[0]++;
                file_o  << ".net net_" << i << " " << v.size() << " ";
                file_o_json  << "\t\t\"net_" << i << "\": [" << endl;
                firstA = true;
                for (auto e:v) {
                    file_o << point_name_v[e.type] << "_" << e.side << "_" << e.r << "_" << e.c << " ";
                    if (firstA==true) {
                        firstA = false;
                    } else {
                        file_o_json << "," << endl;
                    }
                    file_o_json << "\t\t\t\"" << point_name_v[e.type] << "_" << e.side << "_" << e.r << "_" << e.c << "\"";
                }
                i++;
                file_o << endl;
                file_o_json << endl;
                file_o_json << "\t\t]," << endl;
            }     

            //file_o << "HERE STARTS NET WITH FORGOTTEN POINT ..." << endl;

            // Net for points with not net
            firstA = true;
            for (auto tmp:point_v) {
                for (auto e:tmp) {
                    if (e.net==0) {
                        if (firstA==true) {
                            firstA = false;
                        } else {
                            file_o_json << "," << endl;
                        }
                        //net_n[0]++;
                        file_o << ".net net_" << i << " 1 " << point_name_v[e.type]
                               << "_" << e.side << "_" << e.r << "_" << e.c << endl;
                        file_o_json << "\t\t\"net_" << i << "\": [" << endl;
                        file_o_json << "\t\t\t\"" << point_name_v[e.type] << "_" << e.side << "_" << e.r << "_" << e.c << "\"" << endl;
                        file_o_json << "\t\t]";
                        i++;
                    }
                }
            }

            file_o_json << endl;
            file_o_json << "\t}" << endl;
            file_o_json << "}" << endl;

            file_o.close();
            file_o_json.close();

            return;   
        }

        void test_save (string name_txt, string name_json) {
            ofstream file_o(name_txt);
            ofstream file_o_json(name_json);
            if (!file_o.is_open() || !file_o_json.is_open()) {
                cerr << "Could not open the file " << name_txt << " or " << name_json << endl;
                exit (EXIT_FAILURE);
            }

            file_o_json  << "{" << endl;

            int i = 0;
            for (auto v: test_v) {
                i++;
                file_o  << ".test test_" << i << " " << v.size() << " ";
                file_o_json  << "\"test\":{\"name\":\"test_" << i << "\",\"size\":" << v.size() << ",\"point\":[";
                int j = 0;
                for (auto p:v) {
                    j++;
                    file_o << point_name_v[p.type] << "_" << p.side << "_" << p.r << "_" << p.c << " ";
                    if (j!=1) {
                        file_o_json << ",";
                    }
                    file_o_json << point_name_v[p.type] << "_" << p.side << "_" << p.r << "_" << p.c;
                }
                file_o << endl;
                file_o_json << "]}" << endl;
            }     

            file_o_json  << "}" << endl;

            file_o.close();
            file_o_json.close();
    
            return;
        }

        void stat_save (string file_name, int rr, int cc) {
            ofstream file_o(file_name);
            if (!file_o.is_open()) {
                cerr << "Could not open the file " << file_name << endl;
                exit (EXIT_FAILURE);
            }

            file_o << "Board size [mm] " << ((float) rr*SCALE) << "x" << ((float) cc*SCALE) << endl << endl;

            for (int i=0; i<3; i++) {
                if (i<2)
                    file_o << "Stats side " << i+1 << endl;
                else
                    file_o << "Stats board (side 1 + side 2)" << endl;
                file_o << "  #Components with background " << component_background_n[i] << endl;
                file_o << "  #Components with points " << component_point_n[i] << endl;
                file_o << "  #Components nofly " << component_nofly_n[i] << endl;
                file_o << "  #Components notouch " << component_notouch_n[i] << endl;
                file_o << "  #Pins " << pin_n[i] << endl;
                file_o << "  #GNDs " << gnd_n[i] << endl;
                file_o << "  #VDDs " << vdd_n[i] << endl;
                file_o << "  #Clock " << clock_n[i] << endl;
                file_o << "  #NoFly points " << nofly_n[i] << endl;
                file_o << "  #NoTouch points " << notouch_n[i] << endl;
                file_o << "  #Points [total] " << point_n[i] << endl;
                if (i!=2)
                    file_o << endl;
            }

            file_o << "  #Nets " << net_n[0] << endl;
            file_o << "    Net Size: ";
            for (auto nt: net_v) {
                if (nt.size()!=0) {
                    file_o << nt.size() << " ";
                }
            }
            file_o << endl;

            file_o << "  #Test Small " << test_n[0] << endl;
            file_o << "  #Test Medium " << test_n[1] << endl;
            file_o << "  #Test Large " << test_n[2] << endl;

            file_o.close();
            return;   
        }

        void pixel_draw (SDL_Surface *surface, int x, int y, Uint32 colour) {
            int byteperpixel = surface->format->BytesPerPixel;

            Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * byteperpixel;

            // Address to pixel
            *(Uint32 *)p = colour;
        }

        // Draw a line
        void line_draw (SDL_Surface *surface, int x0, int y0, int x1, int y1, Uint32 colour) {
            double x = x1 - x0;
            double y = y1 - y0;
            const double length = sqrt(x * x + y * y);
            const double addx = x / length;
            const double addy = y / length;
            x = x0;
            y = y0;

            for (int i = 0; i < length; i++) {
                pixel_draw (surface, x, y, colour);
                x += addx;
                y += addy;
            }
        }

        // Draw a rectangle
        void rectangle_fraw(SDL_Surface *surface, SDL_Rect rect, Uint32 colour) {
            // Top
            line_draw (surface, rect.x, rect.y, rect.x + rect.w, rect.y, colour);
            // Left
            line_draw (surface, rect.x, rect.y, rect.x, rect.y + rect.h, colour);
            // Bottom
            line_draw (surface, rect.x, rect.y + rect.h, rect.x + rect.w + 1, rect.y + rect.h, colour);
            // Right
            line_draw (surface, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h, colour);
        }

        void img_save (string file_name, int side_n, int rr, int cc) {
            int rcol, gcol, bcol;
            
            // SWAP rows and columns to match interger matrix

            // Initialize library
            SDL_Init(SDL_INIT_VIDEO);

            // Create image with dimensions WIDTHxHEIGHT
            SDL_Surface *surface = SDL_CreateRGBSurface(0, cc, rr, 32, rmask, gmask, bmask, amask);

            // Fill background
            SDL_Rect bg = {0, 0, cc, rr};
            SDL_FillRect(surface, &bg, SDL_MapRGB(surface->format, 255, 255, 255));
         
            for (auto e:point_v[side_n]) {
                // Draw points COLOR --> PIN GND VDD etc
                // Paramters: pixel format, r in rgb 0-255, g in rgb 0-255, b in rgb 0-255
                if (e.type==GND) {
                    rcol = std::get<0>(gnd_color);
                    gcol = std::get<1>(gnd_color);
                    bcol = std::get<2>(gnd_color);
                    pixel_draw (surface, e.c, e.r, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                } else if (e.type==VDD) {
                    rcol = std::get<0>(vdd_color);
                    gcol = std::get<1>(vdd_color);
                    bcol = std::get<2>(vdd_color);
                    pixel_draw (surface, e.c, e.r, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                } else if (e.type==CLOCK) {
                    rcol = std::get<0>(clock_color);
                    gcol = std::get<1>(clock_color);
                    bcol = std::get<2>(clock_color);
                    pixel_draw (surface, e.c, e.r, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                } else if (e.type==NOFLY) {
                    rcol = std::get<0>(nofly_color);
                    gcol = std::get<1>(nofly_color);
                    bcol = std::get<2>(nofly_color);
                    //line_draw (surface, e.c-1, e.r-1, e.c+1, e.r+1, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                    line_draw (surface, e.c-1, e.r, e.c+1, e.r, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                } else if (e.type==NOTOUCH) {
                    rcol = std::get<0>(notouch_color);
                    gcol = std::get<1>(notouch_color);
                    bcol = std::get<2>(notouch_color);
                    //line_draw (surface, e.c, e.r-1, e.c, e.r+1, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                    line_draw (surface, e.c, e.r-1, e.c, e.r+1, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                } else {
                    rcol = std::get<0>(pin_color);
                    gcol = std::get<1>(pin_color);
                    bcol = std::get<2>(pin_color);
                    pixel_draw (surface, e.c, e.r, SDL_MapRGB(surface->format, rcol, gcol, bcol));
                }
            }

            // Draw proper rectangle
            //SDL_Rect rect{400, 0, 100, 150};
            //SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 245, 194, 66));

            // Draw rectangle
            // rectangle_draw (surface, rect, SDL_MapRGB(surface->format, 0, 0, 0));

            // Draw line
            //line_draw (surface, 500, 0, 550, 600, SDL_MapRGB(surface->format, 0, 0, 100));

            SDL_SaveBMP(surface, file_name.c_str());
            SDL_Quit();

            return;   
        }

};    

vector<component> components_read ();
void tmp2comp (vector<string> &, vector<component> &, int);
void create_board (board &,  vector<component>, int, int);
int check (board &, component, int, int, int, int, int);
void insert (board &, component &, int, int, int, int);

int main() {
    board myboard(R_MAX,C_MAX);
    vector<component> components;
    struct stat sb;
    char todayc[LENGTH1], command[LENGTH2];
    string s, todaycpp, name_board, name_txt, name_json;
    int i, r_size, c_size;

    cout << std::setprecision(2) << std::fixed;

    if (stat(BOARD_DIR.c_str(), &sb)==0) {
      cout << "Target directory (exists) " << BOARD_DIR << endl;
    } else {
      cout << "Creating directory        " << BOARD_DIR << endl;
      sprintf (command, "mkdir %s", BOARD_DIR.c_str());
      system (command);
    }

    time_t now = time(0);
    tm *gmtm = gmtime (&now);
    sprintf (todayc, "%s/%04d%02d%02d", BOARD_DIR.c_str(),
	     1900+gmtm->tm_year, 1+gmtm->tm_mon, gmtm->tm_mday);
    todaycpp = todayc;
    if (stat(todayc, &sb)==0) {
      cout << "Removing old directory    " << todaycpp << endl;
      sprintf (command, "/bin/rm -rf %s", todayc);
      system (command);
    }
    cout << "Creating new directory    " << todaycpp << endl;
    sprintf (command, "mkdir %s", todayc);
    system (command);
    
    // srand (getpid());
    components = components_read ();

    if (DEBUG_L1) {
      cout << "Reading Components" << endl;
      i=0;
      for (auto c: components) {
        cout << i+1 << ".";
        c.display ();
        i++;
      }
    }

    //
    // Create Board
    //

    i = 0;
    for (r_size=R_MIN; r_size<=R_MAX; r_size+=R_DELTA) {
        for (c_size=C_MIN; c_size<=C_MAX; c_size+=C_DELTA) {
            cout << "Create board [r=" << r_size << ",c=" << c_size << "]" << endl;
            myboard.reset (r_size, c_size);
            for (long unsigned int x=0; x<components.size(); x++) {
                //cout << x.count;
                components[x].count = 0;
            }
            create_board (myboard, components, r_size, c_size);
            myboard.mat2point (r_size, c_size);
            myboard.net_create ();

            // Create su-directory for that board
            sprintf (command, "mkdir %s/%s%02d", todayc, BOARD_NAME.c_str(), i);
            system (command);

            // Creat board number of two digits
            std::stringstream ss;
            ss << std::setw(2) << std::setfill('0') <<i;
            string s = ss.str();

            // Save board: Matrix Side A and B
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_MATRIX_A;
            myboard.mat_save(name_txt, 0, r_size, c_size);
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_MATRIX_B;
            myboard.mat_save(name_txt, 1, r_size, c_size);

            // Save board: BOARD format
            name_board = BOARD_NAME + s;
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_POINT;
            name_json = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_POINT_JSON;
            myboard.board_save(name_board, name_txt, name_json, r_size, c_size);

            // Save board: Tests
            myboard.test_create (0, 0.10, 0.20);
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_TEST_S;
            name_json = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_TEST_S_JSON;
            myboard.test_save(name_txt, name_json);

            myboard.test_create (1, 0.25, 0.35);
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_TEST_M;
            name_json = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_TEST_M_JSON;
            myboard.test_save(name_txt, name_json);

            myboard.test_create (2, 0.40, 0.50);
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_TEST_L;
            name_json = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_TEST_L_JSON;
            myboard.test_save(name_txt, name_json);

            // Save board: Statistics
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_STAT;
            myboard.stat_save(name_txt, r_size, c_size);

            // Save board: Image Side A and B
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_IMG_A;
            myboard.img_save(name_txt, 0, r_size, c_size);
            name_txt = todaycpp + '/' + BOARD_NAME + s + '/' + BOARD_IMG_B;
            myboard.img_save(name_txt, 1, r_size, c_size);

            i++;
        }
    }

    //
    // Board to Point Vectors
    //

    return EXIT_SUCCESS;
}

vector<component> components_read () {
    vector<component> components;
    vector<string> tmp;
    string s1, s2;
    bool first;

    for (auto e:file_name_input) {
        if (e.count_max==(-1))
            continue;

        ifstream input_file(e.file_name);
        if (!input_file.is_open()) {
            cerr << "Could not open the file " << e.file_name << endl;
            exit (EXIT_FAILURE);
        }

        first = true;
        while (input_file >> s1) {
            if (first) {
                first = false;
                continue;
            }
            if (s1!=".c") {
                tmp.push_back(s1);
            } else {
                tmp2comp (tmp, components, e.count_max);
            }
        }
        tmp2comp (tmp, components, e.count_max);
        
        input_file.close();
    }

    return (components);
}

void tmp2comp (vector<string> &tmp, vector<component> &components, int count_max) {
    int i, j, r, c;

    r = tmp.size();
    c = tmp[0].size();

    component comp(r,c);
        
    for (i=0; i<r; i++) {
        for (j=0; j<c; j++) {
            comp.side[i][j] = tmp[i][j]-'0';
            if (comp.side[i][j]==PIN) {
                comp.pin_n++;
            } else if (comp.side[i][j]==GND) {
                comp.gnd_n++;
            } else if (comp.side[i][j]==VDD) {
                comp.vdd_n++;
            } else if (comp.side[i][j]==CLOCK) {
                comp.clock_n++;
            } else if (comp.side[i][j]==NOFLY) {
                comp.nofly_n++;
            } else if (comp.side[i][j]==NOTOUCH) {
                comp.notouch_n++;
            }
        }
    }

    tmp.clear();

    if (comp.notouch_n!=0) {
        comp.type = NOTOUCH;
    } else if (comp.nofly_n!=0) {
        comp.type = NOFLY;
    } else if (comp.pin_n!=0 || comp.gnd_n!=0 || comp.vdd_n!=0 || comp.clock_n!=0) {
        comp.type = POINT; 
    } else {
        comp.type = BACKGROUND;
    }
 
    comp.count_max = count_max;
    components.push_back(comp);
    
    return;
}

void create_board (board &myboard, vector<component> components , int r_size, int c_size) {
    int rn, direction;
    bool flag, ok;

    rn = (CHECK_COMPONENT==1 || CHECK_COMPONENT_AND_EXIT==1) ?
         0 : myboard.my_rand_i (0, components.size()-1);

    for (int i=0; i<2; i++) {
        flag = true;
        for (int j=0; j<r_size && flag; j++) {
            for (int k=0; k<c_size && flag; k++) {
                // Modify the board (component) density
                if (myboard.my_rand_f(0,1)>DENSITY) {
                    continue;
                }
                direction = check (myboard, components[rn], r_size, c_size, i, j, k);
                if (direction==0 || direction==1) {
                    // Insert component
                    insert (myboard, components[rn], direction, i, j, k);
                    if (components[rn].type==BACKGROUND) {
                        myboard.component_background_n[i]++;
                    } else if (components[rn].type==POINT) {
                        myboard.component_point_n[i]++;
                    } else
                         if (components[rn].type==NOFLY) {
                        myboard.component_nofly_n[i]++;
                    } else
                         if (components[rn].type==NOTOUCH) {
                        myboard.component_notouch_n[i]++;
                    }
                    myboard.pin_n[i] += components[rn].pin_n;
                    myboard.gnd_n[i] += components[rn].gnd_n;
                    myboard.vdd_n[i] += components[rn].vdd_n;
                    myboard.clock_n[i] += components[rn].clock_n;
                    myboard.nofly_n[i] += components[rn].nofly_n;
                    myboard.notouch_n[i] += components[rn].notouch_n;

                    do {
                        rn = (CHECK_COMPONENT==1 || CHECK_COMPONENT_AND_EXIT==1) ?
                             ((rn+1)%components.size()) : (myboard.my_rand_i_rep (rn, 0, components.size()-1));
                        ok = (components[rn].count_max==0 || components[rn].count<components[rn].count_max) ? true : false;
                        //if (!ok) cout << "[1]";
                    } while (!ok);

                    //cout << "[" << rn << "]";
                    if (CHECK_COMPONENT_AND_EXIT==1 && rn==0)
                        flag=false;
                }
            }
        }
    }

    for (int i=0; i<2; i++) {
        myboard.point_n[i] = myboard.pin_n[i] + myboard.gnd_n[i] +
          myboard.vdd_n[i] + myboard.clock_n[i] + myboard.nofly_n[i] +
          myboard.notouch_n[i];
    }

    myboard.component_background_n[2] = myboard.component_background_n[0] + myboard.component_background_n[1];
    myboard.component_point_n[2] = myboard.component_point_n[0] + myboard.component_point_n[1];
    myboard.component_nofly_n[2] = myboard.component_nofly_n[0] + myboard.component_nofly_n[1];
    myboard.component_notouch_n[2] = myboard.component_notouch_n[0] + myboard.component_notouch_n[1];
    myboard.pin_n[2] = myboard.pin_n[0] + myboard.pin_n[1];
    myboard.gnd_n[2] = myboard.gnd_n[0] + myboard.gnd_n[1];
    myboard.vdd_n[2] = myboard.vdd_n[0] + myboard.vdd_n[1];
    myboard.clock_n[2] = myboard.clock_n[0] + myboard.clock_n[1];
    myboard.nofly_n[2] = myboard.nofly_n[0] + myboard.nofly_n[1];
    myboard.notouch_n[2] = myboard.notouch_n[0] + myboard.notouch_n[1];
    myboard.point_n[2] = myboard.point_n[0] + myboard.point_n[1];

    return;
}

int check (board &myboard, component comp, int r_size, int c_size, int i, int j, int k) {
    bool flag;
    int dj, dk, ii, jj, kk, tmp, direction;

    if (myboard.my_rand_f(0.0,1.0) > DIRECTION) {
        dj = comp.r;
        dk = comp.c;
        direction = 0;
    } else {
        dj = comp.c;
        dk = comp.r;
        direction = 1;
    }
    for (ii=0; ii<2; ii++) {
        flag = true;
        if (j+dj<=r_size && k+dk<=c_size) {
            for (jj=j; jj<j+dj && flag; jj++) {
                for (kk=k; kk<k+dk && flag; kk++) { 
                    if (myboard.side[i][jj][kk]!=BACKGROUND) {
                        flag = false;
                    }
                }
            }
            if (flag==true) {
                return (direction);
            }
        }
        if (DIRECTION==0 || DIRECTION==1)
            return 2;
        direction = 1 - direction;
        tmp = dj; dj = dk; dk = tmp;
    }

    return (2);
}

void insert (board &myboard, component &comp, int direction, int i, int j, int k) {
    int dj, dk, jj, kk;

    comp.count++;
    if (direction==0) {
        dj = comp.r;
        dk = comp.c;
        for (jj=0; jj<dj; jj++) {
            for (kk=0; kk<dk; kk++) {
                myboard.side[i][j+jj][k+kk] = comp.side[jj][kk];
            }
        }
    } else {
        dj = comp.c;
        dk = comp.r;
        for (jj=0; jj<dj; jj++) {
            for (kk=0; kk<dk; kk++) {
                //cout << "[" << j+jj << "," << k+kk << "--" << jj << "," << dk-kk-1 << "]" << endl; 
                myboard.side[i][j+jj][k+kk] = comp.side[dk-kk-1][jj];
            }
        }
    }

    return;
}
