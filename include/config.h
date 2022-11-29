#include "yaml-cpp/yaml.h"

/**
 * @file config.h
 * Defines YAML configuration features.
 */

/**
 * Defines the infection mode used by AgentPopulation::behavior().
 */
enum InfectionMode {

	/**
	 * No write operations between agents is performed so all SyncModes can
	 * safely be used.
	 */
	READ,

	/**
	 * The WRITE mode might be more realistic, but requires the `HARD_SYNC` so
	 * that write operations across processes can be performed.
	 */
	WRITE
};

/**
 * Defines the FPMAS synchronization mode used to simulate the Virus model.
 */
enum SyncMode {
	/**
	 * Only DISTANT agents are read from a ghost copy.
	 * Write operations across processes are not handled.
	 */
	GHOST,
	/**
	 * All agents are read from a ghost copy.
	 * Write operations are not handled.
	 */
	GLOBAL_GHOST,
	/**
	 * DISTANT agents are read and written directly from distant processes.
	 */
	HARD_SYNC
};

namespace YAML {
	template<>
		struct convert<InfectionMode> {
			static Node encode(const InfectionMode& mode) {
				Node node;
				switch(mode) {
					case READ:
						node = "READ";
						break;
					case WRITE:
						node = "WRITE";
				}
				return node;
			}

			static bool decode(const Node& node, InfectionMode& mode) {
				if(!node.IsScalar()) {
					return false;
				}
				if(node.as<std::string>() == "READ")
					mode = READ;
				else if(node.as<std::string>() == "WRITE")
					mode = WRITE;
				else
					return false;
				return true;
			}
		};

	template<>
		struct convert<SyncMode> {
			static Node encode(const SyncMode& mode) {
				Node node;
				switch(mode) {
					case GHOST:
						node = "GHOST";
						break;
					case GLOBAL_GHOST:
						node = "GLOBAL_GHOST";
						break;
					case HARD_SYNC:
						node = "HARD_SYNC";
				}
				return node;
			}

			static bool decode(const Node& node, SyncMode& mode) {
				if(!node.IsScalar()) {
					return false;
				}
				if(node.as<std::string>() == "GHOST")
					mode = GHOST;
				else if(node.as<std::string>() == "GLOBAL_GHOST")
					mode = GLOBAL_GHOST;
				else if(node.as<std::string>() == "HARD_SYNC")
					mode = HARD_SYNC;
				else
					return false;
				return true;
			}
		};

}
