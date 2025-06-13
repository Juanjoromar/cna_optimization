#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "network_model/node.hpp"

#include <memory>
#include <cstdint>
#include <vector>
#include <map>

#include <nlohmann/json.hpp>

struct NodePair {
    Node* n1;
    Node* n2;
    double entanglement_fidelity_capacity;

    NodePair(Node* n1, Node* n2):
        n1(n1),
        n2(n2) {
        entanglement_fidelity_capacity = n1->entanglement_fidelity_capacity() < n2->entanglement_fidelity_capacity()? 
            n1->entanglement_fidelity_capacity() : n2->entanglement_fidelity_capacity();
    }

    bool operator<(const NodePair& other) const {
        if (n1->_node_name < other.n1->_node_name) return true;
        if (n1->_node_name > other.n1->_node_name) return false;
        if (n2->_node_name < other.n2->_node_name) return true;
        else return false;
    }
};

struct LambdaCfg {
    double lambda;
    double max_for_lambda;
};

extern std::string network_topology_config_file;

class Network {
public:

    Network();
    Network(const Network& other);
    Network(Network&& other) = default;
    Network(const char* config_json_file);
    Network(nlohmann::json config_json);

    ~Network() = default;

    Network& operator=(const Network& other);

    void init_network_random_instruction_tables(const std::function<double(void)>& rnd01);

    void add_shared_entanglement(NodePair&& node_pair, double entanglement) const;
    void advance_n_time_slots(std::size_t n) const;
    void reset() const;

    void mutate(const std::function<double(void)>& rnd01);

    void print() const;

    std::vector<NodePair> pairs() const { return _node_pairs; }

    double pair_shared_entanglement(const NodePair& pair) const { return _shared_entanglement.at(pair); }
    double amount_entanglement() const;
    double total_entanglement() const;

    nlohmann::json to_json() const;

private:
    std::vector<std::unique_ptr<Node>> _nodes;

    std::vector<NodePair> _node_pairs;
    std::map<NodePair, LambdaCfg> _node_pair_weight;
    mutable std::map<NodePair, double> _shared_entanglement;

    mutable std::size_t _total_time_slots = 0;
    std::uint32_t _time_slots_duration_nanoseconds = 1e6;

    void compute_next_time_slot() const;
};

#endif //NETWORK_HPP