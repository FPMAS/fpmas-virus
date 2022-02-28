#include "fpmas.h"
#include "fpmas/model/serializer.h"
#include "fpmas/model/model.h"

using namespace fpmas::model;

FPMAS_DEFINE_GROUPS(MOVE, DIE);

enum State { SUSCEPTIBLE, INFECTED, RECOVERED, DEAD};

class AgentPopulation : public GridAgent<AgentPopulation> {
	//Taux de rémission
	
    private:
        static const MooreRange<MooreGrid<>> range;
		State state;

    public:
		static double alpha;
		static double beta;
		static double mortality_rate;
		static double reproduction_rate;
		

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

		/**
		 * Agent behavior.
		 */
		void move_type_read();
		

		/**
		 * Agent behavior written.
		 */
		void move_type_written();

		/**
		 * State of AgentPopulation
		 */
		State getState();

		/**
		 * Change State of AgentPopulation
		 */
		void setState(State s);

		/**
		 * Allows to test if you pass from State Infected to State Recovered
		 */
		void recovery();


		/**
		 * Check if you die from infection, and if so, remove you from model
		 */
		void dying();
		
		static void to_json(nlohmann::json& j, const AgentPopulation * agent);
		static AgentPopulation* from_json(const nlohmann::json& j);
};

