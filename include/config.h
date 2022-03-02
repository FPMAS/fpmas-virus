#include "yaml-cpp/yaml.h"

enum InfectionMode {
	READ, WRITE
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

}
