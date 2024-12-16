#include<bits/stdc++.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include<random>

using namespace std ;

const int MX = 201 ; 
const long double ACC = 1e-8 ; 
const int populationSize = 201 ; 
const int mutateRatio = 2 ; // how many cities to remove from the offspring at the time of mutation
const int generations = 3000000 ; 
int numCities = 51 ; 
int numSalesmen = 5 ; 
int k = 10 ; // how many solution to copy from parents to offspring (crossover)
long double dist[MX+1][MX+1] ;
// function to compare two long double values are same or not
bool isSame(long double a , long double b){
    long double diff = a-b;
    if(diff > ACC) return false ; 
    diff = b - a ; 
    if(diff > ACC) return false ; 
    return true ; 
}
// Chromosome representation for MTSP
struct Chromosome {
    vector<vector<int>> routes;  // Routes for multiple salesmen

    long double totalDistance ; 
    vector<long double> individualDistance ;  

    Chromosome(){
        totalDistance = 0.0 ; 
        vector<int> zero = {0}; 
        individualDistance.resize(numSalesmen , 0); 
        for(int i = 0 ; i < numSalesmen ; i++) {
            routes.push_back(zero);
        }
    }

    void calculateCosts(){
        totalDistance = 0.0 ; 
        // for (const auto &route : routes) {
        for(int I = 0 ; I < numSalesmen ; I++){
            long double routeDistance = 0.0;
            int last_city_in_route = -1 ; 
            for (size_t i = 1; i < routes[I].size(); i++) {
                routeDistance += dist[routes[I][i-1]][routes[I][i]] ; 
                last_city_in_route = routes[I][i] ; 
            }

            routeDistance += (dist[last_city_in_route][routes[I][0]]) ; 
            totalDistance += routeDistance;

            individualDistance[I] = routeDistance ; 
        }
    }

    void updateCost(int I){
        long double routeDistance = 0.0;
        int last_city_in_route = -1 ; 
        long double initialCost = individualDistance[I] ; 
        for (size_t i = 1; i < routes[I].size(); i++) {
            routeDistance += dist[routes[I][i-1]][routes[I][i]] ; 
            last_city_in_route = routes[I][i] ; 
        }
        routeDistance += (dist[last_city_in_route][routes[I][0]]) ; 
        totalDistance -= initialCost ;
        totalDistance += routeDistance ; 

        individualDistance[I] = routeDistance ; 
    }

    long double getMaxIndividual(){
        long double mx = 0.0 ; 
        for(auto it : individualDistance){
            if(it > mx) mx = it ; 
        }
        return mx ; 
    }

};
// Initialization of population
void getCityOrder(vector<int> &city){
    city.clear(); 
    for(int i = 1 ; i < numCities ; i++) city.push_back(i); 
}
// This will provide random k indexes of the city to remove 
unordered_set<int> generateRandomNumbers() {
    // Create a vector containing numbers from 1 to n
    vector<int> numbers(numCities-1);
    iota(numbers.begin(), numbers.end(), 1); // Fills numbers with {1, 2, ..., n-1}

    // Shuffle the numbers
    random_device rd;  // Seed source
    mt19937 gen(rd()); // Mersenne Twister random number generator
    shuffle(numbers.begin(), numbers.end(), gen);

    unordered_set<int> st ;
    for(int i = 0 ; i < mutateRatio; i++) st.insert(numbers[i]); 
    return st ; 
}
// Random number generator
int GetRandNum(int size){
    // Create a random device to seed the generator (for non-deterministic randomness)
    random_device rd;
    
    // Mersenne Twister generator seeded with random device
    mt19937 gen(rd());
    
    // Define a distribution range (for example, between 0 and 1)
    uniform_real_distribution<> dis(0.0, 1.0);
    
    // Generate a random number between 0 and 1
    double random_num = dis(gen);
    
    int RAND_NUM =  random_num * size ; 
    return RAND_NUM ; 
}
// RANDOM POPULATION GENERATION.................................
vector<Chromosome> initializePopulation(){
    long long SUM = numCities-1 , sum = 0 ; 
    SUM = SUM*(SUM+1); 
    SUM /= 2 ; 

    vector<Chromosome> population ; 
    
    vector<int> city ; 
    int size; 

    for(int i = 0 ; i < populationSize ; i++){
        // city zero is the depot , so inserting it in the route of every salesperson.....................
        Chromosome temp = Chromosome(); 

        sum = 0 ; 
        
        getCityOrder(city); 
 
        // initializing first salesman...........
        int salesmanIndex = GetRandNum(numSalesmen) , cityIndex ; 

        size = numCities - 1 ; 
        while(size > 0){
            // geting a random city index
            cityIndex =  GetRandNum(size) ; 
            sum += city[cityIndex] ; 

            // inserting cityIndex th city into the routes of salesmanIndex th salesman......
            temp.routes[salesmanIndex].push_back(city[cityIndex]); 

            // swaping the cityIndex th city with the last city 
            swap(city[cityIndex] , city[size-1]); 

            size--; 
            salesmanIndex++; 
            salesmanIndex %= numSalesmen ; 
        }

        // only when solutions are incorrectly generated .............
        if(sum != SUM){
            cout<<i<<" "<<sum<<" "<<SUM<<"\n"; 
            cerr << i<<"th solution of the initial population is incorrect !" << std::endl;
            population.clear(); 
            break; 
        }

        temp.calculateCosts(); 
        population.push_back(temp); 
    }
    return population ; 
}
// Loading the data into dist array....
void LOAD_DATA(){

    // ifstream file("dataFile.txt");

    std::ifstream file("C:\\Users\\akhil\\OneDrive\\Desktop\\Mini Project\\test.txt");      // FOR 51 
    // std::ifstream file("C:\\Users\\akhil\\OneDrive\\Desktop\\Mini Project\\gtsp100.txt");   // FOR 100
    if (!file) {
        cerr << "File could not be opened!" << std::endl;
        return ;
    }

    for (int i = 0; i < numCities ; ++i) {
        for (int j = 0; j < numCities ; ++j) {
            if (!(file >> dist[i][j])) {  // Read values into array
                std::cerr << "Error reading file or not enough data!" << std::endl;
                return ;
            }
        }
    }

    cout<<"Data Loaded Successfully !\n"; 
    file.close();  // Close the file
}
// Crossover code
Chromosome crossover(Chromosome &parent1, Chromosome &parent2) {
    // Initialize child chromosome with empty routes
    Chromosome child = Chromosome() ;
    // Set of city already assigned to the child
    unordered_set<int> assignedcity;
    // Copy depot (assumed to be 0) into assigned city
    assignedcity.insert(0);
    auto isUnassigned = [&assignedcity](int city) {
        return assignedcity.find(city) == assignedcity.end();
    };

    // First Phase: Select the most promising feasible routes
    for (int routeCount = 0; routeCount < k; ++routeCount) {
        Chromosome sourceParent ; 
        int selectedRouteIndex = -1;
        long double bestScore = numeric_limits<long double>::max();

        // Evaluate routes from both parents
        for (int parentIndex = 0; parentIndex < 2; ++parentIndex) {
            const Chromosome &parent = (parentIndex == 0) ? parent1 : parent2;

            for (size_t i = 0; i < numSalesmen; ++i) {
                const auto &route = parent.routes[i];
                // Skip routes that are fully assigned
                if (any_of(route.begin(), route.end(), isUnassigned)){
                    int xx = route.size() ; 
                    if(xx == 1) continue ; 
                    long double score = parent.individualDistance[i]/(xx-1); 
                    if (score < bestScore) {
                        bestScore = score;
                        sourceParent = parent;
                        selectedRouteIndex = i;
                    }
                }
            }
        }

        // If no more routes can be selected, break
        if (selectedRouteIndex == -1) break;

        // Copy the selected route to the child
        const auto &selectedRoute = sourceParent.routes[selectedRouteIndex];
        vector<int> newRoute;
        for (int city : selectedRoute) {
            if (isUnassigned(city)) {
                newRoute.push_back(city);
                assignedcity.insert(city);
            }
        }
        // child.routes.push_back(newRoute); 
        for(auto it : newRoute){
            child.routes[routeCount].push_back(it); 
        }
    }
    child.calculateCosts();

    // Second Phase: Assign remaining unassigned city

    vector<int> avalibleCities ; 
    for(int i = 1 ; i < numCities ; i++){
        if(isUnassigned(i)){
            avalibleCities.push_back(i); 
        }
    }
    // Shuffle the cities randomly
    random_device rd;              // Obtain a random seed
    mt19937 gen(rd());             // Seed the generator
    shuffle(avalibleCities.begin(), avalibleCities.end(), gen);

    // for(auto it : avalibleCities) cout<<it<<" "; 
    // cout<<"\n"; 

    for(auto city : avalibleCities){
        size_t bestRouteIndex = -1;
        size_t bestPosition = -1;

        long double currBestCost = numeric_limits<long double>::max();; 

        // Find the best position in existing routes
        for (size_t i = 0; i < numSalesmen; ++i) {
            auto &route = child.routes[i];
            for (size_t j = 1; j <= route.size(); ++j) { // Consider all insertion points
                long double increment = dist[route[j - 1]][city] + dist[city][route[j % route.size()]] -  dist[route[j - 1]][route[j % route.size()]];

                long double newCost = child.individualDistance[i] + increment ;

                if(newCost <= currBestCost){
                    bestRouteIndex = i;
                    bestPosition = j;
                    currBestCost = newCost ; 
                }
            }
        }

        // Insert the city at the best position
        if (bestRouteIndex != -1 && bestPosition != -1) {
            child.routes[bestRouteIndex].insert(child.routes[bestRouteIndex].begin() + bestPosition, city);
            assignedcity.insert(city);

            child.individualDistance[bestRouteIndex] = currBestCost ; 
        }
    }

    // Recalculate costs and fitness for the child
    child.calculateCosts();
    return child;
}
// Random Mutation Code
void mutate(Chromosome &offspring){

    // Destruction Phase..............

    unordered_set<int> toRemove = generateRandomNumbers(); 

    for(int i = 0 ; i < numSalesmen ; i++){
        bool found = false ; 
        for(int j = 1 ; j < offspring.routes[i].size(); j++){
            if(toRemove.find(offspring.routes[i][j]) != toRemove.end()){
                found = true ; 
                offspring.routes[i][j] = -1 ; 
            }
        }
        if(found){
            offspring.routes[i].erase(remove(offspring.routes[i].begin() , offspring.routes[i].end() , -1) , offspring.routes[i].end()); 
            offspring.updateCost(i); 
        }
    }

    // Construction Phase..............

    vector<int> temp(toRemove.begin() , toRemove.end()) ; 

    // Shuffle the temp vector to randomize the construction phase
    random_device rd;  
    mt19937 gen(rd()); 
    shuffle(temp.begin(), temp.end(), gen);


    for(auto city : temp){
        size_t bestRouteIndex = -1;
        size_t bestPosition = -1;

        long double currBestCost = numeric_limits<long double>::max();; 

        // Find the best position in existing routes
        for (size_t i = 0; i < numSalesmen; ++i) {
            auto &route = offspring.routes[i];
            for (size_t j = 1; j <= route.size(); ++j) { // Consider all insertion points
                long double increment = dist[route[j - 1]][city] + dist[city][route[j % route.size()]] -  dist[route[j - 1]][route[j % route.size()]];

                long double newCost = offspring.individualDistance[i] + increment ;

                if(newCost <= currBestCost){
                    bestRouteIndex = i;
                    bestPosition = j;
                    currBestCost = newCost ; 
                }
            }
        }

        // Insert the city at the best position
        if (bestRouteIndex != -1 && bestPosition != -1) {
            offspring.routes[bestRouteIndex].insert(offspring.routes[bestRouteIndex].begin() + bestPosition, city);

            offspring.individualDistance[bestRouteIndex] = currBestCost ; 
            offspring.updateCost(bestRouteIndex); 
        }
    }
    offspring.calculateCosts(); 
}
// Greedy Mutation Code
void mutateGreedy(Chromosome &offspring){
    
    // Destruction Phase..............

    unordered_set<int> toRemove ; 

    for(int I = 0 ; I < mutateRatio ; I++){
        size_t bestRouteIndex = -1;
        size_t bestPosition = -1;

        long double currBestCost = numeric_limits<long double>::max();; 

        // Find the best position in existing routes
        for (size_t i = 0; i < numSalesmen; ++i) {
            auto &route = offspring.routes[i];
            for (size_t j = 1; j < route.size(); ++j) { // Consider all deletion points
                long double change = dist[route[j - 1]][route[(j + 1)% route.size()]] - dist[route[j-1]][route[j]] - dist[route[j]][route[(j+1) % route.size()]];

                long double newCost = offspring.individualDistance[i] + change ;

                if(newCost <= currBestCost){
                    bestRouteIndex = i;
                    bestPosition = j;
                    currBestCost = newCost ; 
                }
            }
        }

        // Remove the city at the best position
        if (bestRouteIndex != -1 && bestPosition != -1) {
            toRemove.insert(offspring.routes[bestRouteIndex][bestPosition]); 

            offspring.routes[bestRouteIndex].erase(offspring.routes[bestRouteIndex].begin() + bestPosition);
            offspring.updateCost(bestRouteIndex);
        }
    }

    offspring.calculateCosts(); 

    // Construction Phase..............

    vector<int> temp(toRemove.begin() , toRemove.end()) ; 

    // Shuffle the temp vector to randomize the construction phase
    random_device rd;  
    mt19937 gen(rd()); 
    shuffle(temp.begin(), temp.end(), gen);

    for(auto city : temp){
        size_t bestRouteIndex = -1;
        size_t bestPosition = -1;

        long double currBestCost = numeric_limits<long double>::max();; 

        // Find the best position in existing routes
        for (size_t i = 0; i < numSalesmen; ++i) {
            auto &route = offspring.routes[i];
            for (size_t j = 1; j <= route.size(); ++j) { // Consider all insertion points
                long double increment = dist[route[j - 1]][city] + dist[city][route[j % route.size()]] -  dist[route[j - 1]][route[j % route.size()]];

                long double newCost = offspring.individualDistance[i] + increment ;

                if(newCost <= currBestCost){
                    bestRouteIndex = i;
                    bestPosition = j;
                    currBestCost = newCost ; 
                }
            }
        }

        // Insert the city at the best position
        if (bestRouteIndex != -1 && bestPosition != -1) {
            offspring.routes[bestRouteIndex].insert(offspring.routes[bestRouteIndex].begin() + bestPosition, city);

            offspring.individualDistance[bestRouteIndex] = currBestCost ; 
            offspring.updateCost(bestRouteIndex); 
        }
    
        // Recalculate costs and fitness for the offspring
    }
    offspring.calculateCosts(); 
}
// Steady-state selection
void steadyStateSelection(vector<Chromosome> &population,  Chromosome offspring) {
    // Replace the worst chromosome in the population with the offspring if it has better fitness
    int index = -1 ; 
    long double temp = 0.0 ; 
    long double mx = 0.0 ; 
    for(int i = 0 ;i < populationSize ; i++){
        temp = population[i].getMaxIndividual(); 
        if(temp > mx){
            mx = temp ;
            index = i ;  
        }
    }
    long double offspringValue = offspring.getMaxIndividual(); 
    if(mx > offspringValue){
        population[index] = offspring ; 
    }
}
// Main Genetic Algorithm function
void geneticAlgorithm() { 
    srand(time(0)); 
    // Step 1: Initialize population
    vector<Chromosome> population = initializePopulation();
    // Step 2: Run Genetic Algorithm
    int tt = generations ; 
    while(tt){
        // Select two parents for crossover
        int parent1Idx = rand() % populationSize;
        int parent2Idx = rand() % populationSize;

        if(parent1Idx == parent2Idx) continue ; 
        
        Chromosome parent1 = population[parent1Idx];
        Chromosome parent2 = population[parent2Idx];
        
        // // Step 4: Crossover
        Chromosome offspring = crossover(parent1, parent2);
        
        // // Step 6: Mutation
        // double tt = rand() / RAND_MAX; 
        // if(tt <= 0.7) mutateGreedy(offspring);
        // else mutate(offspring);

        mutate(offspring);

        // // Step 7: Steady-state replacement
        steadyStateSelection(population, offspring);
        tt--; 
    }
    
    int index = -1 ; 
    long double tempp = 0.0 ; 
    long double mx = 0.0 ; 
    for(int i = 0 ;i < populationSize ; i++){
        tempp = population[i].getMaxIndividual(); 
        if(tempp > mx){
            mx = tempp ;
            index = i ;  
        }
    }
    Chromosome bestChromosome = population[index]; 

    vector<vector<int>> temp = bestChromosome.routes ; 
    cout<<"Maximum distance by any salesperson : "<<bestChromosome.getMaxIndividual()<<"\n"; 
    
    cout<<"Individual Distances : "; 
    for(auto itt : bestChromosome.individualDistance) cout<<itt<<" "; 
    cout<<"\n"; 

    for(int i = 0 ; i < numSalesmen ; i++){
        cout<<i<<"th salesperson visits -> ";
        for(auto itt : temp[i]) cout<<itt<<" "; 
        cout<<"\n"; 
    }
    cout<<"\n"; 
    cout<<"\n"; 
}

int main() {
    LOAD_DATA(); 

    // cin>>numCities>>numSalesmen ; 

    // for (int i = 0; i < numCities; ++i)
    //     for (int j = 0; j < numCities; ++j)
    //         cin >> dist[i][j]; 

    k = numSalesmen ; 

    geneticAlgorithm(); 

    return 0 ; 
}
