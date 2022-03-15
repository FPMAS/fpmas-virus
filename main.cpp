#include "model.h"
#include "output.h"
#include "fpmas.h"
#include "yaml-cpp/yaml.h"

using fpmas::synchro::HardSyncMode;
using fpmas::synchro::HardSyncModeWithGhostLink;
using fpmas::synchro::GhostMode;
using fpmas::synchro::GlobalGhostMode;

#define SYNC_MODE GlobalGhostMode

#define AGENT_TYPES\
	fpmas::model::GridCell::JsonBase,\
	AgentPopulation::JsonBase

FPMAS_JSON_SET_UP(AGENT_TYPES);

int main(int argc, char *argv[])
{
    
    FPMAS_REGISTER_AGENT_TYPES(AGENT_TYPES);
    fpmas::init(argc, argv);
    {
        YAML::Node config = YAML::LoadFile(argv[1]);
		fpmas::io::FileOutput file("output." + std::to_string(fpmas::communication::WORLD.getSize()) + ".csv");

		VirusModel<SYNC_MODE> model(config);
		ModelOutput output(model, file);

        //Schedules AgentPopulation exectution
        model.scheduler().schedule(0.3, 1, output.job());

        // Runs the model for 10 iterations
		model.runtime().run(config["num_steps"].as<fpmas::scheduler::TimeStep>());
    }
    fpmas::finalize();
    return 0;
}
