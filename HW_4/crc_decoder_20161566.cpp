#include<cstdio>
#include<algorithm>
#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<string>

using namespace std;

FILE *fp1;	// input file
FILE *fp2;	// output file
FILE *fp3;	// result file

int dataword_size; // dataword size
int generator_len; // generator length
string generator;
string input="";
string result="";

int total_word;
int error_word;

void Decoding(char target){
		unsigned char compare = 0x80;
		while(compare>0){
				if((target&compare) != 0)
						input.append("1");
				else
						input.append("0");
				compare >>= 1;
		}
}

void delete_padding(unsigned char num){
		input = input.substr(num);
}

void write_decodefile(){
		int index=0;
		while(index != result.length()){
				unsigned char temp =0;
				unsigned char compare=0x80;
				for(int i=0;i<8;i++){
						if(result[index+i]=='1')
								temp += compare;
						compare >>= 1;
				}
				index += 8;
				fprintf(fp2,"%c",temp);
		}

}


int error_detection(string target){
		string curr = target.substr(0,generator_len);
		int index=0;
		while(index <= target.length() - generator_len){
				if(curr.length() != generator_len)
						curr += target.at(index+generator_len-1);

				if(curr.at(0) == '0'){
						index++;
						curr = curr.substr(1);
						continue;
				}

				for(int i=0;i<generator_len;i++){
						if(curr.at(i) == generator.at(i)){
								curr[i]='0';
						}
						else
								curr[i]='1';
				}
				curr=curr.substr(1);
				index++;
		}
		for(int i=0;i<curr.length();i++){
				if(curr.at(i)=='1'){
						return 1;
				}
		}
		return 0;
}

void dataword_extraction(){
		int granularity = dataword_size + generator_len-1;
		int index=0;
		while(index != input.length()){
				string temp = input.substr(index,granularity);
				error_word += error_detection(temp);
				total_word++;
				result.append(temp.substr(0,dataword_size));
				index += granularity;
		}

}


int main(int argc, char* argv[]){
		char curr;
		bool flag = false;
		unsigned char padding;
		// ---------------------- Exception Handling ---------------------

		if(argc != 6){
				printf("usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
				return -1;
		}

		fp1 = fopen(argv[1],"r");
		fp2 = fopen(argv[2],"w");
		fp3 = fopen(argv[3],"w");

		if(fp1 == NULL){		
				printf("input file open error.\n");
				return -1;
		}

		if(fp2 == NULL){
				printf("output file open error.\n");
				return -1;
		}

		if(fp3 == NULL){
				printf("result file open error.\n");
				return -1;
		}

		dataword_size = atoi(argv[5]);

		if(dataword_size != 4 && dataword_size !=8){
				printf("datawrod size must be 4 or 8.\n");
				return -1;
		}

		generator_len = strlen(argv[4]);
		generator = argv[4];

		// --------------------- Decoding -------------
		while(EOF != fscanf(fp1,"%c",&curr)){
				if(!flag){
						flag=true;
						padding=curr;
						continue;
				}
				Decoding(curr);
		}
		// ---------------------- Delete Padding ----------------------
		delete_padding(padding);
		// ----------------------- Extracting CodeWord  -----------------
		dataword_extraction();
		fprintf(fp3,"%d %d\n",total_word,error_word);
		// ---------------------- Write Decode File ---------------
		write_decodefile();


		fclose(fp1);
		fclose(fp2);
		fclose(fp3);
		return 0;
}

