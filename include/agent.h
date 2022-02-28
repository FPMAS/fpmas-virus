#include "fpmas.h"
#include "fpmas/model/serializer.h"
#include "fpmas/model/model.h"

using namespace fpmas::model;

FPMAS_DEFINE_GROUPS(MOVE, DIE, BIRTH);

enum State { Susceptible, Infected, Recovered, Dead};

std::string printState(State s);

class AgentPopulation : public GridAgent<AgentPopulation> {
	//Taux de rémission
	
    private:
        static const MooreRange<MooreGrid<>> range;
		State state;
		int age;
		bool isDead;

    public:
		static double alpha;
		static double beta;
		static double mortality_rate;
		static int life_expectancy;
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
		AgentPopulation(State cState, int cYearsState, bool deadOrAlive);

		/**
		 * print id with state color
		 */
		void printIDWithStateColor();

		/**
		 * Agent behavior.
		 */
		void move_type_read();
		

		/**
		 * Agent behavior written.
		 */
		void move_type_written();

		/**
		 * Print an agent 
		 */
		void printAgent();

		/**
		 * State of AgentPopulation
		 */
		State getState();

		/**
		 * Change State of AgentPopulation
		 */
		void setState(State s);

		/**
		 * get CurrentBeta
		 */
		double getBeta();

		/**
		 * get if agent is Dead or not
		 */
		bool getDead();

		/**
		 * Calcul of infection probability 
		 */
		double calculInfectionProbality();

		/**
		 * See if you get desease or not
		 */
		void infection();

		/**
		 * See if you get desease or not
		 */
		void tryInfectOtherAgent();

		/**
		 * Allows to get the number of neighbour infected
		 */
		int getNumberOfNeighbourInfected();

		/**
		 * Allows to test if you pass from State Infected to State Recovered
		 */
		void recovery();


		/**
		 * Check if you die from infection, and if so, remove you from model
		 */
		void dying();
		

		/**
		 * Calcul the chance that you live according to your age and life_expectancy
		 */
		double getChanceToDie();

		/**
		 * beahavior for dying group
		 */
		void die_behave();

		/**
		 * behavior for birth group
		 */
		void give_birth();

		

		
		static void to_json(nlohmann::json& j, const AgentPopulation * agent);
		static AgentPopulation* from_json(const nlohmann::json& j);
};

