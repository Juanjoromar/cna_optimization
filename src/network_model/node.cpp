#include "network_model/node.hpp"
#include "network_model/entanglement_interface.hpp"

#include <iostream>

void Node::initialize_node(nlohmann::json node_config) {
    std::size_t n_interfaces = static_cast<std::size_t>(node_config["entanglement_interfaces"]);
        _entanglement_interfaces.reserve(n_interfaces);
        for (std::size_t i = 0; i < n_interfaces; ++i)
            _entanglement_interfaces.push_back(
                std::make_shared<EntanglementInterface>(EntanglementInterface(this, i))
            );
        _instruction_table.set_instruction_table(this, node_config["time_slots"]);
        _swap_probability = node_config["swap_probability"].get<double>();
}

void Node::reset_instruction_table(const std::vector<TimeSlot>& time_slots) {
    _instruction_table.reset(time_slots);
}

void Node::compute_next_time_slot(std::uint32_t time_slot_duration_nanoseconds) {
    _instruction_table.execute_swaps_next_time_slot();
    _instruction_table.execute_stores_next_time_slot(time_slot_duration_nanoseconds);
    for (std::shared_ptr<EntanglementInterface>& interface : _entanglement_interfaces)
        interface->reset_entangled_pair();
}

void Node::mutate(const std::function<double(void)>& rnd01) {
    _instruction_table.mutate(rnd01);
}

std::vector<std::shared_ptr<EntanglementInterface>> Node::get_interfaces() const {
    std::vector<std::shared_ptr<EntanglementInterface>> interface_references;

    interface_references.reserve(_entanglement_interfaces.size());
    for (std::shared_ptr<EntanglementInterface> interface : _entanglement_interfaces)
        interface_references.push_back(std::move(interface));

    return interface_references;
}

void Node::print() const {
    std::cout << "\nNode " << _node_name << " has " << _entanglement_interfaces.size() << " interfaces.\n";
    for (const std::shared_ptr<EntanglementInterface>& iface : _entanglement_interfaces) {
        std::cout << "Interface " << iface->n_interface() 
        << " connected with Node " << iface->fibre_pair().lock()->context()->_node_name << " in interface " << iface->fibre_pair().lock()->n_interface()
        << " and entangled with Node " << iface->entangled_pair().lock()->context()->_node_name << " in interface " << iface->entangled_pair().lock()->n_interface() << ".\n";
    }

    std::cout << "And has instruction table: \n";
    _instruction_table.print();
}