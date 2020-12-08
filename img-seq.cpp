#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <chrono>
#include <sys/types.h>
#include <stdlib.h>
#include <iterator>
#include <sstream>
#include <cstring>
#include <cmath>

using namespace std;


//Constants for Gaussian Blur
const int m[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
const int w = 273;

//Constants for Sobel operator
const int mx[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
const int my[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
const int wsobel = 8;

// These variables are declared as global so that we do not have to pass them as arguments to functions outside of main
auto start_load = chrono::steady_clock::now();
auto end_load = chrono::steady_clock::now();
auto start_store = chrono::steady_clock::now();
auto end_store = chrono::steady_clock::now();
auto start_gauss = chrono::steady_clock::now();
auto end_gauss = chrono::steady_clock::now();
auto start_sobel = chrono::steady_clock::now();
auto end_sobel = chrono::steady_clock::now();
auto end_total = chrono::steady_clock::now();
int file_size_int_new = 0;

bool store(char *file_buffer, int file_size_int, ofstream& output_stream);

char* gauss (char *file_buffer, int file_size_int);

char* sobel(char *file_buffer, int file_size_int);

void printTime(string operation, string inpath);

int main(int argc, char *argv[]){

	//Checking the number of arguments is correct
	if(argc != 4){
		cout << "Wrong format: \n image-seq operation in_path out_path \n operation: copy, gauss, sobel \n";
		return 0;
	}

	//Storing the arguments
	string operation = argv[1];
	string indir = argv[2];
	string outdir = argv[3];

	//Checking that the first argument is a valid operation
	if(operation != "copy" && operation != "gauss" && operation != "sobel"){
		cout << "Unexpected operation: " << argv[1] << "\n image-seq operation in_path out_path \n operation: copy, gauss, sobel \n";
		return 0;
	}

	//Opening the input and output directories
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


struct dirent *file;
//This loop reads the files in the input directory one by one and performs the necessary operation
while((file = readdir(idir)) != NULL){
		//The loading of the file starts here
		start_load = chrono::steady_clock::now();

		string inpath = indir+"/"+file->d_name;
		string outpath = outdir+"/"+file->d_name;

		//Opening the input stream of the corresponding file
		ifstream input_stream(inpath, ios::binary);

		//Checking that the input stream is open and it corresponds to a file
		if (input_stream.is_open() && inpath != indir+"/"+"." && inpath != indir+"/"+"..")
		{
				//Getting the size of the file
				int file_size_int = filesystem::file_size(inpath);


				char *file_buffer = new char[file_size_int];

				input_stream.seekg(0, ios::beg);
				int i = 0;
				while (input_stream.get(file_buffer[i])){
					i++;
				}

				end_load = chrono::steady_clock::now();

				if (int(file_buffer[26]) == 1 && int(file_buffer[27]) == 0 && int(file_buffer[28]) == 24 && int(file_buffer[29]) == 0 && int(file_buffer[30]) == 0 &&
				int(file_buffer[31]) == 0 && int(file_buffer[32]) == 0 && int(file_buffer[33]) == 0){
					ofstream output_stream(outpath, ios::binary);

					if(output_stream.is_open()){
						if(operation == "gauss"){

							start_gauss = chrono::steady_clock::now();
							file_buffer = gauss(file_buffer, file_size_int);

						}
						if(operation == "sobel"){

							start_gauss = chrono::steady_clock::now();
							file_buffer = gauss(file_buffer, file_size_int);

							start_sobel = chrono::steady_clock::now();
							file_buffer = sobel(file_buffer, file_size_int);

						}

						start_store = chrono::steady_clock::now();

						store(file_buffer, file_size_int, output_stream);

						output_stream.close();
					}
					end_total = chrono::steady_clock::now();
					printTime(operation, inpath);
				}
				input_stream.close();
		}
	}
	return 0;
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



	memcpy(&res[0], &file_buffer[0], first_byte_new);

	if(padding != 0){
			for(int i = 0; i < height; i++){
				memcpy(&res[first_byte_new + i*width*3 + i*padding], &file_buffer[first_byte_old + i*width*3 + i*3], width*3+padding);
			}
	}else{
	memcpy(&res[first_byte_new], &file_buffer[first_byte_old], file_size_int-first_byte_old);
}

	res[0] = 'B';
	res[1] = 'M';

	//Total size
	res[5] = (file_size_int_new >> 24);
	res[4] = (file_size_int_new >> 16);
	res[3] = (file_size_int_new >> 8);
	res[2] = (file_size_int_new >> 0);


	//Sets to 0 everything that needs to be 0
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
		end_store = chrono::steady_clock::now();
		return true;
	}
	return false;
}

char* gauss (char *file_buffer, int file_size_int){
	int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);
	int padding = 0;
	if((width*3)%4 !=0){
		padding = 4-(width*3)%4;
	}


	char * res = new char [file_size_int];

	memcpy(res, file_buffer, file_size_int);
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width*3; j+=3){
			int x1 = 0, x2 = 0, x3 = 0;

			for(int s = -2; s <= 2; s++){
				for(int t = -2; t <= 2; t++){
					if(((i+s)*(width*3+padding) + j + t*3) < height*(width*3+padding*height) && ((i+s)*(width*3+padding) + j + t*3 + first_byte_old) >= first_byte_old && (j + t*3) < (width*3)
					&& (j + t*3) >= 0){
						x1 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old]);
						x2 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 1]);
						x3 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 2]);
					}
				}
			}

			if(padding == 0){
				res[i*width*3 + j + first_byte_old] = x1/w;
				res[i*width*3 + j + first_byte_old + 1] = x2/w;
				res[i*width*3 + j + first_byte_old + 2] = x3/w;
			} else {
				if((i*width*3 + j + first_byte_old + 2 + i*3) < file_size_int){
					res[i*width*3 + j + first_byte_old + i*3] = x1/w;
					res[i*width*3 + j + first_byte_old + 1 + i*3] = x2/w;
					res[i*width*3 + j + first_byte_old + 2 + i*3] = x3/w;
				}
			}
		}
		for(int h = 0; h < padding; h++){
			res[i*width*3 + width*3 + first_byte_old + i*3] = 0;
		}
	}
	end_gauss = chrono::steady_clock::now();
	return res;
}


char* sobel(char *file_buffer, int file_size_int){
	int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);
	int padding = 0;
	if((width*3)%4 !=0){
		padding = 4-(width*3)%4;
	}


	char *res = new char[file_size_int];
	memcpy(res, file_buffer, file_size_int);
	char *resx = new char[file_size_int];
	char *resy = new char[file_size_int];

	for(int i = 0; i < height; i++){
		for(int j = 0; j < width*3; j+=3){

				int x1 = 0, x2 = 0, x3 = 0, y1 = 0, y2 = 0, y3 = 0;

				for(int s = -1; s <= 1; s++){
					for(int t = -1; t <= 1; t++){
						if(((i+s)*(width*3+padding) + j + t*3) < height*(width*3+padding*height) && ((i+s)*(width*3+padding) + j + t*3 + first_byte_old) >= first_byte_old && (j + t*3) < (width*3)
						&& (j + t*3) >= 0){

							x1 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3+padding) + j + t*3 + first_byte_old]));
							x2 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 1]));
							x3 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 2]));
							y1 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3+padding) + j + t*3 + first_byte_old]));
							y2 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 1]));
							y3 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 2]));
						}
					}
				}
				if(padding == 0){
					resx[i*width*3 + j + first_byte_old] = x1/wsobel;
					resx[i*width*3 + j + first_byte_old + 1] = x2/wsobel;
					resx[i*width*3 + j + first_byte_old + 2] = x3/wsobel;

					resy[i*width*3 + j + first_byte_old] = y1/wsobel;
					resy[i*width*3 + j + first_byte_old + 1] = y2/wsobel;
					resy[i*width*3 + j + first_byte_old + 2] = y3/wsobel;

					file_buffer[i*width*3 + j + first_byte_old] = abs(resx[i*width*3 + j + first_byte_old]) + abs(resy[i*width*3 + j + first_byte_old]);
					file_buffer[i*width*3 + j + first_byte_old + 1] = abs(resx[i*width*3 + j + first_byte_old + 1]) + abs(resy[i*width*3 + j + first_byte_old + 1]);
					file_buffer[i*width*3 + j + first_byte_old + 2] = abs(resx[i*width*3 + j + first_byte_old + 2]) + abs(resy[i*width*3 + j + first_byte_old + 2]);
				} else {
					if((i*width*3 + j + first_byte_old + 2 + i*padding) < file_size_int){
						resx[i*width*3 + j + first_byte_old + i*padding] = x1/wsobel;
						resx[i*width*3 + j + first_byte_old + 1 + i*padding] = x2/wsobel;
						resx[i*width*3 + j + first_byte_old + 2 + i*padding] = x3/wsobel;

						resy[i*width*3 + j + first_byte_old + i*padding] = y1/wsobel;
						resy[i*width*3 + j + first_byte_old + 1 + i*padding] = y2/wsobel;
						resy[i*width*3 + j + first_byte_old + 2 + i*padding] = y3/wsobel;

						file_buffer[i*width*3 + j + first_byte_old + i*padding] = abs(resx[i*width*3 + j + first_byte_old + i*padding]) + abs(resy[i*width*3 + j + first_byte_old + i*padding]);
						file_buffer[i*width*3 + j + first_byte_old + 1 + i*padding] = abs(resx[i*width*3 + j + first_byte_old + 1 + i*padding]) + abs(resy[i*width*3 + j + first_byte_old + 1 + i*padding]);
						file_buffer[i*width*3 + j + first_byte_old + 2 + i*padding] = abs(resx[i*width*3 + j + first_byte_old + 2 + i*padding]) + abs(resy[i*width*3 + j + first_byte_old + 2 + i*padding]);
					}
				}
		}
	}
	end_sobel = chrono::steady_clock::now();

	return file_buffer;
}


void printTime(string operation, string inpath){
	cout << "File: " << inpath <<" (time: " << chrono::duration_cast<chrono::microseconds>(end_total - start_load).count() << ")\n  Load time: " <<
	chrono::duration_cast<chrono::microseconds>(end_load - start_load).count() << "\n";

	if(operation != "copy"){
		cout << "  Gauss time: " << chrono::duration_cast<chrono::microseconds>(end_gauss - start_gauss).count() << "\n";
		if(operation == "sobel"){
			cout << "  Sobel time: " << chrono::duration_cast<chrono::microseconds>(end_sobel - start_sobel).count() << "\n";
		}
	}
	cout << "  Store time: " << chrono::duration_cast<chrono::microseconds>(end_store - start_store).count() << "\n";
}
