#include "fpmas.h"

using namespace fpmas::io;

/**
 * @file output.h
 * Defines CSV output features.
 */

/**
 * CSV model output.
 *
 * Automatically gathers SUSCEPTIBLE, INFECTED, RECOVERED and DEAD agents counts
 * from all processes and output the result at each time step in a single file
 * from process 0.
 */
class ModelOutput : public fpmas::io::DistributedCsvOutput<
	Local<fpmas::scheduler::TimeStep>, // Time step (local field)
	Reduce<int>,
	Reduce<int>,
	Reduce<int>,
	Reduce<int> 
	> {
		private:
			fpmas::api::model::AgentGroup& alive_group;
			fpmas::api::model::AgentGroup& dead_group;

		public:
			/**
			 * ModelOutput constructor.
			 *
			 * @param model Model from which data is output
			 * @param output Output stream to which data is dumped
			 */
			ModelOutput(
				const fpmas::api::model::Model& model,
				fpmas::api::io::OutputStream& output
				);
};
