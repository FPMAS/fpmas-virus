#ifndef VIRUS_AGENT_H
#define VIRUS_AGENT_H

#include "fpmas.h"
#include "config.h"

using namespace fpmas::model;

/**
 * @file agent.h
 * AgentPopulation implementation.
 */

/**
 * Defines two agent groups:
 * - ALIVE_GROUP: leaving agents, executing AgentPopulation::behavior()
 * - DEAD_GROUP: dead agents, do nothing, used only to count dead agents
 */
FPMAS_DEFINE_GROUPS(ALIVE_GROUP, DEAD_GROUP);

/**
 * Describes the infection state of an AgentPopulation.
 */
enum State { SUSCEPTIBLE, INFECTED, RECOVERED, DEAD};

/**
 * The Agent representing the studied population.
 *
 * The agent moves on a grid, with a mobility and perception range corresponding
 * to its Moore neighborhood.
 */
class AgentPopulation : public GridAgent<AgentPopulation> {
	private:
		static const MooreRange<MooreGrid<>> range;
		State state;

	public:
		/**
		 * Probability to recover at each time step when INFECTED.
		 */
		static double recover_rate;
		/**
		 * Probability to infect a SUSCEPTIBLE neighbor at each time step when
		 * INFECTED.
		 */
		static double infection_rate;
		/**
		 * Probability to die at each time step when INFECTED.
		 */
		static double mortality_rate;

		/**
		 * The agent mobility range is a MooreRange of size 1.
		 */
		FPMAS_MOBILITY_RANGE(range);

		/**
		 * The agent perception range is a MooreRange of size 1.
		 */
		FPMAS_PERCEPTION_RANGE(range);

		/**
		 * Default constructor.
		 */
		AgentPopulation();

		/**
		 * Initialises an agent with the specified state.
		 *
		 * @param state current state of the agent
		 */
		AgentPopulation(State state);


		/**
		 * Gets the infection state of the agent.
		 */
		State getState() const;

		/**
		 * Sets the infection state of the agent.
		 */
		void setState(State s);

		/**
		 * Returns a random number in [0, 1) from the random number generator
		 * embedded in the agent. This allows to reach strict reproducibility
		 * independently from the distribution or process count.
		 *
		 * @return deterministic random number in [0, 1)
		 */
		float random();

		/**
		 * Randomly moves the agent to a cell in its Moore neighborhood.
		 *
		 * The location is only updated at the next time step, when FPMAS
		 * synchronizes the MoveAgentGroup.
		 */
		void move();

		/**
		 * Sets the agent state to RECOVER with a probability #recover_rate,
		 * using the internal random() generator.
		 */
		void recover();

		/**
		 * Sets the agent state to DEAD with a probability #mortality_rate,
		 * using the internal random() generator.
		 *
		 * If the agent dies, it is added to the DEAD_GROUP and removed from the
		 * ALIVE_GROUP as it is not supposed to execute its behavior() anymore.
		 */
		void die();

		/**
		 * Main Agent behavior.
		 *
		 * The behavior of the agent can be described as follows:
		 * 1. Infection process
		 * 2. If INFECTED, try to recover()
		 * 3. If still INFECTED, maybe die()
		 * 4. If not DEAD, move()
		 *
		 * The infection process depends on the InfectionMode.
		 *
		 * If the InfectionMode is READ, then each SUSCEPTIBLE agent performs a
		 * Bernoulli experiment of parameter #infection_rate for each INFECTED
		 * neighbor, and changes its state to INFECTED if at least one
		 * experiment succeeds.
		 *
		 * If the InfectionMode is WRITE, then each INFECTED agent tries to
		 * infect each of its SUSCEPTIBLE neighbors with a probability
		 * #infection_rate.
		 */
		template<InfectionMode mode>
			void behavior();


		/**
		 * ObjectPack serialization size.
		 */
		static std::size_t size(
				const fpmas::io::datapack::ObjectPack& p,
				const AgentPopulation* agent);

		/**
		 * ObjectPack serialization.
		 */
		static void to_datapack(
				fpmas::io::datapack::ObjectPack& p,
				const AgentPopulation* agent);

		/**
		 * ObjectPack deserialization.
		 */
		static AgentPopulation* from_datapack(const fpmas::io::datapack::ObjectPack& p);
};

/**
 * AgentPopulation::behavior() implementation in read only mode.
 */
template<>
void AgentPopulation::behavior<InfectionMode::READ>();
/**
 * AgentPopulation::behavior() implementation with write operations.
 */
template<>
void AgentPopulation::behavior<InfectionMode::WRITE>();

namespace fpmas { namespace io { namespace datapack {
	/**
	 * Infection State serialization rules.
	 */
	template<>
		struct Serializer<State> {

			/**
			 * ObjectPack serialization size.
			 */
			static std::size_t size(const ObjectPack& o, const State& s);

			/**
			 * ObjectPack serialization.
			 */
			static void to_datapack(ObjectPack& o, const State& s);

			/**
			 * ObjectPack deserialization.
			 */
			static State from_datapack(const ObjectPack& o);
		};
}}}
#endif
