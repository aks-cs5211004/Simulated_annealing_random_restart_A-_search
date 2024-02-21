#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <random>
#include <climits>
#include <chrono>
#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <iomanip>
#include <map>
#include <sstream>
#include <fstream>
typedef long long ll;
using namespace std::chrono;

using namespace std;

// defining a structure for the node
typedef struct stateNode{
    ll heuristicCost;
    stateNode* bestNeighbour;
    vector<int> zoneIndexes; //  at location i we have zone zonIndex[i]
} stateNode;

class HillClimb{
    private:
        double time_limit;
        int z,l;
        vector<vector<int> > T; // matrix where T(i,j) =  time taken to go from location i to j.
        vector<vector<int> > N; // matrix where N(i,j) =   number of rounds from zone i to j.
        stateNode* currentState;
        int totalRestart;
        int totalStateChanges;
        ll bestCost;
        stateNode* bestState;
        double Temperature;
        double rate;
        string inpFile,outFile;


    public: 
        HillClimb(string inFile,string outFile){
            this->totalRestart = 0;
            this->totalStateChanges = 0;
            this->Temperature=0;
            this->rate=0.01;
            this->inpFile=inFile;
            this->outFile=outFile;
            readInputFile(inFile);
        }
        int gettotalRestart(){
            return this->totalRestart;
        }
        int gettotalStateChanges(){
            return this->totalStateChanges;
        }
        stateNode* getCurrentState(){
            return this->currentState;
        }
        void setBestState(stateNode* node){
            this->bestState = node;
        }
        void setBestCost(ll cost){
            this->bestCost = cost;
        }
        ll getBestCost(){
            return this->bestCost;
        }
        stateNode* getBestState(){
            return this->bestState;
        }

        void readInputFile(string);
        void writeOutputFile(string);
        void setInitialState();
        void printState(stateNode*);

        ll getCost(stateNode*); // get cost of a state 
        ll derivedCost(stateNode*, int, int);

        stateNode* getBestNeighbour(stateNode*); // get best neighbour of a state
        void startSearch(); // start the search
        void randomRestart(); // restart the search
        stateNode* randomZoneAssign();

};

void HillClimb::printState(stateNode* node){
    for(int i=0;i<l;i++){
        cout<<node->zoneIndexes[i]<<" ";
    }
    cout<<endl;
}


void HillClimb::writeOutputFile(string outputFileName){
    ofstream outputFile(outputFileName);
    map<int,int> zone_to_loc;
    // Check if the file is opened successfully
    if (!outputFile.is_open()) {
        cerr << "Failed to open the file for writing." << std::endl;
        exit(0);
    }
    for(int i=0;i<l;i++){
        if(bestState->zoneIndexes[i]!=0)
            zone_to_loc[bestState->zoneIndexes[i]]= i+1;
    }

    for(auto it:zone_to_loc) {
        outputFile<<it.second<<" ";
    }

    // Close the file
    outputFile.close();
}

void HillClimb::readInputFile(string inputfilename){
    fstream ipfile;
    ipfile.open(inputfilename, ios::in);
    if (!ipfile) {
        cout << "No such file\n";
        exit( 0 );
    }
    else {
        ipfile >> time_limit;
        time_limit=time_limit*(60*1000000);
        time_limit=time_limit-1000000;
        ipfile >> z;
        ipfile >> l;
        if(z>l) {
            cout<<"Number of zones more than locations, check format of input file\n";
            exit(0);
        }
        string buffer;
        T = vector<vector<int> >(l,vector<int>(l,0));
        N = vector<vector<int> >(z+1,vector<int>(z+1,0));
        for(int i=1;i<z+1;i++){
            for(int j=1;j<z+1;j++)
            ipfile>>N[i][j];
        }

        for(int i=0;i<l;i++){
            for(int j=0;j<l;j++)
            ipfile>>T[i][j];
        }
        ipfile.close();
    }
}

stateNode* HillClimb::randomZoneAssign(){
    stateNode* node = new stateNode;
    vector<int> zoneIndexes(l,0);
    for(int i=0;i<l;i++){
        if(i<z) zoneIndexes[i] = i+1;
        else zoneIndexes[i] = 0;
    }
    ll best_cost_till_now=LONG_LONG_MAX;
    stateNode* BestStartNode=new stateNode;
    for(int i=0;i<10;i++){
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine e(seed);
        shuffle(zoneIndexes.begin(),zoneIndexes.end(),e);
        node->zoneIndexes = zoneIndexes;
        node->heuristicCost= getCost(node);
        if(node->heuristicCost<=best_cost_till_now){
            best_cost_till_now=node->heuristicCost;
            BestStartNode->zoneIndexes = node->zoneIndexes;
            BestStartNode->heuristicCost=node->heuristicCost;
        }
    }
    return BestStartNode;
}
ll HillClimb::getCost(stateNode* node){
    ll cost = 0;
    for(int i=0;i<l;i++){
        for(int j=0;j<l;j++){
                cost += T[i][j]*N[node->zoneIndexes[i]][node->zoneIndexes[j]];
        }
    }
    return cost;
}
ll HillClimb::derivedCost(stateNode* parent, int i, int j){
    ll cost = parent->heuristicCost;
    for(int k=0;k<l;k++){
        cost-=T[i][k]*N[parent->zoneIndexes[i]][parent->zoneIndexes[k]];
        cost-=T[j][k]*N[parent->zoneIndexes[j]][parent->zoneIndexes[k]];
        cost-=T[k][i]*N[parent->zoneIndexes[k]][parent->zoneIndexes[i]];
        cost-=T[k][j]*N[parent->zoneIndexes[k]][parent->zoneIndexes[j]];
        cost+=T[k][j]*N[parent->zoneIndexes[k]][parent->zoneIndexes[i]];
        cost+=T[k][i]*N[parent->zoneIndexes[k]][parent->zoneIndexes[j]];
        cost+=T[j][k]*N[parent->zoneIndexes[i]][parent->zoneIndexes[k]];
        cost+=T[i][k]*N[parent->zoneIndexes[j]][parent->zoneIndexes[k]];
    }
    cost+=2*T[i][j]*(N[parent->zoneIndexes[j]][parent->zoneIndexes[i]]+N[parent->zoneIndexes[i]][parent->zoneIndexes[j]]);
    cost-=2*T[i][j]*(N[parent->zoneIndexes[j]][parent->zoneIndexes[j]]+N[parent->zoneIndexes[i]][parent->zoneIndexes[i]]);


    return cost;
}
void HillClimb::setInitialState(){
    this->currentState = randomZoneAssign();
}
stateNode* HillClimb::getBestNeighbour(stateNode* node){
    // randomly swap any two values and compute the cost.
    // do this for all swaps possible in this array.
    ll currCost, bestCost, tempCost;
    stateNode* bestNode = new stateNode;
    stateNode* tempNode = new stateNode;
    bestNode->zoneIndexes.resize(l);
    tempNode->zoneIndexes.resize(l);
    currCost = node->heuristicCost;
    bestCost = currCost;
    bestNode->zoneIndexes = node->zoneIndexes;
    bestNode->heuristicCost= node->heuristicCost;
    tempNode->zoneIndexes= node->zoneIndexes;
    tempNode->heuristicCost=node->heuristicCost;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((time_t)ts.tv_nsec);
    double prob=(double)rand()/RAND_MAX;
    int cont_sampling=min((int)sqrt(l),15);
    int k=0;
    while(k<cont_sampling){
                int i=rand()%l;
                int j=rand()%(l-i)+i;
                tempNode->zoneIndexes=node->zoneIndexes;
                //swap(tempNode->zoneIndexes[i],tempNode->zoneIndexes[j]);
                ll newCost=0;
                newCost = derivedCost(node,i,j);
                if(newCost-currCost<0){
                        swap(tempNode->zoneIndexes[i],tempNode->zoneIndexes[j]);
                        bestCost  = newCost;
                        bestNode->zoneIndexes = tempNode->zoneIndexes;
                        bestNode->heuristicCost = bestCost;   
                        break;
                }
                else if (newCost-currCost>0){
                    if(prob>=1-(double)exp(-(double)(newCost-currCost)/((this->Temperature)))){
                        swap(tempNode->zoneIndexes[i],tempNode->zoneIndexes[j]);
                        bestCost  = newCost;
                        bestNode->zoneIndexes = tempNode->zoneIndexes;
                        bestNode->heuristicCost = bestCost;    
                        break;
                    }
                    else{
                        k++;
                        continue;
                    }
                }
                else{
                    if(prob>=0.3){
                        swap(tempNode->zoneIndexes[i],tempNode->zoneIndexes[j]);
                        bestCost  = newCost;
                        bestNode->zoneIndexes = tempNode->zoneIndexes;
                        bestNode->heuristicCost = bestCost;    
                        break;
                    }
                    else{
                        k++;
                        continue;
                    }
                }
    }


    return bestNode;    
}

void HillClimb::randomRestart(){
    this->totalRestart++;
    this->setInitialState();
}

void HillClimb::startSearch(){
    auto start = high_resolution_clock::now();
    this->setInitialState();
    ll inital_cost=this->currentState->heuristicCost;
    stateNode* bestNeighbour = new stateNode;
    this->setBestCost(LONG_LONG_MAX);
    int step_limit=max(160000,l*l);
    while(this->time_limit>duration_cast<microseconds>(high_resolution_clock::now()- start).count()){
        int c=0;
        auto step_start = high_resolution_clock::now();
        while(c<step_limit && this->time_limit>duration_cast<microseconds>(high_resolution_clock::now()- start).count()){
            double time=(duration_cast<microseconds>(high_resolution_clock::now()- step_start).count());
            this->Temperature=time_limit/time;
            bestNeighbour = getBestNeighbour(this->currentState);
            this->currentState = bestNeighbour;
            this->totalStateChanges++;
            if(currentState->heuristicCost<this->getBestCost()){
                    this->setBestCost(currentState->heuristicCost);
                    this->setBestState(currentState);
                    this->writeOutputFile(this->outFile);
            }
            c++;
        }
        this->randomRestart();
    }
    
}

int main(int argc, char** argv){
    auto exec_time_start=high_resolution_clock::now();
    if ( argc < 3 )
    {   
        cout<<"Missing arguments\n";
        cout<<"Correct format : \n";
        cout << "./main <input_filename> <output_filename>";
        exit ( 0 );
    }
    string inputfilename ( argv[1] );
    string outputfilename(argv[2]);
    
    HillClimb hc(inputfilename,outputfilename);
    hc.startSearch();
    cout<<"Total restarts: "<<hc.gettotalRestart()<<endl;
    cout<<"Total state changes: "<<hc.gettotalStateChanges()<<endl;
    cout<<hc.getCost(hc.getBestState())<<endl;
    cout<<hc.getBestCost()<<endl;
    hc.printState(hc.getBestState());
    
    hc.writeOutputFile(outputfilename);
    auto exec_time_stop=high_resolution_clock::now();
    cout<<"Actual time taken in sec ="<<(duration_cast<seconds>(exec_time_stop- exec_time_start).count())<<endl;

    return 0;
}