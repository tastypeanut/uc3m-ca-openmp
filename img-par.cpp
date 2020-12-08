#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <chrono>
#include <sys/types.h>
/*test*/
#include <stdlib.h>
#include <iterator>
#include <sstream>
#include <cstring>
#include <cmath>
#include <omp.h>

using namespace std;
using namespace chrono;
using clk = high_resolution_clock;

// //Class definitions
// class file{
//   public:
//     string inpath;
//     string outpath;
//     int file_size_int = 0;
//     char *file_buffer;
//     int width = 0;
//     int height = 0;
//     int first_byte_old = 0;
// };

//General variables
const int nthread = 8;
const int first_byte_new = 54;

//Variables for Gaussian Blur
const int m[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
const int w = 273;

//Variables for Sobel operator
const int mx[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
const int my[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
const int wsobel = 8;

bool store(char *file_buffer, int file_size_int, ofstream& output_stream);

char* gauss(char *file_buffer, int file_size_int);

char* sobel(char *file_buffer, int file_size_int);

void printTime(string operation, string inpath, int load_time, int gauss_time, int sobel_time, int store_time);

int main(int argc, char *argv[]){

  if(argc != 4){
		cout << "Wrong format: \n image-seq operation in_path out_path \n operation: copy, gauss, sobel \n";
		return 0;
	}

  string operation = argv[1];
	string indir = argv[2];
	string outdir = argv[3];

  if(operation != "copy" && operation != "gauss" && operation != "sobel"){
		cout << "Unexpected operation: " << argv[1] << "\n image-seq operation in_path out_path \n operation: copy, gauss, sobel \n";
		return 0;
	}

  const char *ind = indir.c_str();
	DIR *idir = opendir(ind);

	const char *outd = outdir.c_str();
	DIR *odir = opendir(outd);

	cout << "Input path:" << indir << "\n";
	cout << "Output path:" << outdir << "\n";

  if (!idir){
		/* Input directory does not exist */
		cout << "Cannot open directory [" << indir << "] \n image-seq operation in_path out_path \n operation: copy, gauss, sobel\n";
		return 0;
	} else if (!odir) {
		/* Output directory does not exist*/
		cout << "Output directory [" << outdir << "] does not exist \n image-seq operation in_path out_path \n operation: copy, gauss, sobel\n";
		return 0;
	}

  omp_set_num_threads(nthread);

  #pragma omp parallel
  {
    struct dirent *file;
    //Execution time variables definition
    auto load_start = clk::now(), load_stop = clk::now(), gauss_start = clk::now(), gauss_stop = clk::now(), sobel_start = clk::now(), sobel_stop = clk::now(), store_start = clk::now(), store_stop = clk::now();
    while((file = readdir(idir)) != NULL){
        string inpath = indir+"/"+file->d_name;
		    string outpath = outdir+"/"+file->d_name;
        ifstream input_stream(inpath, ios::binary);
        if (input_stream.is_open() && inpath != indir+"/"+"." && inpath != indir+"/"+"..") {
          int file_size_int = filesystem::file_size(inpath);
          char *file_buffer = new char[file_size_int];
          input_stream.seekg(0, ios::beg);
          int i = 0;
          while (input_stream.get(file_buffer[i])){
            i++;
          }
          if (int(file_buffer[26]) == 1 && int(file_buffer[27]) == 0 && int(file_buffer[28]) == 24 && int(file_buffer[29]) == 0 && int(file_buffer[30]) == 0 &&
  				int(file_buffer[31]) == 0 && int(file_buffer[32]) == 0 && int(file_buffer[33]) == 0){
            // #pragma omp critical
            // cout << "ID: " << int(id) << " file: " << file -> d_name << " size: " << file_size_int << "\n";
            ofstream output_stream(outpath, ios::binary);
            if(output_stream.is_open()){
              load_stop = clk::now();
              if(operation == "gauss"){
                gauss_start = clk::now();
                file_buffer = gauss(file_buffer, file_size_int);
                gauss_stop = clk::now();
              }
              if(operation == "sobel"){
                gauss_start = clk::now();
                file_buffer = gauss(file_buffer, file_size_int);
                gauss_stop = clk::now();
                sobel_start = clk::now();
                file_buffer = sobel(file_buffer, file_size_int);
                sobel_stop = clk::now();
              }
              store_start = clk::now();
              store(file_buffer, file_size_int, output_stream);
              store_stop = clk::now();
              auto load_time = duration_cast<microseconds>(load_stop - load_start);
              auto gauss_time = duration_cast<microseconds>(gauss_stop - gauss_start);
              auto sobel_time = duration_cast<microseconds>(sobel_stop - sobel_start);
              auto store_time = duration_cast<microseconds>(store_stop - store_start);
              printTime(operation, inpath, load_time.count(), gauss_time.count(), sobel_time.count(), store_time.count());
              input_stream.close();
            }
          }
        }
    }

    // #pragma omp critical
    // {
    //   if (file != NULL){
    //     cout << "ID: " << int(id) << " file: " << file -> d_name << "\n";
    //   }
    // }
  }
}

bool store (char *file_buffer, int file_size_int, ofstream& output_stream){
  int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	int first_byte_new = 54;
	int file_size_int_new = file_size_int-abs(first_byte_old-first_byte_new);
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);
	int padding = 0;
	if((width*3)%4 !=0){
		padding = 4-(width*3)%4;
	}
  char * res = new char [file_size_int_new];
  #pragma omp critical
  {
    memcpy(&res[0], &file_buffer[0], first_byte_new);
  }
  if(padding != 0){
    for(int i = 0; i < height; i++){
      #pragma omp critical
      {
        memcpy(&res[first_byte_new + i*width*3 + i*padding], &file_buffer[first_byte_old + i*width*3 + i*3], width*3+padding);
      }
    }
  } else {
    #pragma omp critical
    {
      memcpy(&res[first_byte_new], &file_buffer[first_byte_old], file_size_int-first_byte_old);
    }
  }

  res[0] = 'B';
	res[1] = 'M';

	//Total size
	res[5] = (file_size_int_new >> 24);
	res[4] = (file_size_int_new >> 16);
	res[3] = (file_size_int_new >> 8);
	res[2] = (file_size_int_new >> 0);

	//Sets to 0 everything that needs to be 0
  #pragma omp parallel for
	for(int i = 0; i <= 3; i++){
		res[i+6] = 0;
		res[i+30] = 0;
		res[i+46] = 0;
		res[i+50] = 0;
	}

	//6, 7, 8, 9

	//Value 54
	res[13] = char(first_byte_new >> 24);
	res[12] = char(first_byte_new >> 16);
	res[11] = char(first_byte_new >> 8);
	res[10] = char(first_byte_new >> 0);

	//Value 40
	res[17] = (40 >> 24);
	res[16] = (40 >> 16);
	res[15] = (40 >> 8);
	res[14] = (40 >> 0);

	//Numero de píxeles de ancho aqui 18, 19, 20, 21
	//Numero de píxeles de alto aqui 22, 23, 24, 25

	//Value 1
	res[26] = (1);
	res[27] = (1 >> 8);

	//Value 24
	res[28] = (24);
	res[29] = (24 >> 8);

	//30, 31, 32, 33

	int img_size = file_size_int_new - 54;
	//Size of the image
	res[34] = (img_size);
	res[35] = (img_size >> 8);
	res[36] = (img_size >> 16);
	res[37] = (img_size >> 24);

	//Value 2835
	res[38] = char(2835);
	res[39] = char(2835 >> 8);
	res[40] = char(2835 >> 16);
	res[41] = char(2835 >> 24);

	//Value 2835
	res[42] = char(2835);
	res[43] = char(2835 >> 8);
	res[44] = char(2835 >> 16);
	res[45] = char(2835 >> 24);

	//46, 47, 48, 49
	//50, 51, 52, 53

  if(output_stream.write(res, file_size_int_new)){
		return true;
	}
	return false;
}

char *gauss (char *file_buffer, int file_size_int){
  int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
  int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
  int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);
  int padding = 0;
  if((width*3)%4 !=0){
		padding = 4-(width*3)%4;
	}
  char *res = new char [file_size_int];
  #pragma omp critical
  {
    memcpy(res, file_buffer, file_size_int);
  }
  #pragma omp parallel for collapse(2)
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width*3; j+=3){
      int x1 = 0, x2 = 0, x3 = 0;
      for(int s = -2; s <= 2; s++){
        for(int t = -2; t <= 2; t++){
          if(padding == 0){
            if(((i+s)*(width*3) + j + t*3) < height*width*3 && ((i+s)*(width*3) + j + t*3 + first_byte_old) >= first_byte_old && (j + t*3) < width*3 && (j + t*3) >= 0){
							x1 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3) + j + t*3 + first_byte_old]);
							x2 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3) + j + t*3 + first_byte_old + 1]);
							x3 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3) + j + t*3 + first_byte_old + 2]);
						}
          } else {
            if(((i+s)*(width*3+padding) + j + t*3) < height*(width*3+padding*height) && ((i+s)*(width*3+padding) + j + t*3 + first_byte_old) >= first_byte_old && (j + t*3) < (width*3)
						&& (j + t*3) >= 0){
							x1 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old]);
							x2 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 1]);
							x3 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 2]);
						}
          }
        }
      }
      if(padding == 0){
        res[i*width*3 + j + first_byte_old] = x1/w;
				res[i*width*3 + j + first_byte_old + 1] = x2/w;
				res[i*width*3 + j + first_byte_old + 2] = x3/w;
      } else {
        if(j+i*padding == 0 && i > 0){
          //res[i*width*3 + j + first_byte_old + i*3] = 0;
					//res[i*width*3 + j + first_byte_old + 1 + i*3] = 0;
					//res[i*width*3 + j + first_byte_old + 2 + i*3] = 0;
        } else {
          if((i*width*3 + j + first_byte_old + 2 + i*3) < file_size_int){
            res[i*width*3 + j + first_byte_old + i*3] = x1/w;
  					res[i*width*3 + j + first_byte_old + 1 + i*3] = x2/w;
  					res[i*width*3 + j + first_byte_old + 2 + i*3] = x3/w;
          }
        }
      }
    }
  }
  return res;
}

char *sobel (char *file_buffer, int file_size_int){
  int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);
  char *res = new char[file_size_int];
  #pragma omp critical
  {
    memcpy(res, file_buffer, file_size_int);
  }
  char *resx = new char[file_size_int];
	char *resy = new char[file_size_int];

  #pragma omp parallel for collapse(2)
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width*3; j+=3){
      int x1 = 0, x2 = 0, x3 = 0, y1 = 0, y2 = 0, y3 = 0;
      for(int s = -1; s <= 1; s++){
        for(int t = -1; t <= 1; t++){
          if(((i+s)*(width*3) + j + t*3) < height*width*3 && ((i+s)*(width*3) + j + t*3 + first_byte_old) >= first_byte_old && (j + t*3) < width*3 && (j + t*3) >= 0){
            x1 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte_old]));
            x2 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte_old + 1]));
            x3 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte_old + 2]));
            y1 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte_old]));
            y2 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte_old + 1]));
            y3 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte_old + 2]));
          }
        }
      }
      resx[i*width*3 + j + first_byte_old] = x1/wsobel;
			resx[i*width*3 + j + first_byte_old + 1] = x2/wsobel;
			resx[i*width*3 + j + first_byte_old + 2] = x3/wsobel;

			resy[i*width*3 + j + first_byte_old] = y1/wsobel;
			resy[i*width*3 + j + first_byte_old + 1] = y2/wsobel;
			resy[i*width*3 + j + first_byte_old + 2] = y3/wsobel;

			file_buffer[i*width*3 + j + first_byte_old] = abs(resx[i*width*3 + j + first_byte_old]) + abs(resy[i*width*3 + j + first_byte_old]);
			file_buffer[i*width*3 + j + first_byte_old + 1] = abs(resx[i*width*3 + j + first_byte_old + 1]) + abs(resy[i*width*3 + j + first_byte_old + 1]);
			file_buffer[i*width*3 + j + first_byte_old + 2] = abs(resx[i*width*3 + j + first_byte_old + 2]) + abs(resy[i*width*3 + j + first_byte_old + 2]);
    }
  }
  return file_buffer;
}

void printTime(string operation, string inpath, int load_time, int gauss_time, int sobel_time, int store_time){
  int total_time = load_time + gauss_time + sobel_time + store_time;
  #pragma omp critical
  {
    cout << "File: " << inpath <<" (time: " << total_time  << ")\n  Load time: " << load_time << "\n";
    if(operation != "copy"){
      cout << "  Gauss time: " << gauss_time << "\n";
  		if(operation == "sobel"){
  			cout << "  Sobel time: " << sobel_time << "\n";
  		}
    }
    cout << "  Store time: " << store_time << "\n";
  }
}
