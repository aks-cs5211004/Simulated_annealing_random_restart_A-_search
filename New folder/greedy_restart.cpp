#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <random>
#include <climits>
#include <chrono>
#include <map>
#include <stdio.h>
using namespace std::chrono;

using namespace std;

// defining a structure for the node
typedef struct stateNode{
    int heuristicCost;
    stateNode* bestNeighbour;
    vector<int> zoneIndexes; //  at location i we have zone zonIndex[i]
} stateNode;

class HillClimb{
    private:
        int t,z,l;
        vector<vector<int> > T; // matrix where T(i,j) =  time taken to go from location i to j.
        vector<vector<int> > N; // matrix where N(i,j) =   number of rounds from zone i to j.
        stateNode* currentState;
        int totalRestart;
        int totalStateChanges;
        int bestCost;
        stateNode* bestState;
        map <vector<int>,int> loc_to_cost;

    public: 
        HillClimb(int t,int z, int l, vector<vector<int> > Times, vector<vector<int> > Ns,map <vector<int>,int> lc ){
            this->t = t*60;
            this->z = z;
            this->l = l;
            this->T = Times;
            this->N = Ns;
            this->totalRestart = 0;
            this->totalStateChanges = 0;
            this->loc_to_cost=lc;
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
        void setBestCost(int cost){
            this->bestCost = cost;
        }
        int getBestCost(){
            return this->bestCost;
        }
        stateNode* getBestState(){
            return this->bestState;
        }


        void setInitialState();
        void printState(stateNode*);

        int getCost(stateNode*); // get cost of a state 
        int derivedCost(stateNode*, int, int);

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

stateNode* HillClimb::randomZoneAssign(){
    stateNode* node = new stateNode;
    vector<int> zoneIndexes(l,0);
    for(int i=0;i<l;i++){
        if(i<z) zoneIndexes[i] = i+1;
        else zoneIndexes[i] = 0;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);
    shuffle(zoneIndexes.begin(),zoneIndexes.end(),e);
    node->zoneIndexes = zoneIndexes;
    node->heuristicCost= getCost(node);
    // cout<< "Initial State: ";
    // for(int i=0;i<l;i++){
    //     cout<<node->zoneIndexes[i]<<" ";
    // }
    // cout<<endl;
    return node;
}
int HillClimb::getCost(stateNode* node){
    int cost = 0;
    for(int i=0;i<l;i++){
        for(int j=0;j<l;j++){
            // if(node->zoneIndexes[i] != -1 && node->zoneIndexes[j] != -1)
                cost += T[i][j]*N[node->zoneIndexes[i]][node->zoneIndexes[j]];
        }
    }
    return cost;
}
int HillClimb::derivedCost(stateNode* parent, int i, int j){
    int cost = parent->heuristicCost;
    // int flagi=1,flagj=1;
    // if(parent->zoneIndexes[i]==-1)  flagi =0;
    // if(parent->zoneIndexes[j]==-1)  flagj =0;
    for(int k=0;k<l;k++){
        // if(parent->zoneIndexes[k]!=-1){
        cost-=T[i][k]*N[parent->zoneIndexes[i]][parent->zoneIndexes[k]];
        cost-=T[j][k]*N[parent->zoneIndexes[j]][parent->zoneIndexes[k]];
        cost-=T[k][i]*N[parent->zoneIndexes[k]][parent->zoneIndexes[i]];
        cost-=T[k][j]*N[parent->zoneIndexes[k]][parent->zoneIndexes[j]];
        cost+=T[k][j]*N[parent->zoneIndexes[k]][parent->zoneIndexes[i]];
        cost+=T[k][i]*N[parent->zoneIndexes[k]][parent->zoneIndexes[j]];
        cost+=T[j][k]*N[parent->zoneIndexes[i]][parent->zoneIndexes[k]];
        cost+=T[i][k]*N[parent->zoneIndexes[j]][parent->zoneIndexes[k]];
        // }
    }
    cost+=2*T[i][j]*(N[parent->zoneIndexes[j]][parent->zoneIndexes[i]]+N[parent->zoneIndexes[i]][parent->zoneIndexes[j]]);
    cost-=2*T[i][j]*(N[parent->zoneIndexes[j]][parent->zoneIndexes[j]]+N[parent->zoneIndexes[i]][parent->zoneIndexes[i]]);


    return cost;
}
void HillClimb::setInitialState(){
    this->currentState = randomZoneAssign();
    // this->currentState = new stateNode;
    // this->currentState->zoneIndexes.resize(l);
    // cin>>this->currentState->zoneIndexes[0];
    // cin>>this->currentState->zoneIndexes[1];
    // cin>>this->currentState->zoneIndexes[2];
    // cin>>this->currentState->zoneIndexes[3];
    // this->currentState->heuristicCost = getCost(this->currentState);
}
stateNode* HillClimb::getBestNeighbour(stateNode* node){
    // randomly swap any two values and compute the cost.
    // do this for all swaps possible in this array.
    int currCost, bestCost, tempCost;
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
    
    for(int i=0;i<l-1;i++){
        for(int j=i+1; j<l;j++){
                tempNode->zoneIndexes=node->zoneIndexes;
                swap(tempNode->zoneIndexes[i],tempNode->zoneIndexes[j]);
                // std::cout<<"Neighbour in consideration : ";
                // for(int i=0;i<l;i++){
                //     std::cout<<tempNode->zoneIndexes[i]<<" ";
                // }
                // std::cout<<endl;
                int newCost=0;
                // if(loc_to_cost.count(tempNode->zoneIndexes)==0){
                newCost = derivedCost(node,i,j);
                //         loc_to_cost[tempNode->zoneIndexes] = newCost;
                // }
                // else{
                //       newCost=loc_to_cost[tempNode->zoneIndexes];
                // }

                
                if(newCost <= bestCost){
                        bestCost  = newCost;
                        bestNode->zoneIndexes = tempNode->zoneIndexes;
                        bestNode->heuristicCost = bestCost;
                    
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
    cout<<"Initial State: ";
    printState(this->currentState);
    cout<<this->currentState->heuristicCost<<endl;
    stateNode* bestNeighbour = new stateNode;
    this->setBestCost(INT_MAX);
    int step_limit=l*l*z;

    while(this->t*1000000>duration_cast<microseconds>(high_resolution_clock::now()- start).count()){
        int c=0;
        while(c<step_limit){
            this->totalStateChanges++;
            bestNeighbour = getBestNeighbour(this->currentState);
            if(bestNeighbour->heuristicCost == this->currentState->heuristicCost) break;
            else this->currentState = bestNeighbour;
            c++;
        }
        if(this->currentState->heuristicCost < this->getBestCost()){
            this->setBestCost(this->currentState->heuristicCost);
            this->setBestState(this->currentState);
            cout<<"Best cost till now from getCost()....................."<<this->getCost(currentState)<<endl;
        }
        cout<<"Random Restart No. "<<this->totalRestart<<endl;
        this->randomRestart();
    } 
}

// change everything from pointer to vector<int> or int array
int main(){
    int t,z,l;
    cout<<"Enter the number of zones and locations: ";
    cin>>t>>z>>l;
    vector<vector<int> > T(l,vector<int>(l));
    vector<vector<int> > N(z+1,vector<int>(z+1,0));
    map <vector<int>,int> locost;
    for(int i=1;i<z+1;i++){
        for(int j=1;j<z+1;j++){
            cin>>N[i][j];
        }
    }
    for(int i=0;i<l;i++){
        for(int j=0;j<l;j++){
            cin>>T[i][j];
        }
    }
    HillClimb hc(t,z,l,T,N,locost);
    hc.startSearch();
    cout<<"Total restarts: "<<hc.gettotalRestart()<<endl;
    cout<<"Total state changes: "<<hc.gettotalStateChanges()<<endl;
    cout<<"Best cost: "<<hc.getBestCost()<<endl;
    cout<<"Final state: "<< endl;
    hc.printState(hc.getBestState());
    return 0;
}