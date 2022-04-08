#include "fpmas/utils/perf.h"
#include "fpmas.h"

using namespace fpmas::utils::perf;

extern Monitor monitor;
extern Probe total_time;
extern Probe init_model;
extern Probe init_grid;
extern Probe init_agents;
extern Probe init_infected;
extern Probe init_lb;
extern Probe run_model;

void commit_probes();

using fpmas::utils::Max;
using fpmas::io::Reduce;

class PerfOutput : public fpmas::io::DistributedCsvOutput<
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>
		> {
			public:
				PerfOutput(fpmas::api::io::OutputStream& output);
		};
