#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <chrono>
using namespace std;

//Variables for Gaussian Blur
const int m[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
const int w = 273;
//Variables for Sobel operator
const int mx[5][5] = {{1,2,1},{0,0,0},{-1,-2,-1}};
const int my[5][5] = {{-1,0,1},{-2,0,2},{-1,0,1}};

int main(int argc, char *argv[]){

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
	/* Here goes the hard stuff */



	return 0;
}
