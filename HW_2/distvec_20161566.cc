#include<stdio.h>
#include<iostream>
#include<queue>
#include<vector>
#include<string.h>

using namespace std;

#define MAX_NODE 100
#define MAX_COST 100
#define DISCONNECTED -999
#define THRESHOLD 9901
#define MAX_MESSAGE 1000

int n;	//node number
FILE *fp1,*fp2,*fp3,*fout;

queue<int> update;
typedef struct _Node{
		pair<int, int> rout_table[MAX_NODE];	// Routing Table
		vector< pair<int,int> > neighbor;	// Neighbor Nodes
}Node;
Node node[MAX_NODE];	

void Simulate(){
		fseek(fp2,0,SEEK_SET);	// Set Read pointer to start of file
		char temp[MAX_MESSAGE];
		char message[MAX_MESSAGE];
		while(fgets(temp,MAX_MESSAGE,fp2)!=NULL){
				int start,end,curr,cost;
				int pos=3;
				if(strlen(temp)==1)
						continue;
				sscanf(temp,"%d %d",&start,&end);
				if(start>=10)
						pos++;
				if(end>=10)
						pos++;
				strcpy(message,temp+pos);
				
				queue<int> hops;
				curr=start;
				cost=node[start].rout_table[end].second;

				// Do simulations
				if(node[curr].rout_table[end].first==-1){
						fprintf(fout,"from %d to %d cost infinite hops ureachable message%s",start,end,message);
				}
				else{
						while(curr != end){
								hops.push(curr);
								curr = node[curr].rout_table[end].first;
						}
						// Print simulations

						fprintf(fout,"from %d to %d ",start,end);
						fprintf(fout,"cost %d hops ",cost);
						while(!hops.empty()){
								fprintf(fout, "%d ",hops.front());
								hops.pop();
						}
						fprintf(fout, "message%s",message);
				}
		}
		fprintf(fout,"\n");
}

void Print_Routing_Tables(){
		for(int i=0;i<n;i++){
				for(int j=0;j<n;j++){
						if(node[i].rout_table[j].first==-1)
								continue;
						fprintf(fout,"%d %d %d\n",j, node[i].rout_table[j].first, node[i].rout_table[j].second);
				}
				fprintf(fout,"\n");
		}
}

void Update_Routing_Table(){
		// Trade Routing Tables with adjacent node

		while(!update.empty()){
				bool modified=false;
				int i = update.front();	// Node that sends Routing Table
				int num = node[i].neighbor.size();
				update.pop();

				for(int j=0;j<num;j++){
						int next = node[i].neighbor[j].first;	// for each neighbor node
						int cost = node[i].neighbor[j].second;	// cost to next neighbor node

						for(int k=0;k<n;k++){
								int new_cost = cost;
								int prev_cost = node[i].rout_table[k].second;
								
								if(node[next].rout_table[k].first==-1){	// there is no route(i-->next-->k)
										if(node[i].rout_table[k].first==next){
												node[i].rout_table[k]=make_pair(-1,DISCONNECTED);
												modified=true;
										}
										continue;
								}
								new_cost += node[next].rout_table[k].second; // there is route(i-->next + next-->k)
								if(new_cost > THRESHOLD){
										node[i].rout_table[k]=make_pair(-1,DISCONNECTED);
										modified=true;
										continue;
								}
								//printf("curr node: %d, neighbor: %d, destination: %d, prev_cost: %d, next_cost: %d\n",i,next,k,prev_cost,new_cost);

								// Update Cases
								// 1. new route is more cheap
								// 2. there was no route in the first place
								// 3. If the Received Routing table is from the 'next'

								if(new_cost < prev_cost || prev_cost == DISCONNECTED){
										node[i].rout_table[k].first = next;
										node[i].rout_table[k].second = new_cost;
										modified=true;	

								}
								else if(new_cost == prev_cost){ //Tie-breaking Rule 1 apply
										if(node[i].rout_table[k].first > next){
												node[i].rout_table[k].first=next;
												modified=true;
										}
								}
								else{
										if(node[i].rout_table[k].first == next){ // If previous Route passes next, it should be updated no matter what!
												node[i].rout_table[k].first = next;
												node[i].rout_table[k].second = new_cost;
												modified=true;

										}
								}
						}
				}
				if(modified){
						for(int j=0;j<num;j++){
								update.push(node[i].neighbor[j].first);
						}
				}
		}
}

int main(int argc, char*argv[]){
		int temp1,temp2,temp3;

		if(argc!=4){
				fprintf(stderr,"usage: distvec topologyfile messagesfile changesfile\n");
				return 1;
		}
		fp1=fopen(argv[1],"r");
		fp2=fopen(argv[2],"r");
		fp3=fopen(argv[3],"r");
		fout=fopen("output_dv.txt","w");

		if(fp1==NULL || fp2==NULL || fp3==NULL){
				fprintf(stderr,"Error: open input file.\n");
				return 1;
		}

		// STEP 0

		// 0-1) Make Initial Routing Table
		fscanf(fp1,"%d",&n);
		for(int i=0;i<n;i++){
				for(int j=0;j<n;j++){
						if(i==j)	
								node[i].rout_table[j]=make_pair(i,0);
						else	
								node[i].rout_table[j]=make_pair(-1,DISCONNECTED);
				}
		}

		while(fscanf(fp1,"%d %d %d",&temp1,&temp2,&temp3) != EOF){
				node[temp1].rout_table[temp2]=make_pair(temp2,temp3);	// Add To initial Routing Table
				node[temp2].rout_table[temp1]=make_pair(temp1,temp3);

				node[temp1].neighbor.push_back(make_pair(temp2,temp3));	// Add To Neighbor
				node[temp2].neighbor.push_back(make_pair(temp1,temp3));
		}

		// 0-2) Update Routing Table
		for(int i=0;i<n;i++)
				update.push(i);
		Update_Routing_Table();
		Print_Routing_Tables();
		// 0-3) Simulate with message.txt
		Simulate();

		// STEP 1
		int change_start, change_end, change_value;
		while(fscanf(fp3,"%d %d %d", &change_start,&change_end,&change_value)!=EOF){
				// 1-1) Apply One line of change to network
				// Disconnecting connection

				int num=node[change_start].neighbor.size();
				for(int i=0;i<num;i++){
						if(node[change_start].neighbor[i].first==change_end){
								node[change_start].neighbor.erase(node[change_start].neighbor.begin()+i);
								break;
						}
				}
				num=node[change_end].neighbor.size();
				for(int i=0;i<num;i++){
						if(node[change_end].neighbor[i].first==change_start){
								node[change_end].neighbor.erase(node[change_end].neighbor.begin()+i);
								break;
						}
				}

				// Updating New Connection

				if(change_value != DISCONNECTED){		
						node[change_start].neighbor.push_back(make_pair(change_end,change_value));
						node[change_end].neighbor.push_back(make_pair(change_start,change_value));
				}
				else{
						for(int i=0;i<n;i++){
								if(node[change_start].rout_table[i].first==change_end)
										node[change_start].rout_table[i]=make_pair(-1,DISCONNECTED);
								if(node[change_end].rout_table[i].first==change_start)
										node[change_end].rout_table[i]=make_pair(-1,DISCONNECTED);
						}
				}
				num = node[change_start].neighbor.size();
				for(int i=0;i<num;i++)
						update.push(node[change_start].neighbor[i].first);
				num=node[change_end].neighbor.size();
				for(int i=0;i<num;i++)
						update.push(node[change_end].neighbor[i].first);

				// 1-2) Update Routing Table, Print Routing Table Simulate
				update.push(change_start);
				update.push(change_end);
				Update_Routing_Table();
				Print_Routing_Tables();
				Simulate();
		}


		printf("Complete. Output file written to output_dv.txt.\n");

		fclose(fp1);
		fclose(fp2);
		fclose(fp3);
		fclose(fout);

		return 0;
}
