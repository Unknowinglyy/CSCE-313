#include "country.h"
#include <iostream>
//comment for testing
Country::~Country() {
    for (int i = 0; i < no_of_states; i++){
        delete state_list[i];
    }
    delete[] state_list;
}

//Performs deep copy of State* states[] i.e. the array of state objects
void Country::addStates (State* states[]) {
    for (int i = 0; i < no_of_states; i++) {
        state_list[i] = new State(states[i]->getName(), states[i]->getPopulationData());
    }
}

//Find the population density of country
double Country::find_population_density_of_country() {
    long country_population = 0;
    double country_area = 0;

    for(int i = 0; i < no_of_states; i++){
        PopulationData* state_data = state_list[i]->getPopulationData();
        country_population += (*state_data).population;
        country_area += (*state_data).area;
    }
    return (country_population) / (country_area);

}

//Return a State with highest population Density
State* Country::find_highest_population_density_state() {
    State* highest_density_state = nullptr;
    double max_density_so_far = 0;

    for(int i = 0; i < no_of_states; i++){
        double current_density = state_list[i]->getPopulationDensity();
        if(current_density>max_density_so_far){
            highest_density_state = state_list[i];
            max_density_so_far = current_density;
        }
    }
    return highest_density_state;
}

std::string Country::getName(){
    return country_name;
}

int Country::getNoOfStates(){
    return no_of_states;
}


