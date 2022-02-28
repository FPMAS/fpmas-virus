#include "include/agent.h"
#include "fpmas.h"
#include "yaml-cpp/yaml.h"

using fpmas::synchro::HardSyncMode;
using fpmas::synchro::HardSyncModeWithGhostLink;
using fpmas::synchro::GhostMode;
using fpmas::synchro::GlobalGhostMode;

#define SYNC_MODE HardSyncModeWithGhostLink

#define AGENT_TYPES fpmas::model::GridCell::JsonBase, AgentPopulation::JsonBase

FPMAS_JSON_SET_UP(AGENT_TYPES);

int main(int argc, char *argv[])
{
    
    FPMAS_REGISTER_AGENT_TYPES(AGENT_TYPES);
    fpmas::init(argc, argv);
    {
        YAML::Node node = YAML::LoadFile(argv[1]);
        //Nombre d'agent, 1000 par défaut si on ne peut pas lire nb_agent
        int nombreAgent;
        if(node["nb_agent"]) nombreAgent = node["nb_agent"].as<int>();
        else nombreAgent = 1000;

        //Nombre d'agent infecté
        int nb_infected_agent_begin = 1;
        if(node["nb_infected_agent_begin"]) nb_infected_agent_begin = node["nb_infected_agent_begin"].as<int>();

        if(nb_infected_agent_begin > nombreAgent){
            std::cout << "Le nombre d'agent infecté doit etre inférieux au nombre d'agent total" << std::endl;
            fpmas::finalize();
            return 0;
        }

        //Taille de la grille
        int gridHeight;
        int gridWidth;
        if(node["grid_size"]){
            int value = node["grid_size"].as<int>();
            gridHeight = value;
            gridWidth = value;
        }else{
            gridHeight = 50;
            gridWidth = 50;
        }

        //Runtime
        int runtime;
        if(node["runtime"]) runtime = node["runtime"].as<int>();
        else runtime = 30;
        
        //Mode (Read ou Write)
        std::string mode;
        if(node["mode"]) mode = node["mode"].as<std::string>();
        else mode = "read";

        //ALPHA && BETA && MORTALITY && LIFE_EXPECTANCY && REPRODUCTION_RATE
        if(node["alpha"]) AgentPopulation::alpha = node["alpha"].as<double>();
        else AgentPopulation::alpha = 0.2;
        if(node["beta"]) AgentPopulation::beta = node["beta"].as<double>();
        else AgentPopulation::beta = 0.2;
        if(node["mortality"]) AgentPopulation::mortality_rate = node["mortality"].as<double>();
        else AgentPopulation::mortality_rate = 0.1;
        if(node["reproduction_rate"]) AgentPopulation::reproduction_rate = node["reproduction_rate"].as<double>();
        else AgentPopulation::reproduction_rate = 0.3;
        //Fin lire YAML

        fpmas::model::GridLoadBalancing gridLoadBalancing(gridHeight,gridWidth, fpmas::communication::WORLD);

        //Defines a new GridModel
        GridModel<SYNC_MODE> model(gridLoadBalancing);

        //Avoir une seed différentes à chaque fois, qui est envoyé sur chaque processeur
        std::size_t seed;
        FPMAS_ON_PROC(model.getMpiCommunicator(), 0){
            fpmas::random::random_device rd_device;
            seed = rd_device();
        }
        fpmas::communication::TypedMpi<std::size_t> mpi(fpmas::communication::WORLD);
        seed = mpi.bcast(seed,0);
        fpmas::seed(seed);
        //Fin Seed

        //Defines a grid builder, that will build a grid of size height,width
        MooreGridBuilder<> grid_builder(gridHeight,gridWidth);

        //Builds the grid (distributed process)
        grid_builder.build(model);

        //Behavior for read or write
        Behavior<AgentPopulation> move_behavior{
            (mode == "read")? &AgentPopulation::move_type_read : &AgentPopulation::move_type_written
        };

		fpmas::model::IdleBehavior die_behavior;
        

        // Builds a new MoveAgentGroup associated to move_behavior
        auto& move_group = model.buildMoveGroup(MOVE, move_behavior);

        auto& die_group = model.buildGroup(DIE, die_behavior);      


        fpmas::io::FileOutput out("output.csv");

        // A Watcher used to count how many agent are infected on all LOCAL agents
        fpmas::io::Watcher<int> count_infected = [&move_group] (){
            int infected = 0;
            for(auto agent : move_group.localAgents()){
                if(((AgentPopulation*) agent)->getState() == INFECTED) infected++;
            }
            return infected;
        };

        fpmas::io::Watcher<int> count_susceptible = [&move_group] (){
            int susceptible = 0;
            for(auto agent : move_group.localAgents()){
                if(((AgentPopulation*) agent)->getState() == SUSCEPTIBLE) susceptible++;
            }
            return susceptible;
        };

        fpmas::io::Watcher<int> count_removed = [&move_group] (){
            int removed = 0;
            for(auto agent : fpmas::utils::ptr_wrapper_cast<AgentPopulation>(move_group.localAgents())){
                if(((AgentPopulation*) agent)->getState() == RECOVERED) removed++;
            }
            return removed;
        };

        fpmas::io::Watcher<int> count_dead = [&die_group] (){
            return die_group.localAgents().size();
        };



        // Random uniform mapping for the built grid
        UniformGridAgentMapping mapping(
            grid_builder.width(), grid_builder.height(),
            nombreAgent
        );

        //Build agent instances with a 'new AgentPopulation' statement
        DefaultSpatialAgentFactory<AgentPopulation> factory; 
        
        //Initializes GridAgentExamples on the grid (distribued process)/
        //All agents are automatically added on to the move_group
        GridAgentBuilder<>().build(
            model, {move_group}, factory, mapping
        );
        model.graph().synchronize();
        
        using fpmas::io::Reduce;
        using fpmas::io::Local;

        fpmas::io::DistributedCsvOutput<
			Local<fpmas::scheduler::TimeStep>, // Time step (local field)
            Reduce<int>,
            Reduce<int>,
			Reduce<int>,
            Reduce<int> 
		> csv_output( model.getMpiCommunicator(), 0 /* root process */, out,
					{"T", [&model] () {
						// Current time step
						return fpmas::scheduler::time_step(model.runtime().currentDate());
						}},
                    {"S", count_susceptible},
					{"I", count_infected},
                    {"R", count_removed},
                    {"D", count_dead}
		);
        
        //Select nb_infected_agent_begin random agent and put him on state infected

        

            
        
        int nb_total = nb_infected_agent_begin/model.getMpiCommunicator().getSize();
        if(nb_infected_agent_begin%model.getMpiCommunicator().getSize() != 0){
            FPMAS_ON_PROC(model.getMpiCommunicator(), 0){
                nb_total++;
            }
        }
        for (size_t i = 0; i < nb_total; i++)
        {
            fpmas::random::random_device r2d2;
            fpmas::random::UniformIntDistribution <> numeroAgent(0,move_group.localAgents().size()-1);//Entre 0 et le nombre d'agent possible
            AgentPopulation *agent = fpmas::utils::ptr_wrapper_cast<AgentPopulation>(move_group.localAgents().at(numeroAgent(r2d2)));
            while(agent->getState() == INFECTED){
                fpmas::random::UniformIntDistribution <> numeroAgent(0,move_group.localAgents().size()-1);//Entre 0 et le nombre d'agent possible
                agent = fpmas::utils::ptr_wrapper_cast<AgentPopulation>(move_group.localAgents().at(numeroAgent(r2d2)));
            }
            agent->setState(INFECTED);
        }
        
        
        
        
        
        

        //Schedules AgentPopulation exectution
        model.scheduler().schedule(0,1,model.loadBalancingJob());
        
        model.scheduler().schedule(0.1, 1, move_group.jobs());

        model.scheduler().schedule(0.2, 1, csv_output.job());

        model.scheduler().schedule(0.3, 1, die_group.jobs());

        // Runs the model for 10 iterations
        model.runtime().run(runtime);
    }
    fpmas::finalize();
    return 0;
}
