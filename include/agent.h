#ifndef VIRUS_AGENT_H
#define VIRUS_AGENT_H

#include "fpmas.h"
#include "config.h"

using namespace fpmas::model;

FPMAS_DEFINE_GROUPS(ALIVE_GROUP, DEAD_GROUP);

enum State { SUSCEPTIBLE, INFECTED, RECOVERED, DEAD};

class AgentPopulation : public GridAgent<AgentPopulation> {
	private:
		static const MooreRange<MooreGrid<>> range;
		State state;

	public:
		static double recover_rate;
		static double infection_rate;
		static double mortality_rate;

		FPMAS_MOBILITY_RANGE(range);
		FPMAS_PERCEPTION_RANGE(range);

		/**
		 * Empty Constructor
		 */
		AgentPopulation();

		/**
		 * Constructor
		 */
		AgentPopulation(State state);

		float random() {
			static fpmas::random::UniformRealDistribution<float> rd_float(0, 1);
			return rd_float(this->rd());
		}

		void move();

		template<InfectionMode mode>
			void behavior();

		/**
		 * State of AgentPopulation
		 */
		State getState() const;

		/**
		 * Change State of AgentPopulation
		 */
		void setState(State s);

		/**
		 * Allows to test if you pass from State Infected to State Recovered
		 */
		void recover();


		/**
		 * Check if you die from infection, and if so, remove you from model
		 */
		void die();

		static std::size_t size(
				const fpmas::io::datapack::ObjectPack& p,
				const AgentPopulation* agent);
		static void to_datapack(
				fpmas::io::datapack::ObjectPack& p,
				const AgentPopulation* agent);
		static AgentPopulation* from_datapack(const fpmas::io::datapack::ObjectPack& p);
};

template<>
void AgentPopulation::behavior<InfectionMode::READ>();
template<>
void AgentPopulation::behavior<InfectionMode::WRITE>();

namespace fpmas { namespace io { namespace datapack {
	template<>
		struct Serializer<State> {
			static std::size_t size(const ObjectPack& o, const State& s);
			static void to_datapack(ObjectPack& o, const State& s);
			static State from_datapack(const ObjectPack& o);
		};
}}}
#endif
