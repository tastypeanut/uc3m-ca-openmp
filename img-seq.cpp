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
//#include <math.h>
/*test*/
namespace fs = std::filesystem;
using namespace std;


//Variables for Gaussian Blur
const int m[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
const int w = 273;
//Variables for Sobel operator

const int mx[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
const int my[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
const int wsobel = 8;

auto start_load = chrono::steady_clock::now();
auto end_load = chrono::steady_clock::now();
auto start_store = chrono::steady_clock::now();
auto end_store = chrono::steady_clock::now();
auto start_gauss = chrono::steady_clock::now();
auto end_gauss = chrono::steady_clock::now();
auto start_sobel = chrono::steady_clock::now();
auto end_sobel = chrono::steady_clock::now();
auto end_total = chrono::steady_clock::now();


bool store(char *file_buffer, int file_size_int, ofstream& output_stream);

bool gauss (char *file_buffer, int file_size_int);

bool sobel(char *file_buffer, int file_size_int);

void printTime(string operation, string inpath);

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

/*Opening directory and checking bmp files*/
struct dirent *file;
while((file = readdir(idir)) != NULL){
		start_load = chrono::steady_clock::now();


		string inpath = indir+"/"+file->d_name;
		string outpath = outdir+"/"+file->d_name;

		ifstream input_stream(inpath, ios::binary);

		if (input_stream.is_open() && inpath != indir+"/"+"." && inpath != indir+"/"+"..")
		{
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
							gauss(file_buffer, file_size_int);

						}
						if(operation == "sobel"){

							start_gauss = chrono::steady_clock::now();
							gauss(file_buffer, file_size_int);

							start_sobel = chrono::steady_clock::now();
							sobel(file_buffer, file_size_int);

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
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 | (unsigned char)file_buffer[22]);

	file_buffer[0] = 'B';
	file_buffer[1] = 'M';

	//Total size
	file_buffer[5] = (/*(binary)*/file_size_int >> 24) & 0xFF;
	file_buffer[4] = (/*(binary)*/file_size_int >> 16) & 0xFF;
	file_buffer[3] = (/*(binary)*/file_size_int >> 8) & 0xFF;
	file_buffer[2] = (/*(binary)*/file_size_int) & 0xFF;

	//Sets to 0 everything that needs to be 0
	for(int i = 0; i <= 3; i++){
		file_buffer[i+6] = 0;
		file_buffer[i+30] = 0;
		file_buffer[i+46] = 0;
		file_buffer[i+50] = 0;
	}

	//Value 54
	file_buffer[13] = (/*(binary)*/54 >> 24) & 0xFF;
	file_buffer[12] = (/*(binary)*/54 >> 16) & 0xFF;
	file_buffer[11] = (/*(binary)*/54 >> 8) & 0xFF;
	file_buffer[10] = (/*(binary)*/54) & 0xFF;

	//Value 40
	file_buffer[17] = (/*(binary)*/40 >> 24) & 0xFF;
	file_buffer[16] = (/*(binary)*/40 >> 16) & 0xFF;
	file_buffer[15] = (/*(binary)*/40 >> 8) & 0xFF;
	file_buffer[14] = (/*(binary)*/40) & 0xFF;

	//Numero de píxeles de ancho aqui
	//Numero de píxeles de alto aqui

	//Value 1
	file_buffer[27] = (/*(binary)*/1 >> 8) & 0xFF;
	file_buffer[26] = (/*(binary)*/1) & 0xFF;

	//Value 24
	file_buffer[27] = (/*(binary)*/24 >> 8) & 0xFF;
	file_buffer[26] = (/*(binary)*/24) & 0xFF;

	//Size of the image
	file_buffer[37] = (/*(binary)*/ width*height*3 >> 24) & 0xFF;
	file_buffer[36] = (/*(binary)*/ width*height*3 >> 16) & 0xFF;
	file_buffer[35] = (/*(binary)*/ width*height*3 >> 8) & 0xFF;
	file_buffer[34] = (/*(binary)*/ width*height*3) & 0xFF;

	//Value 2835
	file_buffer[41] = (/*(binary)*/ 2835 >> 24) & 0xFF;
	file_buffer[40] = (/*(binary)*/ 2835 >> 16) & 0xFF;
	file_buffer[39] = (/*(binary)*/ 2835 >> 8) & 0xFF;
	file_buffer[38] = (/*(binary)*/ 2835) & 0xFF;

	//Value 2835
	file_buffer[45] = (/*(binary)*/ 2835 >> 24) & 0xFF;
	file_buffer[44] = (/*(binary)*/ 2835 >> 16) & 0xFF;
	file_buffer[43] = (/*(binary)*/ 2835 >> 8) & 0xFF;
	file_buffer[42] = (/*(binary)*/ 2835) & 0xFF;

	if(output_stream.write(file_buffer, file_size_int)){
		end_store = chrono::steady_clock::now();
		return true;
	}
	return false;
}

bool gauss (char *file_buffer, int file_size_int){
	int first_byte = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);

	char * res = new char [file_size_int];

	memcpy(res, file_buffer, file_size_int);

	for(int i = 0; i < height; i++){
		for(int j = 0; j < width*3; j+=3){
				int x1 = 0, x2 = 0, x3 = 0;
				for(int s = -2; s <= 2; s++){
					for(int t = -2; t <= 2; t++){
						if(((i+s)*(width*3) + j + t*3 + first_byte) < height*width*3 && ((i+s)*(width*3) + j + t*3 + first_byte) >= first_byte){
							x1 += m[s+2][t+2] * int((unsigned char)(file_buffer[(i+s)*(width*3) + j + t*3 + first_byte]));
							x2 += m[s+2][t+2] * int((unsigned char)(file_buffer[(i+s)*(width*3) + j + t*3 + first_byte + 1]));
							x3 += m[s+2][t+2] * int((unsigned char)(file_buffer[(i+s)*(width*3) + j + t*3 + first_byte + 2]));
						}
					}
				}

				res[i*width*3 + j + first_byte] = x1/w;
				res[i*width*3 + j + first_byte + 1] = x2/w;
				res[i*width*3 + j + first_byte + 2] = x3/w;
		}
	}

	memcpy(file_buffer, res, file_size_int);

	end_gauss = chrono::steady_clock::now();
	return true;
}


bool sobel(char *file_buffer, int file_size_int){
	int first_byte = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);

	char *res = new char[file_size_int];
	memcpy(res, file_buffer, file_size_int);
	char *resx = new char[file_size_int];
	char *resy = new char[file_size_int];

	for(int i = 0; i < height; i++){
		for(int j = 0; j < width*3; j+=3){

				int x1 = 0, x2 = 0, x3 = 0, y1 = 0, y2 = 0, y3 = 0;

				for(int s = -1; s <= 1; s++){
					for(int t = -1; t <= 1; t++){
							x1 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte]));
							x2 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte + 1]));
							x3 += mx[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte + 2]));
					}
				}

				resx[i*width*3 + j + first_byte] = x1/wsobel;
				resx[i*width*3 + j + first_byte + 1] = x2/wsobel;
				resx[i*width*3 + j + first_byte + 2] = x3/wsobel;

				for(int s = -1; s <= 1; s++){
					for(int t = -1; t <= 1; t++){
							y1 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte]));
							y2 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte + 1]));
							y3 += my[s+1][t+1] * int((unsigned char)(res[(i+s)*(width*3) + j + t*3 + first_byte + 2]));
					}
				}

				resy[i*width*3 + j + first_byte] = y1/wsobel;
				resy[i*width*3 + j + first_byte + 1] = y2/wsobel;
				resy[i*width*3 + j + first_byte + 2] = y3/wsobel;

				file_buffer[i*width*3 + j + first_byte] = abs(resx[i*width*3 + j + first_byte]) + abs(resy[i*width*3 + j + first_byte]);
				file_buffer[i*width*3 + j + first_byte + 1] = abs(resx[i*width*3 + j + first_byte + 1]) + abs(resy[i*width*3 + j + first_byte + 1]);
				file_buffer[i*width*3 + j + first_byte + 2] = abs(resx[i*width*3 + j + first_byte + 2]) + abs(resy[i*width*3 + j + first_byte + 2]);

		}
	}

	end_sobel = chrono::steady_clock::now();
	return true;
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
