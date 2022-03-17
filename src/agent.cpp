#include <algorithm>
#include <cstdlib>
#include <fpmas/random/distribution.h>
#include <fpmas/utils/log.h>

#include "model.h"

const MooreRange<MooreGrid<>> AgentPopulation::range(1);
double AgentPopulation::alpha = 0.0;
double AgentPopulation::beta = 0.0;
double AgentPopulation::mortality_rate = 0;

AgentPopulation::AgentPopulation() : AgentPopulation(SUSCEPTIBLE) {
}

AgentPopulation::AgentPopulation(State state)
	: state(state) {
	}

void AgentPopulation::move() {
	// Gets GraphCells currently in the agent mobility field.
	// Here it represents the 8 Moore neighbor cells, and the current location
	// cell.
	auto cell = this->mobilityField()
		.sort() // Sort cells according to their position: the mobility field is in the same order independently from the current distribution
		.random(this->rd()); // Selects an item independently from the current distribution

	FPMAS_LOGI(fpmas::communication::WORLD.getRank(), "AGENT",
			"Agent %s moves to %s",
			FPMAS_C_STR(this->node()->getId()),
			FPMAS_C_STR(cell->location())
			);
	// Moves to this cell
	this->moveTo(cell);
}

State AgentPopulation::getState() {
	return this->state;
}

void AgentPopulation::setState(State s){
	this->state = s;
}

void AgentPopulation::recover(){
	float random = this->random();
	if(random < alpha){
		FPMAS_LOGI(fpmas::communication::WORLD.getRank(), "AGENT",
				"Agent %s recovers", FPMAS_C_STR(this->node()->getId())
				);
		this->setState(RECOVERED); //Acquire dans le setState
	} 
}

void AgentPopulation::die(){
	float random = this->random();
	if(random < mortality_rate){
		FPMAS_LOGI(fpmas::communication::WORLD.getRank(), "AGENT",
				"Agent %s dies", FPMAS_C_STR(this->node()->getId())
				);
		this->setState(DEAD);
		this->model()->getGroup(DEAD_GROUP).add(this);
		this->model()->getGroup(ALIVE_GROUP).remove(this);
	}
}

template<>
void AgentPopulation::behavior<InfectionMode::READ>() {
	if(this->getState() == SUSCEPTIBLE) {
		std::size_t infected_neighbors = 0;
		for(auto agent : this->perceptions<AgentPopulation>()) {
			fpmas::model::ReadGuard read(agent);

			if(agent->getState() == INFECTED)
				infected_neighbors++;
		}
		float infection_probability = 1 - std::pow(1-beta, infected_neighbors);

		//Random
		float random = this->random();
		if(random <  infection_probability){
			FPMAS_LOGI(fpmas::communication::WORLD.getRank(), "AGENT",
					"Agent %s gets infected", FPMAS_C_STR(this->node()->getId())
					);
			fpmas::model::AcquireGuard acq(this);
			this->setState(INFECTED); // Write local
		}
	}

	fpmas::model::AcquireGuard acq(this);
	if(this->getState() == INFECTED)
		recover(); 
	if(this->getState() == INFECTED)
		die();
	if(this->getState() != DEAD)
		move();
}

template<>
void AgentPopulation::behavior<InfectionMode::WRITE>(){
	if(this->getState() == INFECTED){
		for(auto agent : this->perceptions<AgentPopulation>()){
			// Ensures each agent is infected at most by one agent
			fpmas::model::AcquireGuard acquire(agent);
			if(agent->getState() == SUSCEPTIBLE){
				float random = this->random();

				if(random < beta){
					FPMAS_LOGI(fpmas::communication::WORLD.getRank(), "AGENT",
							"Agent %s infects %s",
							FPMAS_C_STR(this->node()->getId()),
							FPMAS_C_STR(agent->node()->getId())
							);
					agent->setState(INFECTED);
				}
			}	
		}
	}

	fpmas::model::AcquireGuard acq(this);
	if(this->getState() == INFECTED)
		recover();
	if(this->getState() == INFECTED)
		die();
	if(this->getState() != DEAD)
		move();
}
