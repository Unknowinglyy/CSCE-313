#include <iostream>
#include "country.h"
// #include "country.cpp" //remember to remove before final submittion
#include "state.h"
#include "reader.h"

int main() {

    //Read each row from CSV and store them on csvRows (2D vector)
    std::vector<std::vector<std::string>> csvRows;
    Reader stateReader("./us_state_data.csv");
    stateReader.readData(csvRows);
    int no_of_states = csvRows.size();
    State* state_list[50];
    //Create State Objects to pass them to Country Object
    //CSV format: StateName, Area, Population
    for(int i = 0; i < no_of_states ; i++){
        auto row = csvRows[i];
        PopulationData data(stod(row[1]),stol(row[2]));
        state_list[i] = new State(row[0],&data);
    }
    csvRows.clear();
    Country *usa = new Country("United States", no_of_states);

    // addStates performs a deep copy of state objects

    usa->addStates(state_list);

    std::cout << "Population Density of " << usa->getName() << " : " << usa->find_population_density_of_country() <<std::endl;

    State* highest_density_state = usa->find_highest_population_density_state();
    std::cout << "Highest Population Density State is : " << highest_density_state->getName() << " with density of : " << highest_density_state->getPopulationDensity() <<std::endl;
    for(int i = 0; i < no_of_states; i++){
        delete state_list[i];
    }
    delete usa;
}
