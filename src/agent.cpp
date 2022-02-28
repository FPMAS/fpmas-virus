#include <math.h>
#include "../include/agent.h"

const std::string red("\033[0;31m");
const std::string green("\033[1;32m");
const std::string reset("\033[0m");

//---------------------------------------------AGENT------------------------------------------------------------


std::string printState(State s){
	switch (s)
	{
	case 0:
		return "Susceptible";
		break;
	case 1:
		return "Infected";
		break;
	default:
		break;
	}
	return "Removed";
}

double full_random(int min, int max){
	fpmas::random::random_device rd;
	fpmas::random::UniformRealDistribution<> real_distribution(min,max);
	return real_distribution(rd);
}

const MooreRange<MooreGrid<>> AgentPopulation::range(1);
double AgentPopulation::alpha = 0.0;
double AgentPopulation::beta = 0.0;
double AgentPopulation::mortality_rate = 0;
int AgentPopulation::life_expectancy = 0;
double AgentPopulation::reproduction_rate = 0.0;

AgentPopulation::AgentPopulation(){
	this->state = State::Susceptible;
	this->age = 0;
	this->isDead = false;
}

AgentPopulation::AgentPopulation(State cState, int cYearsState, bool deadOrAlive) : state(cState), age(cYearsState), isDead(deadOrAlive){}

void AgentPopulation::printIDWithStateColor(){
	
	if(this->getState() == State::Infected){
		std::cout << red << this->node()->getId() << reset << " ";
	}
	else{
		if(this->getState() == State::Recovered){
			std::cout << green << this->node()->getId() << reset << " ";
		}else{
			std::cout << this->node()->getId() <<  " ";
		}
	}
}

void AgentPopulation::printAgent(){
	std::cout
		<< "Agent ";
		this->printIDWithStateColor();
		std::cout << ":" << std::endl
		<< "	State: " << printState(this->getState()) << std::endl
		<< "    Step: " << this->model()->runtime().currentDate() << std::endl
		<< "    Location: " << this->locationCell()->location() << std::endl // Faire un readguard sur this->locationCell
		<< "    Perceptions: [ ";
	for(auto agent : this->perceptions<AgentPopulation>()){
		if(agent->getState() != State::Dead) agent->printIDWithStateColor();
	}	
		
	std::cout << "]" << std::endl;
	
}

void AgentPopulation::move_type_read() {
	//printAgent();
	if(this->getState() == State::Susceptible) this->infection();
	else if(this->getState() == State::Infected){
		this->recovery(); 
	}
	dying();

	// Gets GraphCells currently in the agent mobility field.
	// In this examples, this represents the 8 Moore neighbor cells, and the
	// current location cell.
	auto mobility_field = this->mobilityField();

	// Selects a random GridCell from the mobility field
	auto cell = mobility_field.random();

	// Moves to this cell
	this->moveTo(cell);

	this->age ++;
	
}

void AgentPopulation::move_type_written(){
	//printAgent();
	dying();
	if(this->getState() != State::Dead){
		if(this->getState() == State::Infected){
		tryInfectOtherAgent();
		recovery();//Nombre de pas de temps après lequel il peut essayer de ne plus etre malade
	}
	

		// Gets GraphCells currently in the agent mobility field.
		// In this examples, this represents the 8 Moore neighbor cells, and the
		// current location cell.
		auto mobility_field = this->mobilityField();

		// Selects a random GridCell from the mobility field
		auto cell = mobility_field.random();

		// Moves to this cell
		this->moveTo(cell);

		this->age ++;
	}
}

State AgentPopulation::getState() {
	fpmas::model::ReadGuard read(this);	
	return this->state;
}

void AgentPopulation::setState(State s){
	fpmas::model::AcquireGuard acquire(this);
	this->state = s;
}

double AgentPopulation::getBeta(){
	return this->beta;
}

bool AgentPopulation::getDead(){
	return this->isDead;
}

double AgentPopulation::calculInfectionProbality(){
	int numberInfected = this->getNumberOfNeighbourInfected();
	double res = 0;
	/*int cptWithoutDead = 0;
	for(auto agent : this->perceptions<AgentPopulation>())
		if(agent->getState() != State::Dead) cptWithoutDead++;*/
	if(numberInfected > 0){
		res = 1 - pow(1-beta,numberInfected);//(beta*numberInfected)/cptWithoutDead;
	}
	
	return res;
}


int AgentPopulation::getNumberOfNeighbourInfected() {
	int res = 0;
	
	for (auto agent : this->perceptions<AgentPopulation>())
	{	
		if(agent->getState()  == State::Infected){//Acquire in getState
			res++;
		}
	}
	
	return res;
}


void AgentPopulation::infection(){
	//Random
	float random = full_random(0,1);
	double taux = calculInfectionProbality(); 
	if(random <  taux){
		//std::cout << "Random = " << random << " et je suis inférieur à " << taux << std::endl;
		this->setState(State::Infected); //Acquire in setState	
	}
}

void AgentPopulation::tryInfectOtherAgent(){
	
	for(auto agent : this->perceptions<AgentPopulation>()){
		
		if(agent->getState() == State::Susceptible){
			//Random
			float random = full_random(0,1);
			
			if(random < beta){
				agent->setState(State::Infected);
			}
		}	
	}
}

void AgentPopulation::die_behave(){
	if(this->getState() != State::Dead) this->setState(State::Dead);
}


void AgentPopulation::recovery(){
	float random = full_random(0,1);
	if(random < alpha){
	 	this->setState(State::Recovered); //Acquire dans le setState
	} 
}


double AgentPopulation::getChanceToDie(){ //TODO Refaire avec de meilleur valeur et si possible un vrai calcul 
	if(age >= life_expectancy && life_expectancy != 0) return 1;
	return 0;
}


void AgentPopulation::dying(){
	if(this->getState() == State::Infected){
		float random = full_random(0,1);
		if(random < mortality_rate){
			isDead = true;
		}
	}
	
	if(!isDead){
		double chanceOfDying = this->getChanceToDie();
		if(full_random(0,1) <= chanceOfDying){ 
			isDead = true;
		}
	}
	
	if(isDead){
		this->model()->getGroup(DIE).add(this);
		this->model()->getGroup(MOVE).remove(this);
		this->model()->getGroup(BIRTH).remove(this);
		this->setState(State::Dead);
	}
		
}

void AgentPopulation::give_birth(){
	if(this->getState() != State::Infected && this->age >= 5){ // 5 car C'est le 14 mai 1939 que la péruvienne Lina Medina est devenue la plus jeune maman du monde, à l'âge de 5 ans. 
		float random = full_random(0,1);
		if(random < reproduction_rate){
			AgentPopulation *newAgent = new AgentPopulation();
			this->model()->getGroup(MOVE).add(newAgent);
			this->model()->getGroup(BIRTH).add(newAgent);
			newAgent->initLocation(this->locationCell());
		}
	}
}


void AgentPopulation::to_json(nlohmann::json& j, const AgentPopulation * agent){
	j["s"] = agent->state;
	j["a"] = agent->age;
	j["d"] = agent->isDead;
}

AgentPopulation* AgentPopulation::from_json(const nlohmann::json& j){
	return new AgentPopulation(
		j.at("s").get<State>(),
		j.at("a").get<int>(),
		j.at("d").get<bool>()
	);
}