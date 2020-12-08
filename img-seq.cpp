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

				//file_buffer is an array that stores all the bytes in the opened file
				char *file_buffer = new char[file_size_int];
				input_stream.seekg(0, ios::beg);
				int i = 0;
				while (input_stream.get(file_buffer[i])){
					i++;
				}

				//The loading of the file ends here
				end_load = chrono::steady_clock::now();

				//Checking that the file is valid in accordance to the project statement
				if (int(file_buffer[26]) == 1 && int(file_buffer[27]) == 0 && int(file_buffer[28]) == 24 && int(file_buffer[29]) == 0 && int(file_buffer[30]) == 0 &&
				int(file_buffer[31]) == 0 && int(file_buffer[32]) == 0 && int(file_buffer[33]) == 0){

					//Opening the output stream
					ofstream output_stream(outpath, ios::binary);

					if(output_stream.is_open()){
						//If the operation is not copy, the gaussian blur operation will be performed
						if(operation != "copy"){

							start_gauss = chrono::steady_clock::now();
							file_buffer = gauss(file_buffer, file_size_int);

							//If the operation is equal to sobel, the sobel operator will be used
							if(operation == "sobel"){

								start_sobel = chrono::steady_clock::now();
								file_buffer = sobel(file_buffer, file_size_int);

							}
						}


						start_store = chrono::steady_clock::now();

						store(file_buffer, file_size_int, output_stream);

						//Closing the output stream
						output_stream.close();
					}
					end_total = chrono::steady_clock::now();
					printTime(operation, inpath);
				}
				//Closing the input stream
				input_stream.close();
		}
	}
	return 0;
}

//This function will write the bytes in file_buffer into a new file in the output directory, after changing some information from the header
bool store (char *file_buffer, int file_size_int, ofstream& output_stream){
	//Byte where the values of the pixels start in the original file
	int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	//Byte where the values of the pixels start in the new file
	int first_byte_new = 54;
	//Size of the new file
	int file_size_int_new = file_size_int-abs(first_byte_old-first_byte_new);



	char * res = new char [file_size_int_new];
	//Copying the header of the file
	memcpy(&res[0], &file_buffer[0], first_byte_new);

	//The bytes for the image itself will be copied starting after 54 bytes
	memcpy(&res[first_byte_new], &file_buffer[first_byte_old], file_size_int-first_byte_old);


	//Changing the header to the required values:

	res[0] = 'B';
	res[1] = 'M';

	//Total size
	res[5] = (file_size_int_new >> 24);
	res[4] = (file_size_int_new >> 16);
	res[3] = (file_size_int_new >> 8);
	res[2] = (file_size_int_new >> 0);


	//Value 0
	for(int i = 0; i <= 3; i++){
		res[i+6] = 0;
		res[i+30] = 0;
		res[i+46] = 0;
		res[i+50] = 0;
	}

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

	//Value 1
	res[26] = (1);
	res[27] = (1 >> 8);

	//Value 24
	res[28] = (24);
	res[29] = (24 >> 8);

	//Size of the image
	int img_size = file_size_int_new - 54;
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


	//Writing the file in the input stream
	if(output_stream.write(res, file_size_int_new)){
		end_store = chrono::steady_clock::now();
		return true;
	}
	return false;
}

//Function to perform the gaussian blur operation on an image stored in the file_buffer
char* gauss (char *file_buffer, int file_size_int){
	//Byte where the values of the pixels start in the original file
	int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	//Width and height of the image in pixels
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);

	//The value of the number of bytes that need to be padded in the file so that each line of pixels starts on a word boundary
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
					//Checking that we do not get out of the boundaries of the image
					if(((i+s)*(width*3+padding) + j + t*3) < height*(width*3+padding*height) && ((i+s)*(width*3+padding) + j + t*3 + first_byte_old) >= first_byte_old && (j + t*3) < (width*3)
					&& (j + t*3) >= 0){
						//Each pixel is composed of three bytes, so we use three variables
						x1 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old]);
						x2 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 1]);
						x3 += m[s+2][t+2] * (unsigned char)(file_buffer[(i+s)*(width*3+padding) + j + t*3 + first_byte_old + 2]);
					}
				}
			}

			//Storing the new values
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
		//Padding the file with bytes of value 0
		for(int h = 0; h < padding; h++){
			res[i*width*3 + width*3 + first_byte_old + i*3] = 0;
		}
	}

	if(padding != 0){
		for(int i = 0; i < height; i++){
			//If there is padding, each line will be copied one by one with the padding taken into account
			memcpy(&file_buffer[first_byte_old + i*width*3 + i*padding], &res[first_byte_old + i*width*3 + i*3], width*3+padding);
		}
	}
	end_gauss = chrono::steady_clock::now();
	return file_buffer;
}

//Function to perform the sobel operation on an image stored in the file_buffer
char* sobel(char *file_buffer, int file_size_int){
	//Byte where the values of the pixels start in the original file
	int first_byte_old = int((unsigned char)file_buffer[13] << 24 |  (unsigned char)file_buffer[12] << 16 |  (unsigned char)file_buffer[11] << 8 |(unsigned char)file_buffer[10]);
	//Width and height of the image in pixels
	int width = int((unsigned char)file_buffer[21] << 24  |  (unsigned char)file_buffer[20] << 16 |  (unsigned char)file_buffer[19] << 8 | (unsigned char)file_buffer[18]);
	int height = int((unsigned char)file_buffer[25] << 24 |  (unsigned char)file_buffer[24] << 16 |  (unsigned char)file_buffer[23] << 8 |(unsigned char)file_buffer[22]);

	//The value of the number of bytes that need to be padded in the file so that each line of pixels starts on a word boundary
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
						//Checking that we do not get out of the boundaries of the image
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

					//Storing the new values obtained with the operation
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

						//Storing the new values obtained with the operation
						file_buffer[i*width*3 + j + first_byte_old + i*padding] = abs(resx[i*width*3 + j + first_byte_old + i*padding]) + abs(resy[i*width*3 + j + first_byte_old + i*padding]);
						file_buffer[i*width*3 + j + first_byte_old + 1 + i*padding] = abs(resx[i*width*3 + j + first_byte_old + 1 + i*padding]) + abs(resy[i*width*3 + j + first_byte_old + 1 + i*padding]);
						file_buffer[i*width*3 + j + first_byte_old + 2 + i*padding] = abs(resx[i*width*3 + j + first_byte_old + 2 + i*padding]) + abs(resy[i*width*3 + j + first_byte_old + 2 + i*padding]);
					}
				}
		}

		//Padding the file with bytes of value 0
		for(int h = 0; h < padding; h++){
			res[i*width*3 + width*3 + first_byte_old + i*3] = 0;
		}
	}
	end_sobel = chrono::steady_clock::now();

	return file_buffer;
}

//This function prints the time that a file took to perform the different operations of the program
void printTime(string operation, string inpath){
	//First it will print the total time for all the operations and the loading time
	cout << "File: " << inpath <<" (time: " << chrono::duration_cast<chrono::microseconds>(end_total - start_load).count() << ")\n  Load time: " <<
	chrono::duration_cast<chrono::microseconds>(end_load - start_load).count() << "\n";

	//If the operation is not copy it will print the gauss time, and if it is sobel it will print the sobel time as well
	if(operation != "copy"){
		cout << "  Gauss time: " << chrono::duration_cast<chrono::microseconds>(end_gauss - start_gauss).count() << "\n";
		if(operation == "sobel"){
			cout << "  Sobel time: " << chrono::duration_cast<chrono::microseconds>(end_sobel - start_sobel).count() << "\n";
		}
	}
	//Finally it will print the time to store the file
	cout << "  Store time: " << chrono::duration_cast<chrono::microseconds>(end_store - start_store).count() << "\n";
}
