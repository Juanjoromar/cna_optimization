#ifndef NODE_HPP
#define NODE_HPP

#include "network_model/instruction_table.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <vector>
#include <string>

class EntanglementInterface;
class Network;

class Node {
public:

    const std::string _node_name;

    Node(Network* network, std::string name): _node_name(name), _network(network) {}

    void initialize_node(nlohmann::json node_config);

    void reset_instruction_table(const std::vector<TimeSlot>& time_slots);

    void compute_next_time_slot(std::uint32_t time_slot_duration_nanoseconds);

    void mutate(const std::function<double(void)>& rnd01);

    void print() const;
    
    Network* network() const { return _network; }

    std::vector<std::shared_ptr<EntanglementInterface>> get_interfaces() const;
    std::size_t n_interfaces() const { return _entanglement_interfaces.size(); }

    double entanglement_fidelity_capacity() const { return _entanglement_fidelity_capacity; }
    std::uint32_t expected_swaps_per_nanosecond() const { return _expected_swaps_per_nanosecond; }
    double swap_probability() const { return _swap_probability; }

    std::vector<TimeSlot> time_slots() { return _instruction_table.time_slots(); }

    std::shared_ptr<EntanglementInterface> operator[](std::size_t n) { return _entanglement_interfaces[n]; }

    ~Node() = default;

private:
    
    Network* _network;
    std::vector<std::shared_ptr<EntanglementInterface>> _entanglement_interfaces;
    InstructionTable _instruction_table;
    double _entanglement_fidelity_capacity = 0.9;
    std::uint32_t _expected_swaps_per_nanosecond = 1000;
    double _swap_probability = 0.5;
};

#endif //NODE_HPP