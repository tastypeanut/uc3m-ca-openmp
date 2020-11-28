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

int getFileSize(ifstream & input_stream);

int main(int argc, char *argv[]){

	friend int getFileSize(ifstream& input_stream);

	auto start = chrono::steady_clock::now();

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

	auto img1 = chrono::steady_clock::now();
	cout << "Time in microseconds : " << chrono::duration_cast<chrono::microseconds>(img1 - start).count() << " microseconds \n";


/*Opening directory and checking bmp files*/
struct dirent *file;
while((file = readdir(idir)) != NULL){
		string inpath = indir+"/"+file->d_name;
		string outpath = outdir+"/"+file->d_name;

		ifstream input_stream( inpath, ios::binary );
  	ofstream output_stream( outpath, ios::binary );
		int file_size = getFileSize(&input_stream);
		char *file_buffer = new char[file_size]; //Cambiar

		if (input_stream.is_open() && output_stream.is_open())
		{
				input_stream.seekg(0, ios::beg);
				input_stream.getline(file_buffer, file_size); //Cambiar

				if (int(file_buffer[26]) == 1 && int(file_buffer[27]) == 0 && int(file_buffer[28]) == 24 && int(file_buffer[29]) == 0 && int(file_buffer[30]) == 0 &&
				int(file_buffer[31]) == 0 && int(file_buffer[32]) == 0 && int(file_buffer[33]) == 0){
					cout << "valid BMP file";
					if(operation == "copy"){

					}
					if(operation == "gauss"){

					}
					if(operation == "sobel"){

					}
				}
		}

		//output_stream.write(file_buffer, sizeof(file_buffer));

		output_stream.close();
		input_stream.close();
	}
	return 0;
}

int getFileSize(ifstream & input_stream){
	input_stream.seekg(0, ios::end);
	return input_stream.tellg();
}



/*
int copy (){

}

int gauss (){

}

int sobel(){

}*/
