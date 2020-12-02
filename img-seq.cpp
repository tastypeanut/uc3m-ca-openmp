#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <chrono>
#include <sys/types.h>
/*test*/
#include <iterator>
#include <sstream>
#include <cstring>
/*test*/
using namespace std;
/*
//Variables for Gaussian Blur
const int m[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
const int w = 273;
//Variables for Sobel operator
const int mx[5][5] = {{1,2,1},{0,0,0},{-1,-2,-1}};
const int my[5][5] = {{-1,0,1},{-2,0,2},{-1,0,1}};
*/

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

int sobel();

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
				//cout << file->d_name << " has a size of " << file_size_int << " ";
				char *file_buffer = new char[file_size_int];

				input_stream.seekg(0, ios::beg);
				int i = 0;
				while (input_stream.get(file_buffer[i])){
					i++;
				}

				end_load = chrono::steady_clock::now();

				if (int(file_buffer[26]) == 1 && int(file_buffer[27]) == 0 && int(file_buffer[28]) == 24 && int(file_buffer[29]) == 0 && int(file_buffer[30]) == 0 &&
				int(file_buffer[31]) == 0 && int(file_buffer[32]) == 0 && int(file_buffer[33]) == 0){
					//cout << "and is a valid BMP file \n";

					if(operation == "copy"){
						ofstream output_stream(outpath, ios::binary);
						start_store = chrono::steady_clock::now();

						if (output_stream.is_open()){
							store(file_buffer, file_size_int, output_stream);
						}

						end_store = chrono::steady_clock::now();
						output_stream.close();
					}
					if(operation == "gauss"){
						ofstream output_stream(outpath, ios::binary);
						if (output_stream.is_open()){

							gauss(file_buffer, file_size_int);

							store(file_buffer, file_size_int, output_stream);
						}
						output_stream.close();
					}
					if(operation == "sobel"){
						ofstream output_stream(outpath, ios::binary);
						if (output_stream.is_open()){

						}
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
	if(output_stream.write(file_buffer, file_size_int)){
		return true;
	}
	return false;
}

bool gauss (char *file_buffer, int file_size_int){
	int first_byte = int((unsigned char)file_buffer[10] |  (unsigned char)file_buffer[11] |  (unsigned char)file_buffer[12] |(unsigned char)file_buffer[13]);
	int width = int((unsigned char)file_buffer[18] |  (unsigned char)file_buffer[19] |  (unsigned char)file_buffer[20] |(unsigned char)file_buffer[21]);
	int height = int((unsigned char)file_buffer[22] |  (unsigned char)file_buffer[23] |  (unsigned char)file_buffer[24] |(unsigned char)file_buffer[25]);

	cout << width << " " << height << "\n";

	file_buffer[first_byte] = char(0); //Blue
	file_buffer[first_byte + 1] = char(0); //Green
	file_buffer[first_byte + 2] = char(255); //Red

	char * res = new char [file_size_int];

	memcpy(res, file_buffer, file_size_int);

	cout << int((unsigned char) res[first_byte + 2]) << " " << int((unsigned char) file_buffer[first_byte + 2]) << "\n";


	// int i = 0, j = 0;
	// file_buffer[i*width*3 + j + first_byte];
	// file_buffer[i*width*3 + j + first_byte + 1];
	// file_buffer[i*width*3 + j + first_byte + 2]



	// cout << "Blue: " << int(file_buffer[i*width*3 + j + first_byte]) << " Green: " << int(file_buffer[i*width*3 + j + first_byte + 1]) << " Red: " << int((unsigned char) file_buffer[i*width*3 + j + first_byte + 2]);
	//
	// cout << " " << int(i*width*3 + j + first_byte) << "  " << first_byte;

	return true;

}

/*
int sobel(){

}*/

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
