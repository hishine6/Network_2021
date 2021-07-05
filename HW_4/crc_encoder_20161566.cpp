#include<cstdio>
#include<algorithm>
#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<string>

using namespace std;

FILE *fp1;	// input file
FILE *fp2;	// output file

int dataword_size; // dataword size
int generator_len; // generator length
string generator;
string result="";


// Generating Dataword from character 
// Char -> String
string dataword_generator(char target){
		string value="";
		unsigned char compare;
		if(dataword_size==4)
				compare=0x08;
		else
				compare=0x80;

		while(compare>0){
				if((target & compare) != 0)
						value.append("1");
				else 
						value.append("0");
				compare >>= 1;
		}	
		return value;
}

// Modulo-2 Division (making codeword)
void codeword_generator(char target){
		string dataword = dataword_generator(target);
		dataword.append(generator_len-1,'0');
		string curr = dataword.substr(0,generator_len);
		int index=0;
		
		while(index <= dataword.length() - generator_len){
				if(curr.length() != generator_len)
					curr += dataword.at(index+generator_len-1);

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
		result.append(dataword.substr(0,dataword_size));
		result.append(curr);
}


int main(int argc, char* argv[]){
		char curr;

		// ------------------ Exception Handling ----------------------
		
		if(argc != 5){
				printf("usage: ./crc_encoder input_file output_file generator dataword_size\n");
				return -1;
		}

		fp1 = fopen(argv[1],"r");
		fp2 = fopen(argv[2],"w");

		if(fp1 == NULL){		
				printf("input file open error.\n");
				return -1;
		}

		if(fp2 == NULL){
				printf("output file open error.\n");
				return -1;
		}

		dataword_size = atoi(argv[4]);

		if(dataword_size != 4 && dataword_size !=8){
				printf("datawrod size must be 4 or 8.\n");
				return -1;
		}

		generator_len = strlen(argv[3]);
		generator = argv[3];

		// --------------------- CodeWord Generating -----------------

		while(EOF != fscanf(fp1,"%c",&curr)){
				if(dataword_size == 4){
						char first = curr & 0xF0;
						char second = curr & 0x0F;
						first >>=4;
						codeword_generator(first);
						codeword_generator(second);
				}
				else{
						codeword_generator(curr);
				}

		}
		// ----------------- Padding ------------------------
		int pad = (8 - result.length()%8)%8;
		string padding = "";
		padding.append(pad,'0');
		padding.append(result);
		result = padding;
	
		// ----------------- Change String to Bit -------------------
		int len = result.length()/8;
		unsigned char temp = pad & 0xFF;
		fprintf(fp2,"%c",temp);
		for(int i=0;i<len;i++){
				unsigned char target = 0;
				unsigned char compare = 0x80;
				for(int j=0;j<8;j++){
						if(result[8*i+j]=='1')
								target |= compare;
						compare >>= 1;
				}
				fprintf(fp2,"%c",target);
		}

		fclose(fp1);
		fclose(fp2);
		return 0;
}

