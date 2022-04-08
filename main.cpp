#include "model.h"
#include "output.h"
#include "fpmas.h"
#include "yaml-cpp/yaml.h"
#include "fpmas/utils/perf.h"
#include <algorithm>
#include <fpmas/io/csv_output.h>
#include <fpmas/io/output.h>

using fpmas::synchro::hard::ghost_link::HardSyncMode;
using fpmas::synchro::GhostMode;
using fpmas::synchro::GlobalGhostMode;
using namespace fpmas::utils::perf;

#define SYNC_MODE HardSyncMode

#define AGENT_TYPES\
	fpmas::model::GridCell::JsonBase,\
	AgentPopulation::JsonBase

FPMAS_DATAPACK_SET_UP(AGENT_TYPES);

int main(int argc, char *argv[])
{
	if(argc > 2) {
		unsigned long seed = std::stoul({argv[2]});
		fpmas::seed(seed);
	}
    
    FPMAS_REGISTER_AGENT_TYPES(AGENT_TYPES);
    fpmas::init(argc, argv);

		total_time.start();
    {
        YAML::Node config = YAML::LoadFile(argv[1]);
		fpmas::io::FileOutput file(
				"output." + std::to_string(fpmas::communication::WORLD.getSize())
				+ ".csv");

		fpmas::api::model::SpatialModel<fpmas::model::GridCell>* model;
		switch(config["sync_mode"].as<SyncMode>()) {
			case GHOST:
				model = new VirusModel<GhostMode>(config);
				break;
			case GLOBAL_GHOST:
				model = new VirusModel<GlobalGhostMode>(config);
				break;
			case HARD_SYNC:
				model = new VirusModel<HardSyncMode>(config);
				break;
		};

		ModelOutput output(*model, file);
        model->scheduler().schedule(0.3, 1, output.job());

		init_lb.start();
		model->runtime().execute(model->loadBalancingJob());
		init_lb.stop();

		run_model.start();
		// Runs the model for 10 iterations
		model->runtime().run(config["num_steps"].as<fpmas::scheduler::TimeStep>());
		run_model.stop();

		delete model;
    }
	total_time.stop();

	fpmas::io::FileOutput perf_file("perf.csv");

	commit_probes();
	PerfOutput perf_output(perf_file);
	perf_output.dump();

    fpmas::finalize();
    return 0;
}
