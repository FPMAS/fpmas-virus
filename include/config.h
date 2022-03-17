#include "yaml-cpp/yaml.h"

enum InfectionMode {
	READ, WRITE
};

enum SyncMode {
	GHOST, GLOBAL_GHOST, HARD_SYNC
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
