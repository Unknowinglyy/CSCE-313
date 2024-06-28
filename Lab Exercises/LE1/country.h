#ifndef COUNTRY_H
#define COUNTRY_H
#include "string"
#include "state.h"

class Country{

    int no_of_states;
    State** state_list;
    std::string country_name;
    public:
        Country(const std::string& _country_name, int _no_of_states){
            country_name = _country_name;
            no_of_states = _no_of_states;
            state_list = new State *[no_of_states];
        }

        std::string getName();

        int getNoOfStates();

        void addStates (State* states[]);

        double find_population_density_of_country();

        State* find_highest_population_density_state();
        
        ~Country();
};
#endif //COUNTRY_H
