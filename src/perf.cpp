#include "perf.h"

Monitor monitor;
Probe total_time("total_time");
Probe init_model("init_model");
Probe init_grid("init_grid");
Probe init_agents("init_agents");
Probe init_infected("init_infected");
Probe init_lb("init_lb");
Probe run_model("run_model");

void commit_probes() {
	monitor.commit(total_time);
	monitor.commit(init_model);
	monitor.commit(init_grid);
	monitor.commit(init_agents);
	monitor.commit(init_infected);
	monitor.commit(init_lb);
	monitor.commit(run_model);
}

PerfOutput::PerfOutput(fpmas::api::io::OutputStream& output) 
	: fpmas::io::DistributedCsvOutput<
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>,
		Reduce<std::chrono::milliseconds, Max<std::chrono::milliseconds>>
		>(
				fpmas::communication::WORLD, 0, output,
				{total_time.label(), [] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(total_time.label())
						);
				}},
				{init_model.label(), [] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(init_model.label())
						);
				}},
				{init_grid.label(), [] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(init_grid.label())
						);
				}},
				{init_agents.label(), [] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(init_agents.label())
						);
				}},
				{init_infected.label(), [] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(init_infected.label())
						);
				}},
				{init_lb.label(), [] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(init_lb.label())
						);
				}},
				{run_model.label(), [] {return
				std::chrono::duration_cast<std::chrono::milliseconds>(
						monitor.totalDuration(run_model.label())
						);
				}}
	) {
	}

