#ifndef STATE_H
#define STATE_H
#include "string"
#include <cstring>


struct PopulationData{
    double area;
    long population;
    PopulationData(double area, long population) {
        this->area = area;
        this->population = population;
    };
    PopulationData () : area(), population() {}
};


class State{

    std::string state_name;
    PopulationData* state_data;

public:

    State(std::string state_name, PopulationData* _state_data)  {
        this->state_name = state_name;
        state_data = new PopulationData();
        memcpy(state_data, _state_data, sizeof(PopulationData));
    }

    State() {};

    ~State() {
        delete state_data;
    };

    std::string getName(){
        return state_name;
    }

    PopulationData* getPopulationData(){
        return state_data;
    }

    double getPopulationDensity(){
        return ((state_data->population) / (state_data->area));
    }
};


#endif //STATE_H
