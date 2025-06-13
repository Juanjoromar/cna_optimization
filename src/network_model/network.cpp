#include "network_model/network.hpp"

#include "network_model/node.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>

static const LambdaCfg lambda_cfg{2.84777403440728, 0.6831071864140762}; //For the moment is harcoded

Network::Network() {
    using json = nlohmann::json;
    std::ifstream json_config_file(network_topology_config_file);

    json network_cfg = json::parse(json_config_file);

    //Creates all nodes
    _nodes.reserve(network_cfg["nodes"].size());
    for (auto& element : network_cfg["nodes"].items()) {
        _nodes.push_back(std::make_unique<Node>(Node(this, element.key())));
        _nodes.back()->initialize_node(element.value());
    }
    //Emptying all instruction_tables
    for (std::unique_ptr<Node>& node : _nodes)
        node->reset_instruction_table(std::vector<TimeSlot>(1, TimeSlot(node.get(), std::vector<SwapPair>())));

    //Initialize the pairs
    std::size_t n_nodes = _nodes.size();
    std::size_t n_pairs = n_nodes*(n_nodes - 1)/2;
    
    _node_pairs.reserve(n_pairs);
    for (std::size_t i = 0; i < n_nodes - 1; ++i)
    for (std::size_t j = i + 1; j < n_nodes; ++j) {
            _node_pairs.push_back(NodePair{_nodes[i].get(), _nodes[j].get()});
            _node_pair_weight[_node_pairs.back()] = lambda_cfg;
            _shared_entanglement[_node_pairs.back()] = 0;
        }
    //Config all links
    for (auto& element : network_cfg["links"].items()) {
        Node* node1, * node2;
        
        std::string node1_name, node2_name;

        std::istringstream link_name(element.key());
        std::getline(link_name, node1_name, '-');
        std::getline(link_name, node2_name);

        for (const std::unique_ptr<Node>& node : _nodes) {
            if (node->_node_name == node1_name)
                node1 = node.get();

            else if (node->_node_name == node2_name)
                node2 = node.get();
        }

        std::string interface1, interface2;

        std::istringstream interfaces(element.value()["interfaces"].get<std::string>());

        std::getline(interfaces, interface1, '-');
        std::getline(interfaces, interface2);
        
        pair((*node1)[std::stoul(interface1)], (*node2)[std::stoul(interface2)]);
        (*node1)[std::stoul(interface1)]->reset_entangled_pair();
        (*node2)[std::stoul(interface2)]->reset_entangled_pair();
    }
}

Network::Network(const Network& other):
    _total_time_slots(other._total_time_slots),
    _time_slots_duration_nanoseconds(other._time_slots_duration_nanoseconds) {
    
    nlohmann::json network_cfg = other.to_json();

    //Creates all nodes
    _nodes.reserve(network_cfg["nodes"].size());
    for (auto& element : network_cfg["nodes"].items()) {
        _nodes.push_back(std::make_unique<Node>(Node(this, element.key())));
        _nodes.back()->initialize_node(element.value());
    }

    //Initialize the pairs
    std::size_t n_nodes = _nodes.size();
    _node_pairs.reserve(n_nodes*(n_nodes - 1)/2);
    for (std::size_t i = 0; i < n_nodes - 1; ++i)
    for (std::size_t j = i + 1; j < n_nodes; ++j) {
            _node_pairs.push_back(NodePair{_nodes[i].get(), _nodes[j].get()});
            _node_pair_weight[_node_pairs.back()] = other._node_pair_weight.at(NodePair{other._nodes[i].get(), other._nodes[j].get()});
            _shared_entanglement[_node_pairs.back()] = other._shared_entanglement.at(NodePair{other._nodes[i].get(), other._nodes[j].get()});
        }
    //Config all links
    for (auto& element : network_cfg["links"].items()) {
        Node* node1, * node2;
        
        std::string node1_name, node2_name;

        std::istringstream link_name(element.key());
        std::getline(link_name, node1_name, '-');
        std::getline(link_name, node2_name);

        for (const std::unique_ptr<Node>& node : _nodes) {
            if (node->_node_name == node1_name)
                node1 = node.get();

            else if (node->_node_name == node2_name)
                node2 = node.get();
        }

        std::string interface1, interface2;

        std::istringstream interfaces(element.value()["interfaces"].get<std::string>());

        std::getline(interfaces, interface1, '-');
        std::getline(interfaces, interface2);
        
        pair((*node1)[std::stoul(interface1)], (*node2)[std::stoul(interface2)]);
        (*node1)[std::stoul(interface1)]->reset_entangled_pair();
        (*node2)[std::stoul(interface2)]->reset_entangled_pair();
    }
}

Network::Network(const char* config_json_file) {
    using json = nlohmann::json;
    std::ifstream json_config_file(config_json_file);

    json network_cfg = json::parse(json_config_file);

    //Creates all nodes
    _nodes.reserve(network_cfg["nodes"].size());
    for (auto& element : network_cfg["nodes"].items()) {
        _nodes.push_back(std::make_unique<Node>(Node(this, element.key())));
        _nodes.back()->initialize_node(element.value());
    }

    //Initialize the pairs
    std::size_t n_nodes = _nodes.size();
    std::size_t n_pairs = n_nodes*(n_nodes - 1)/2;
    _node_pairs.reserve(n_pairs);
    for (std::size_t i = 0; i < n_nodes - 1; ++i)
    for (std::size_t j = i + 1; j < n_nodes; ++j) {
            _node_pairs.push_back(NodePair{_nodes[i].get(), _nodes[j].get()});
            _node_pair_weight[_node_pairs.back()] = lambda_cfg;
            _shared_entanglement[_node_pairs.back()] = 0;
        }
    //Config all links
    for (auto& element : network_cfg["links"].items()) {
        Node* node1, * node2;
        
        std::string node1_name, node2_name;

        std::istringstream link_name(element.key());
        std::getline(link_name, node1_name, '-');
        std::getline(link_name, node2_name);

        for (const std::unique_ptr<Node>& node : _nodes) {
            if (node->_node_name == node1_name)
                node1 = node.get();

            else if (node->_node_name == node2_name)
                node2 = node.get();
        }

        std::string interface1, interface2;

        std::istringstream interfaces(element.value()["interfaces"].get<std::string>());

        std::getline(interfaces, interface1, '-');
        std::getline(interfaces, interface2);
        
        pair((*node1)[std::stoul(interface1)], (*node2)[std::stoul(interface2)]);
        (*node1)[std::stoul(interface1)]->reset_entangled_pair();
        (*node2)[std::stoul(interface2)]->reset_entangled_pair();
    }

}

Network::Network(nlohmann::json config_json) {
    //Creates all nodes
    _nodes.reserve(config_json["nodes"].size());
    for (auto& element : config_json["nodes"].items()) {
        _nodes.push_back(std::make_unique<Node>(Node(this, element.key())));
        _nodes.back()->initialize_node(element.value());
    }
    //std::cout << config_json["nodes"].dump();

    //Initialize the pairs
    std::size_t n_nodes = _nodes.size();
    std::size_t n_pairs = n_nodes*(n_nodes - 1)/2;
    _node_pairs.reserve(n_pairs);
    for (std::size_t i = 0; i < n_nodes - 1; ++i)
    for (std::size_t j = i + 1; j < n_nodes; ++j) {
            _node_pairs.push_back(NodePair{_nodes[i].get(), _nodes[j].get()});
            _node_pair_weight[_node_pairs.back()] = lambda_cfg;
            _shared_entanglement[_node_pairs.back()] = 0;
        }
    //Config all links
    for (auto& element : config_json["links"].items()) {
        Node* node1, * node2;
        
        std::string node1_name, node2_name;

        std::istringstream link_name(element.key());
        std::getline(link_name, node1_name, '-');
        std::getline(link_name, node2_name);

        for (const std::unique_ptr<Node>& node : _nodes) {
            if (node->_node_name == node1_name)
                node1 = node.get();

            else if (node->_node_name == node2_name)
                node2 = node.get();
        }

        std::string interface1, interface2;

        std::istringstream interfaces(element.value()["interfaces"].get<std::string>());

        std::getline(interfaces, interface1, '-');
        std::getline(interfaces, interface2);
        
        pair((*node1)[std::stoul(interface1)], (*node2)[std::stoul(interface2)]);
        (*node1)[std::stoul(interface1)]->reset_entangled_pair();
        (*node2)[std::stoul(interface2)]->reset_entangled_pair();
    }
}

Network& Network::operator=(const Network& other) {

    _total_time_slots = other._total_time_slots;
    _time_slots_duration_nanoseconds = other._time_slots_duration_nanoseconds;

    nlohmann::json network_cfg = other.to_json();
    
    //Creates all nodes
    _nodes.clear();
    _nodes.reserve(network_cfg["nodes"].size());
    for (auto& element : network_cfg["nodes"].items()) {
        _nodes.push_back(std::make_unique<Node>(Node(this, element.key())));
        _nodes.back()->initialize_node(element.value());
    }

    //Initialize the pairs
    std::size_t n_nodes = _nodes.size();
    _node_pairs.clear();
    _node_pairs.reserve(n_nodes*(n_nodes - 1)/2);
    _node_pair_weight.clear();
    _shared_entanglement.clear();
    for (std::size_t i = 0; i < n_nodes - 1; ++i)
    for (std::size_t j = i + 1; j < n_nodes; ++j) {
            _node_pairs.push_back(NodePair{_nodes[i].get(), _nodes[j].get()});
            _node_pair_weight[_node_pairs.back()] = other._node_pair_weight.at(NodePair{other._nodes[i].get(), other._nodes[j].get()});
            _shared_entanglement[_node_pairs.back()] = other._shared_entanglement.at(NodePair{other._nodes[i].get(), other._nodes[j].get()});
        }
    //Config all links
    for (auto& element : network_cfg["links"].items()) {
        Node* node1, * node2;
        
        std::string node1_name, node2_name;

        std::istringstream link_name(element.key());
        std::getline(link_name, node1_name, '-');
        std::getline(link_name, node2_name);

        for (const std::unique_ptr<Node>& node : _nodes) {
            if (node->_node_name == node1_name)
                node1 = node.get();

            else if (node->_node_name == node2_name)
                node2 = node.get();
        }

        std::string interface1, interface2;

        std::istringstream interfaces(element.value()["interfaces"].get<std::string>());

        std::getline(interfaces, interface1, '-');
        std::getline(interfaces, interface2);
        
        pair((*node1)[std::stoul(interface1)], (*node2)[std::stoul(interface2)]);
        (*node1)[std::stoul(interface1)]->reset_entangled_pair();
        (*node2)[std::stoul(interface2)]->reset_entangled_pair();
    }

    return *this;
}

void Network::init_network_random_instruction_tables(const std::function<double(void)>& rnd01) {
    for (std::unique_ptr<Node>& node : _nodes) {
        std::size_t n_time_slots = static_cast<std::size_t>(rnd01() * 9) + 1;
        std::vector<TimeSlot> time_slots;
        time_slots.reserve(n_time_slots);
        
        for (std::size_t i_time_slot = 0; i_time_slot < n_time_slots; ++i_time_slot) {
            std::vector<std::shared_ptr<EntanglementInterface>> interfaces = node->get_interfaces();
            std::vector<SwapPair> swap_instructions;

            while (interfaces.size() > 1) {
                std::size_t interface_n = static_cast<std::size_t>(rnd01() * interfaces.size());
                std::shared_ptr<EntanglementInterface> interface = interfaces[interface_n];
                interfaces.erase(std::find(interfaces.begin(), interfaces.end(), interface));
                if (rnd01() > 0.3) { //We create a Swap
                    std::shared_ptr<EntanglementInterface> interface2 = interfaces[static_cast<std::size_t>(rnd01() * interfaces.size())];
                    interfaces.erase(std::find(interfaces.begin(), interfaces.end(), interface2));
                    swap_instructions.push_back(SwapPair(interface, interface2));
                }
            }

            time_slots.push_back(TimeSlot(node.get(), swap_instructions));
        }

        node->reset_instruction_table(time_slots);
    }
}

void Network::add_shared_entanglement(NodePair&& node_pair, double entanglement) const {
    if (_shared_entanglement.find(node_pair) == _shared_entanglement.end())
        std::swap(node_pair.n1, node_pair.n2);
    _shared_entanglement[node_pair] += entanglement;
}

void Network::advance_n_time_slots(std::size_t n) const {
    for (std::size_t i = 0; i < n; ++i) {
        this->compute_next_time_slot();
        ++_total_time_slots;
    }
}

void Network::reset() const {
    for (const NodePair& pair : _node_pairs)
        _shared_entanglement[pair] = 0;
    
    _total_time_slots = 0;
}

void Network::mutate(const std::function<double(void)>& rnd01) {
    for (std::unique_ptr<Node>& node : _nodes) {
        if (rnd01() < 0.8)
            node->mutate(rnd01);
    }
}

double Network::amount_entanglement() const {
    double amount_of_entanglement = 0;
    for (NodePair pair : _node_pairs) {
        double shared_entanglement = _shared_entanglement.at(pair);
        if (shared_entanglement <= 0.0) continue;
        LambdaCfg lambda_cfg = _node_pair_weight.at(pair);
        amount_of_entanglement += exp(-lambda_cfg.lambda)*pow(lambda_cfg.lambda, shared_entanglement) / (std::tgamma(shared_entanglement) * lambda_cfg.max_for_lambda);
    }
    return amount_of_entanglement;
}

double Network::total_entanglement() const {
    double total_entanglement = 0;
    for (NodePair pair : _node_pairs)
        total_entanglement += _shared_entanglement.at(pair);
    return total_entanglement;
}

void Network::compute_next_time_slot() const {
    for (const NodePair& node_pair : _node_pairs)
        _shared_entanglement[node_pair] *= node_pair.entanglement_fidelity_capacity;
    for (const std::unique_ptr<Node>& node : _nodes)
        node->compute_next_time_slot(_time_slots_duration_nanoseconds);

}

nlohmann::json Network::to_json() const {
    using json = nlohmann::json;

    json network_json;

    for (const std::unique_ptr<Node>& node : _nodes) {

        json time_slots;

        for (const TimeSlot& time_slot : node->time_slots()) {
            json time_slot_json;
            for (const Swap& swap : time_slot.swap_instructions())
                time_slot_json.push_back(swap.swap_pair().to_string());
            time_slots.push_back(time_slot_json);
        }
        

        network_json["nodes"][node->_node_name] = {
            {"swap_probability", node->swap_probability()},
            {"entanglement_interfaces", node->n_interfaces()},
            {"time_slots", time_slots}
        };

        for (const std::shared_ptr<EntanglementInterface>& interface : node->get_interfaces()) {
            std::string link_name = interface->context()->_node_name + "-" + interface->fibre_pair().lock()->context()->_node_name;
            if (network_json["links"].contains(interface->fibre_pair().lock()->context()->_node_name + "-" + interface->context()->_node_name)) continue;
            network_json["links"][link_name] = {
                {"interfaces", std::to_string(interface->n_interface())+"-"+std::to_string(interface->fibre_pair().lock()->n_interface())}
            };
        }
    }
    return network_json;
}

void Network::print() const {
    for (const std::unique_ptr<Node>& node : _nodes)
        node->print();
    for (const NodePair& pair : _node_pairs) {
        std::cout << "Pair " << pair.n1->_node_name << "-" << pair.n2->_node_name
            << " shares: " << _shared_entanglement.at(pair) << " (expected number of entangled pairs)\n";
    }
    std::cout << "Total entanglement is: " << this->total_entanglement()
    << " and the amount of entanglement is: " << this->amount_entanglement() << "\n";
}