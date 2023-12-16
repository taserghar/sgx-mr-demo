#include <cstdio>
#include <cstring>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#define MAP_READ_RECORD "MAP_READ_RECORD"
#define MAP_WRITE_RECORD "MAP_WRITE_RECORD"
#define MAP_READ_BLOCK "MAP_READ_BLOCK"
#define MAP_WRITE_BLOCK "MAP_WRITE_BLOCK"

#define REDUCE_READ_RECORD "REDUCE_READ_RECORD"
#define REDUCE_WRITE_RECORD "REDUCE_WRITE_RECORD"
#define REDUCE_READ_BLOCK "REDUCE_READ_BLOCK"
#define REDUCE_WRITE_BLOCK "REDUCE_WRITE_BLOCK"

vector<int>map_block_arr;
vector<int>map_record_arr;
vector<int>reduce_block_arr;
vector<int>reduce_record_arr;

vector<int>reduce_record_per_block;



int map_read_block_cnt;
int map_write_block_cnt;
int map_read_record_cnt;
int map_write_record_cnt;

int reduce_read_block_cnt;
int reduce_write_block_cnt;
int reduce_read_record_cnt;
int reduce_write_record_cnt;

double get_rand() {
   srand (rand());
  double r = ((double) rand() / (RAND_MAX));
  // cout<<"rand: "<<r<<endl;
  return r;
}

void process_arr(vector<int>&arr, string tag) {
	cout<<"------------------------"<<tag<<"-----------------------"<<endl;
	int read = 0;
	int write = 0;
	for( int val : arr) {
		if ( val == 1) {
			if( write !=0) {
				cout<<tag<<" , "<<read<<","<<write<<endl;
				read = write = 0;
			}
			read++;
		} else{
			write++;
		}
	}
	if( write !=0) {
		cout<<tag<<" , "<<read<<","<<write<<endl;
		read = write = 0;
	}
}

void process_agg_or_not(vector<int>&arr, string tag) {
	cout<<"------------------------"<<tag<<"-----------------------"<<endl;
	int prev = -1;
	for( int val : arr) {
		// cout<<"V:"<<val<<endl;
		if ( val == 1) {
			if(prev == 1) {
				cout<<"0"<<endl;
			} else {
				cout<<"1"<<endl;
			}
			prev = 1;
		} else{
			prev = 0;
		}
	}
}

void process_randomize_record_per_block(vector<int>&arr, double P1, double P2, string tag) {
	// cout<<"------------------------"<<tag<<"-----------------------"<<endl;
	int tot = 0;
	int record_limit = 300;
	int read_block_cnt = 0;
	int write_record_cnt = 0;
	int prev = -1;
	for( int val : arr) {
		// cout<<"V:"<<val<<endl;
		if( val == 11) {
			read_block_cnt++;
		} else if ( val == 1) { //read_record
			if(prev == 1) { // not aggregated
				if( get_rand() < P1) {
					 // cout<<"dummy" << endl; // dummy record
					if (  get_rand() < P2 ) {
						 write_record_cnt++;
					}
				} else {
					// cout<<"0"<<endl; // no record
				}
			} else { // aggregated
				// cout<<"1"<<endl; //real record
				write_record_cnt++;
			}
			prev = 1;
		} else{ // write record
			prev = 0;
		}

		if(write_record_cnt == record_limit) {
			cout<<read_block_cnt<<endl;
			tot++;
			read_block_cnt = 0;
			write_record_cnt = 0;
		}
	}

	cout<<"TOT: "<<tot<<endl;
}


void process_randomize_sagar(vector<int>&arr, double P1, double P2, string tag) {
	// cout<<"------------------------"<<tag<<"-----------------------"<<endl;
	int tot = 0;
	int record_limit = 300;
	int read_block_cnt = 0;
	int write_record_cnt = 0;
	int prev = -1;
	for( int val : arr) {
		// cout<<"V:"<<val<<endl;
		if( val == 11) {
			read_block_cnt++;
		} else if ( val == 1) { //read_record
			if(prev == 1) { // not aggregated
				if( get_rand() < P1) {
					 // cout<<"dummy" << endl; // dummy record
					if (  get_rand() < P2 ) {
						 write_record_cnt++;
					}
				} else {
					// cout<<"0"<<endl; // no record
				}
			} else { // aggregated
				// cout<<"1"<<endl; //real record
				write_record_cnt++;
			}
			prev = 1;
		} else{ // write record
			prev = 0;
		}

		if(write_record_cnt == record_limit) {
			cout<<read_block_cnt<<endl;
			tot++;
			read_block_cnt = 0;
			write_record_cnt = 0;
		}
	}

	cout<<"TOT: "<<tot<<endl;
}

void process_record_per_block(vector<int>&arr, string tag) {
	// cout<<"------------------------"<<tag<<"-----------------------"<<endl;
	int write = 0;
	int tot_block = 0;
	for( int val : arr) {
		if ( val == 1) {
			cout<<write<<endl;
			write = 0;
			tot_block++;
		} else{
			write++;
		}
	}
	cout<<write<<endl;
	// cout<<"Total Block: "<<tot_block<<endl;
}
int main(){
	std::ifstream infile("output.txt");
	std::string line;
	// block wise count
	int map_in_count = 0;
	int map_out_count = 0;
	int reduce_in_count = 0;
	int reduce_out_count = 0;

	while (std::getline(infile, line))
	{
	    std::istringstream iss(line);
		if (line.find(MAP_READ_BLOCK) != std::string::npos) {
			map_block_arr.push_back(1);
			map_in_count++;
		} else if(line.find(MAP_WRITE_BLOCK) != std::string::npos) {
			map_block_arr.push_back(2);
			map_out_count++;
		} else if (line.find(MAP_READ_RECORD) != std::string::npos) {
			map_record_arr.push_back(1);
		} else if(line.find(MAP_WRITE_RECORD) != std::string::npos) {
			map_record_arr.push_back(2);
		} else if (line.find(REDUCE_READ_BLOCK) != std::string::npos) {
			// reduce_block_arr.push_back(1);
			reduce_record_per_block.push_back(1);
			reduce_record_arr.push_back(11);
			reduce_in_count++;
		} else if(line.find(REDUCE_WRITE_BLOCK) != std::string::npos) {
			reduce_block_arr.push_back(2);

			reduce_out_count++;
		} else if (line.find(REDUCE_READ_RECORD) != std::string::npos) {
			reduce_record_arr.push_back(1);
		} else if(line.find(REDUCE_WRITE_RECORD) != std::string::npos) {
			reduce_record_arr.push_back(2);
			reduce_record_per_block.push_back(2);
		}



	    // process pair (a,b)
	}

	// cout<< "map blocks & records: " << map_block_arr.size()<< " "<< map_record_arr.size()<< endl;
	// cout<< "reduce blocks & record: " << reduce_block_arr.size()<< " "<< reduce_record_arr.size()<< endl;

	// process_arr(map_block_arr, "map_block");
	// process_arr(map_record_arr, "map_record");
	// process_arr(reduce_block_arr, "reduce_block");
	// process_arr(reduce_record_arr, "reduce_record");

	// cout<<"Block wise count:"<<endl;
	// cout<<"Map In Count: "<<map_in_count<<endl;
	// cout<<"Map Out Count: "<<map_out_count<<endl;
	// cout<<"Reduce In Count: "<<reduce_in_count<<endl;
	// cout<<"Reduce Out Count: "<<reduce_out_count<<endl;

	// process_record_per_block(reduce_record_per_block, "reduce_record_per_block");

	// process_agg_or_not(reduce_record_arr, "agg or not");
	process_randomize_record_per_block(reduce_record_arr, 0.5, 0.5,"randomized_record");
	return 0;
}
