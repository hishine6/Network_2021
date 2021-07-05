#include<stdio.h>
#include<iostream>
#include<queue>
#include<string.h>


#define MAX_NODE 100
#define MAX_COST 100
#define DISCONNECTED -999 // = 1 bigger than MAXIMUM COST
#define MAX_MESSAGE 1000
using namespace std;

FILE *fp1, *fp2, *fp3, *fout;
int n;	// number of nodes

typedef struct _Node{
		pair<int, int> rout_table[MAX_NODE];	// Routing Table
		vector< pair<int,int> > neighbor;	//Neighbor Nodes
}Node;
Node node[MAX_NODE];

void Simulate(){
		fseek(fp2,0,SEEK_SET);
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
								fprintf(fout,"%d ",hops.front());
								hops.pop();
						}
						fprintf(fout,"message%s",message);
				}
		}
		fprintf(fout,"\n");
}




// Tie-Breaking rule 2
// First, try the lower cost
// Second, if cost is same, try the smaller ID router
struct cmp{
		bool operator() (pair<int, int> &a, pair<int, int> &b){
				if(a.second==b.second){
						return a.first < b.first;
				}
				else{
						return a.second < b.second;
				}
		}
};

void Dijkstras(int router_num){
		int num;
		priority_queue <pair <int, int>, vector<pair<int, int> >,cmp >PQ; 
		
		// first: next node, second: cost of expanding that node
		
		// 1) Push First Router
		PQ.push(make_pair(router_num,0));

		// 2) Pop Next Router
		while(!PQ.empty()){
				// 2) Pop Next router,
				int parent[MAX_NODE];
				int next_node = PQ.top().first;	// Next Spannable node 
				int next_cost = PQ.top().second;	// The cost to that spannable node
				PQ.pop();

				// Check neighbors of next_node, and update them
				num=node[next_node].neighbor.size();
				for(int i=0;i<num;i++){
						int test_node = node[next_node].neighbor[i].first;
						int test_cost = node[next_node].neighbor[i].second;

						if(node[router_num].rout_table[test_node].second > next_cost + test_cost || node[router_num].rout_table[test_node].second==DISCONNECTED){	// If new Route
								// Update Cost
								node[router_num].rout_table[test_node].second = next_cost + test_cost;
								// Update 'Next'
								parent[test_node]=next_node;
								if(node[router_num].rout_table[next_node].first == router_num)
										node[router_num].rout_table[test_node].first = test_node;
								else
									node[router_num].rout_table[test_node].first = node[router_num].rout_table[next_node].first;
								// Push to queue
								PQ.push(make_pair(test_node,next_cost+test_cost));
						}
						else if(node[router_num].rout_table[test_node].second == next_cost + test_cost){	// Tie-breaking rule 3
								int new_parent=next_node;
								int old_parent=parent[test_node];

								if(old_parent > new_parent){	// Compare Parent ID, Change Root Parent
										parent[test_node]=next_node;
										if(node[router_num].rout_table[next_node].first == router_num)
												node[router_num].rout_table[test_node].first = test_node;
										else
											node[router_num].rout_table[test_node].first = node[router_num].rout_table[next_node].first;
								}
								PQ.push(make_pair(test_node,next_cost+test_cost));
						}
				}
		}
}

void Print_Routing_Tables(){
		for(int i=0;i<n;i++){
				for(int j=0;j<n;j++){
						if(node[i].rout_table[j].first==-1)
								continue;
						fprintf(fout,"%d %d %d\n",j,node[i].rout_table[j].first,node[i].rout_table[j].second);
				}
				fprintf(fout,"\n");
		}
}


void Update_Routing_Table(){
		// Update Each Routing Table using Dijkstra algorithm
		for(int i=0;i<n;i++){
				for(int j=0;j<n;j++){
						if(i==j)
								node[i].rout_table[j]=make_pair(i,0);
						else
								node[i].rout_table[j]=make_pair(-1,DISCONNECTED);
				}
		}
		for(int i=0;i<n;i++)
				Dijkstras(i);
}

int main(int argc, char*argv[]){
		int temp1,temp2,temp3;

		if(argc!=4){
				fprintf(stderr,"usage: linkstate topologyfile messagesfile changesfile\n");
				return 1;
		}
		fp1=fopen(argv[1],"r");
		fp2=fopen(argv[2],"r");
		fp3=fopen(argv[3],"r");
		fout=fopen("output_ls.txt","w");

		if(fp1==NULL || fp2==NULL || fp3==NULL){
				fprintf(stderr,"Error: open input file.\n");
				return 1;
		}

		// STEP 0

		// 0-1) Make Initial Routing Table
		fscanf(fp1,"%d",&n);
		while(fscanf(fp1,"%d %d %d",&temp1,&temp2,&temp3) != EOF){
				node[temp1].neighbor.push_back(make_pair(temp2,temp3));
				node[temp2].neighbor.push_back(make_pair(temp1,temp3));
		}

		// 0-2) Update Routing Table Using Dijkstra
		Update_Routing_Table();
		Print_Routing_Tables();

		// 0-3) Simulate with message.txt
		Simulate();

		// STEP 1

		int change_start, change_end, change_value;
		while(fscanf(fp3, "%d %d %d",&change_start,&change_end,&change_value)!=EOF){
				if(change_value == DISCONNECTED){
						// Delete the neighbors of change_start, change_end
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

				}
				else{
						int num=node[change_start].neighbor.size();
						for(int i=0;i<num;i++){
								if(node[change_start].neighbor[i].first == change_end){
										node[change_start].neighbor.erase(node[change_start].neighbor.begin()+i);
										break;
								}
						}
						num=node[change_end].neighbor.size();
						for(int i=0;i<num;i++){
								if(node[change_end].neighbor[i].first == change_start){
										node[change_end].neighbor.erase(node[change_end].neighbor.begin()+i);
										break;
								}
						}
						node[change_start].neighbor.push_back(make_pair(change_end,change_value));
						node[change_end].neighbor.push_back(make_pair(change_start,change_value));
				}
				Update_Routing_Table();
				Print_Routing_Tables();
				Simulate();
		}

		printf("Complete. Output file written to output_ls.txt.\n");
		
		fclose(fp1);
		fclose(fp2);
		fclose(fp3);
		fclose(fout);
		return 0;
}
