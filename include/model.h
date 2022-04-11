#include "agent.h"

using namespace fpmas::model;

template<template<typename> class SyncMode>
class VirusModel : public GridModel<SyncMode>{
	private:
		int grid_width;
		int grid_height;

		Behavior<AgentPopulation> agent_behavior;
		fpmas::model::IdleBehavior dead_behavior;


		fpmas::model::GridLoadBalancing grid_load_balancing;

		void initInfected(std::size_t infected_count);

	public:
		VirusModel(const YAML::Node& config);

};

template<template<typename> class SyncMode>
VirusModel<SyncMode>::VirusModel(const YAML::Node& config) :
	fpmas::model::GridModel<SyncMode>(grid_load_balancing),
	grid_width(config["grid"]["width"].as<int>()),
	grid_height(config["grid"]["height"].as<int>()),
	grid_load_balancing(grid_width, grid_height, fpmas::communication::WORLD)
{
	InfectionMode infection_mode
		= config["infection_mode"].as<InfectionMode>();
	switch(infection_mode) {
		case READ:
			agent_behavior = {&AgentPopulation::behavior<READ>};
			break;
		case WRITE:
			agent_behavior = {&AgentPopulation::behavior<WRITE>};
			break;
	}
	AgentPopulation::alpha = config["alpha"].as<float>();
	AgentPopulation::beta = config["beta"].as<float>();
	AgentPopulation::mortality_rate = config["mortality"].as<float>();

	//Defines a grid builder, that will build a grid of size height,width
	fpmas::model::MooreGridBuilder<> grid_builder(grid_width, grid_height);

	//Builds the grid (distributed process)
	grid_builder.build(*this);

	// Builds a new MoveAgentGroup associated to move_behavior
	auto& agent_group = this->buildMoveGroup(ALIVE_GROUP, agent_behavior);
	auto& die_group = this->buildGroup(DEAD_GROUP, dead_behavior);      

	// Random uniform mapping for the built grid
	UniformGridAgentMapping mapping(
			grid_builder.width(), grid_builder.height(),
			config["agent_count"].as<std::size_t>()
			);

	GridAgentBuilder<> agent_builder;
	//Initializes GridAgentExamples on the grid (distribued process)/
	//All agents are automatically added on to the agent_group
	agent_builder.build(
			*this, // Build agents in this model
			{agent_group}, // Add agents to agent_group
			// Factory method
			[infection_mode] () {
			return new AgentPopulation;
			},
			mapping // Agent counts per cell
			);

	// Initializes infected agents
	agent_builder.initSample(
			config["init_infected"].as<std::size_t>(),
			[] (fpmas::api::model::GridAgent<fpmas::model::GridCell>* agent) {
			((AgentPopulation*) agent)->setState(INFECTED);
			}
			);

	this->graph().synchronize();

	//Schedules AgentPopulation execution
	this->scheduler().schedule(0.0, 1, this->loadBalancingJob());

	this->scheduler().schedule(0.1, 1, agent_group.jobs());
}
