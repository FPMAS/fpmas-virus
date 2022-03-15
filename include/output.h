#include "fpmas.h"

using namespace fpmas::io;

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
			ModelOutput(
				const fpmas::api::model::Model& model,
				fpmas::api::io::OutputStream& output
				);
};
