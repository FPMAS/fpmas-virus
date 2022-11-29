#include "agent.h"

using namespace fpmas::model;

/**
 * @file model.h
 * Virus model implementation.
 */

/**
 * Virus model definition.
 *
 * The model is based on a grid on which agents are uniformly placed at the
 * initialization of the model. A subset of randomly selected agents is also
 * INFECTED.
 *
 * The model then consists in executing the AgentPopulation::behavior() at each
 * time step.
 *
 * The model is deterministically initialized, so that the initial state of the
 * model only depends on the provided seed but not on the current distribution
 * or the process count. If the `GLOBAL_GHOST` synchronization is used, the
 * complete model execution is deterministic and independent from the process
 * count, but not with the `HARD_SYNC` or `GHOST` modes.
 */
template<template<typename> class SyncMode>
class VirusModel : public GridModel<SyncMode>{
	private:
		int grid_width;
		int grid_height;

		/*
		 * When alive, agents in the ALIVE_GROUP execute
		 * AgentPopulation::behavior().
		 */
		Behavior<AgentPopulation> agent_behavior;
		/*
		 * When dead, agents do nothing but are still contained in the
		 * DEAD_GROUP.
		 */
		fpmas::model::IdleBehavior dead_behavior;

		/*
		 * The model uses the GridLoadBalancing, that is the most efficient in
		 * this case (grid based model with a uniform agent distribution).
		 */
		fpmas::model::GridLoadBalancing grid_load_balancing;

	public:
		/**
		 * VirusModel constructor.
		 *
		 * See the example config.yml file and the README.md documentation for
		 * more information about how the model is configured.
		 */
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
	AgentPopulation::recover_rate = config["recover_rate"].as<float>();
	AgentPopulation::infection_rate = config["infection_rate"].as<float>();
	AgentPopulation::mortality_rate = config["mortality_rate"].as<float>();

	//Defines a grid builder, that will build a grid of size height,width
	fpmas::model::MooreGridBuilder<> grid_builder(grid_width, grid_height);

	//Builds the grid (distributed process)
	grid_builder.build(*this);

	// Builds a new MoveAgentGroup associated to agent_behavior
	auto& agent_group = this->buildMoveGroup(ALIVE_GROUP, agent_behavior);
	// Agents do nothing in this group
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

	//Deterministically initializes `init_infected` agents on the grid
	agent_builder.initSample(
			config["init_infected"].as<std::size_t>(),
			[] (fpmas::api::model::GridAgent<fpmas::model::GridCell>* agent) {
			((AgentPopulation*) agent)->setState(INFECTED);
			}
			);

	this->graph().synchronize();

	// Schedules load balancing at each time step
	this->scheduler().schedule(0.0, 1, this->loadBalancingJob());

	//Schedules AgentPopulation execution at each time step
	this->scheduler().schedule(0.1, 1, agent_group.jobs());
}
