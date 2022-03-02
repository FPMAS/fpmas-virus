#include "agent.h"
#include "config.h"
#include <fpmas/random/random.h>

using namespace fpmas::model;

template<template<typename> class SyncMode>
class VirusModel : public GridModel<SyncMode>{
	private:
		InfectionMode infection_mode;
		int grid_width;
		int grid_height;

		Behavior<AgentPopulation> move_behavior;
		fpmas::model::IdleBehavior dead_behavior;


        fpmas::model::GridLoadBalancing grid_load_balancing;

		void initInfected(std::size_t infected_count);

	public:
		VirusModel(const YAML::Node& config);

};

template<template<typename> class SyncMode>
VirusModel<SyncMode>::VirusModel(const YAML::Node& config) :
	fpmas::model::GridModel<SyncMode>(grid_load_balancing),
	infection_mode(config["infection_mode"].as<InfectionMode>()),
	grid_width(config["grid"]["width"].as<int>()),
	grid_height(config["grid"]["height"].as<int>()),
	move_behavior(
			(infection_mode == READ) ? &AgentPopulation::move_type_read
			: &AgentPopulation::move_type_written
			),
	grid_load_balancing(grid_width, grid_height, fpmas::communication::WORLD)
{
	AgentPopulation::alpha = config["alpha"].as<float>();
	AgentPopulation::beta = config["beta"].as<float>();
	AgentPopulation::mortality_rate = config["mortality"].as<float>();

	//Defines a grid builder, that will build a grid of size height,width
	fpmas::model::MooreGridBuilder<> grid_builder(grid_width, grid_height);

	//Builds the grid (distributed process)
	grid_builder.build(*this);

	// Builds a new MoveAgentGroup associated to move_behavior
	auto& move_group = this->buildMoveGroup(ALIVE_GROUP, move_behavior);
	auto& die_group = this->buildGroup(DEAD_GROUP, dead_behavior);      

	// Random uniform mapping for the built grid
	UniformGridAgentMapping mapping(
			grid_builder.width(), grid_builder.height(),
			config["agent_count"].as<std::size_t>()
			);


	//Build agent instances with a 'new AgentPopulation' statement
	DefaultSpatialAgentFactory<AgentPopulation> factory; 
 
	//Initializes GridAgentExamples on the grid (distribued process)/
	//All agents are automatically added on to the move_group
	GridAgentBuilder<>().build(
			*this, {move_group}, factory, mapping
			);
 
	auto init_infected =  fpmas::random::split_sample(
			this->getMpiCommunicator(), move_group.localAgents(),
			config["init_infected"].as<std::size_t>()
			);
	for(auto agent : init_infected)
		((AgentPopulation*) agent)->setState(INFECTED);

	//Schedules AgentPopulation execution
	this->scheduler().schedule(0.0, 1, this->loadBalancingJob());

	this->scheduler().schedule(0.1, 1, move_group.jobs());

	this->scheduler().schedule(0.2, 1, die_group.jobs());
}
