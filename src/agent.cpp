#include <cstdlib>

#include "agent.h"

double full_random(int min, int max){
	fpmas::random::random_device rd;
	fpmas::random::UniformRealDistribution<> real_distribution(min,max);
	return real_distribution(rd);
}

const MooreRange<MooreGrid<>> AgentPopulation::range(1);
double AgentPopulation::alpha = 0.0;
double AgentPopulation::beta = 0.0;
double AgentPopulation::mortality_rate = 0;

AgentPopulation::AgentPopulation() : AgentPopulation(SUSCEPTIBLE) {
}

AgentPopulation::AgentPopulation(State state)
	: state(state) {}

void AgentPopulation::move_type_read() {
	//printAgent();
	if(this->getState() == SUSCEPTIBLE) {
		std::size_t infected_neighbors = 0;
		for(auto agent : this->perceptions<AgentPopulation>())
			if(agent->getState() == INFECTED)
				infected_neighbors++;
		float infection_probability = 1 - std::pow(1-beta, infected_neighbors);

		//Random
		float random = full_random(0,1);
		if(random <  infection_probability){
			this->setState(INFECTED); //Acquire in setState	
		}
	} else if(this->getState() == INFECTED){
		this->recovery(); 
	}
	dying();

	// Gets GraphCells currently in the agent mobility field.
	// In this examples, this represents the 8 Moore neighbor cells, and the
	// current location cell.
	auto mobility_field = this->mobilityField();

	// Selects a random GridCell from the mobility field
	auto cell = mobility_field.random();

	// Moves to this cell
	this->moveTo(cell);
}

void AgentPopulation::move_type_written(){
	//printAgent();
	dying();

	if(this->getState() != DEAD){
		if(this->getState() == INFECTED){
			for(auto agent : this->perceptions<AgentPopulation>()){
				if(agent->getState() == SUSCEPTIBLE){
					//Random
					float random = full_random(0,1);

					if(random < beta){
						agent->setState(INFECTED);
					}
				}	
			}
			recovery();//Nombre de pas de temps après lequel il peut essayer de ne plus etre malade
		}
	

		// Gets GraphCells currently in the agent mobility field.
		// In this examples, this represents the 8 Moore neighbor cells, and the
		// current location cell.
		auto mobility_field = this->mobilityField();

		// Selects a random GridCell from the mobility field
		auto cell = mobility_field.random();

		// Moves to this cell
		this->moveTo(cell);
	}
}

State AgentPopulation::getState() {
	fpmas::model::ReadGuard read(this);	
	return this->state;
}

void AgentPopulation::setState(State s){
	fpmas::model::AcquireGuard acquire(this);
	this->state = s;
}

void AgentPopulation::recovery(){
	float random = full_random(0,1);
	if(random < alpha){
	 	this->setState(RECOVERED); //Acquire dans le setState
	} 
}

void AgentPopulation::dying(){
	if(this->getState() == INFECTED){
		float random = full_random(0,1);
		if(random < mortality_rate){
			this->setState(DEAD);
		}
	}
	
	if(this->getState() == DEAD){
		this->model()->getGroup(DEAD_GROUP).add(this);
		this->model()->getGroup(ALIVE_GROUP).remove(this);
	}
}


void AgentPopulation::to_json(nlohmann::json& j, const AgentPopulation * agent){
	j["s"] = agent->state;
}

AgentPopulation* AgentPopulation::from_json(const nlohmann::json& j){
	return new AgentPopulation(
		j.at("s").get<State>()
	);
}
