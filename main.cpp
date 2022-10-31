#include "model.h"
#include "output.h"
#include "fpmas.h"
#include "yaml-cpp/yaml.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

using fpmas::synchro::hard::ghost_link::HardSyncMode;
using fpmas::synchro::GhostMode;
using fpmas::synchro::GlobalGhostMode;

#define SYNC_MODE HardSyncMode

#define AGENT_TYPES\
	fpmas::model::GridCell::JsonBase,\
	AgentPopulation::JsonBase

FPMAS_DATAPACK_SET_UP(AGENT_TYPES);

int main(int argc, char *argv[])
{
    FPMAS_REGISTER_AGENT_TYPES(AGENT_TYPES);

	CLI::App app("fpmas-metamodel");
	std::string config_file;
	app.add_option("config_file", config_file, "fpmas-virus configuration file")
		->required();
	unsigned long seed = fpmas::random::default_seed;
	app.add_option("-s,--seed", seed, "Random seed");

	CLI11_PARSE(app, argc, argv);

	fpmas::seed(seed);
    fpmas::init(argc, argv);
    {
        YAML::Node config = YAML::LoadFile(argv[1]);
		fpmas::io::FileOutput file("output.csv");

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
		ModelOutput output(*model, file);
        model->scheduler().schedule(0.3, 1, output.job());

		// Runs the model for 10 iterations
		model->runtime().run(config["num_steps"].as<fpmas::scheduler::TimeStep>());

		delete model;
    }
    fpmas::finalize();
    return 0;
}
