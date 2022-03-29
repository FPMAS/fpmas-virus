#include "model.h"
#include "output.h"
#include "fpmas.h"
#include "yaml-cpp/yaml.h"
#include "fpmas/utils/perf.h"
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

	Monitor monitor;
	Probe total_time("total_time");
	Probe init_model("init_model");
	Probe init_lb("init_lb");
	Probe run_model("run_model");

	total_time.start();
    {
        YAML::Node config = YAML::LoadFile(argv[1]);
		fpmas::io::FileOutput file(
				"output." + std::to_string(fpmas::communication::WORLD.getSize())
				+ ".csv");

		init_model.start();
		fpmas::api::model::Model* model;
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
		init_model.stop();

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

		monitor.commit(init_model);
		monitor.commit(init_lb);
		monitor.commit(run_model);
    }
	total_time.stop();
	monitor.commit(total_time);

	using fpmas::io::Reduce;
	using fpmas::utils::Max;
	fpmas::io::FileOutput perf_file("perf.csv");
	fpmas::io::DistributedCsvOutput<
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>
		> perf_output(
				fpmas::communication::WORLD, 0, perf_file,
				{total_time.label(), [&monitor, &total_time] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(total_time.label())
						);
				}},
				{init_model.label(), [&monitor, &init_model] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(init_model.label())
						);
				}},
				{init_lb.label(), [&monitor, &init_lb] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(init_lb.label())
						);
				}},
				{run_model.label(), [&monitor, &run_model] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(run_model.label())
						);
				}}
	);
	perf_output.dump();

    fpmas::finalize();
    return 0;
}
