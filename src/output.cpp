#include "output.h"
#include "agent.h"

ModelOutput::ModelOutput(
		const fpmas::api::model::Model& model,
		fpmas::api::io::OutputStream& output
		) :
	fpmas::io::DistributedCsvOutput<
			Local<fpmas::scheduler::TimeStep>, // Time step (local field)
			Reduce<int>,
			Reduce<int>,
			Reduce<int>,
			Reduce<int> 
		>(
				fpmas::communication::WORLD, 0, output,
				{"T", [&model] () {
					// Current time step
					return fpmas::scheduler::time_step(model.runtime().currentDate());
				}},
				{"S", [this] () {
					int susceptible = 0;
					for(auto agent : alive_group.localAgents())
						if(((AgentPopulation*) agent)->getState() == SUSCEPTIBLE)
							susceptible++;
					return susceptible;
				}},
				{"I", [this] () {
					int infected = 0;
					for(auto agent : alive_group.localAgents())
						if(((AgentPopulation*) agent)->getState() == INFECTED)
							infected++;
					return infected;
				}},
				{"R", [this] () {
					int removed = 0;
					for(auto agent : alive_group.localAgents())
						if(((AgentPopulation*) agent)->getState() == RECOVERED)
							removed++;
					return removed;
				}},
				{"D", [this] () {
					return dead_group.localAgents().size();
				}}),
				alive_group(model.getGroup(ALIVE_GROUP)),
				dead_group(model.getGroup(DEAD_GROUP))
				{
				}
