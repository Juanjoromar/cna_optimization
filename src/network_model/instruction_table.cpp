#include "network_model/instruction_table.hpp"

#include "network_model/entanglement_interface.hpp"
#include "network_model/instruction.hpp"
#include "network_model/node.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <string>

#include <iostream>

TimeSlot::TimeSlot(Node* context, const std::vector<SwapPair>& swap_instructions):
    _context(context) {
    
    _swap_instructions.reserve(swap_instructions.size());

    std::vector<std::shared_ptr<EntanglementInterface>> remaining_node_interfaces = context->get_interfaces(); 

    for (SwapPair pair : swap_instructions) {
        std::shared_ptr<EntanglementInterface> tmp_e1 = pair.e1.lock(), tmp_e2 = pair.e2.lock();
        if (tmp_e1->context() != context || tmp_e1->context() != context)
            throw "Incorrect context in swap_instructions";
        
        _swap_instructions.push_back(Swap(pair));
        remaining_node_interfaces.erase(
            std::find(
                remaining_node_interfaces.begin(), remaining_node_interfaces.end(), tmp_e1
            )
        );
        remaining_node_interfaces.erase(
            std::find(
                remaining_node_interfaces.begin(), remaining_node_interfaces.end(), tmp_e2
            )
        );
    }

    _store_instructions.reserve(context->n_interfaces() - 2*swap_instructions.size());
    for (std::shared_ptr<EntanglementInterface> interface_ref : remaining_node_interfaces) {
        if (interface_ref->context() != context)
            throw "Incorrect context in interface";

        _store_instructions.push_back(Store(interface_ref));
    }
}

void TimeSlot::execute_swaps() {
    for (Swap& instruction : _swap_instructions)
        instruction.execute();
}

void TimeSlot::execute_stores(std::uint32_t time_slot_duration_nanoseconds) {
    for (Store& instruction : _store_instructions)
        instruction.execute(time_slot_duration_nanoseconds);
}

void TimeSlot::mutate(const std::function<double(void)>& rnd01) {
    if (_context->n_interfaces() < 2) return;

    double action = rnd01();
    if (action < 0.333) {
        //Change one swap for two stores
        if (_swap_instructions.size() < 1) return;

        Swap& swap_to_change = _swap_instructions[static_cast<std::size_t>(rnd01()*_swap_instructions.size())];
        _swap_instructions.erase(
            std::find(
                _swap_instructions.begin(), _swap_instructions.end(), swap_to_change
            )
        );
        _store_instructions.push_back(Store(swap_to_change.swap_pair().e1));
        _store_instructions.push_back(Store(swap_to_change.swap_pair().e2));
    } else if (action < 0.666) {
        //Change two stores for one swap
        if (_store_instructions.size() < 2) return;
        Store store1 = _store_instructions[static_cast<std::size_t>(rnd01()*_store_instructions.size())];
        _store_instructions.erase(
            std::find(
                _store_instructions.begin(), _store_instructions.end(), store1
            )
        );
        Store store2 = _store_instructions[static_cast<std::size_t>(rnd01()*_store_instructions.size())];
        _store_instructions.erase(
            std::find(
                _store_instructions.begin(), _store_instructions.end(), store2
            )
        );
        _swap_instructions.push_back(Swap(SwapPair(store1.interface().lock(), store2.interface().lock())));
    } else {
        //Mix two swaps
        if (_swap_instructions.size() < 2) return;
        Swap swap1 = _swap_instructions[static_cast<std::size_t>(rnd01()*_swap_instructions.size())];
        _swap_instructions.erase(
            std::find(
                _swap_instructions.begin(), _swap_instructions.end(), swap1
            )
        );
        Swap swap2 = _swap_instructions[static_cast<std::size_t>(rnd01()*_swap_instructions.size())];
        _swap_instructions.erase(
            std::find(
                _swap_instructions.begin(), _swap_instructions.end(), swap2
            )
        );
        if (rnd01() > 0.5) {
            _swap_instructions.push_back(Swap(SwapPair(swap1.swap_pair().e1.lock(), swap2.swap_pair().e1.lock())));
            _swap_instructions.push_back(Swap(SwapPair(swap1.swap_pair().e2.lock(), swap2.swap_pair().e2.lock())));
        } else {
            _swap_instructions.push_back(Swap(SwapPair(swap1.swap_pair().e1.lock(), swap2.swap_pair().e2.lock())));
            _swap_instructions.push_back(Swap(SwapPair(swap1.swap_pair().e2.lock(), swap2.swap_pair().e1.lock())));
        }
    }
}

void TimeSlot::print() const {
    for (const Swap& swap : _swap_instructions) {
        std::cout << swap.swap_pair().to_string() << ", ";
    }
}

void InstructionTable::set_instruction_table(Node* context, nlohmann::json time_slots) {
    _context = context;
    _time_slots.reserve(time_slots.size());

    for (auto& time_slot : time_slots.items()) {
        std::vector<SwapPair> swap_instructions;
        swap_instructions.reserve(time_slot.value().size());
        for (auto& swap_instruction : time_slot.value().items()) {
            std::string interface1, interface2;

            std::istringstream interfaces(swap_instruction.value().get<std::string>());

            std::getline(interfaces, interface1, '-');
            std::getline(interfaces, interface2);

            swap_instructions.push_back(SwapPair{(*context)[std::stoul(interface1)], (*context)[std::stoul(interface2)]});
        }
        _time_slots.push_back(TimeSlot(_context, swap_instructions));
    }
}

void InstructionTable::execute_swaps_next_time_slot() {
    if (_swaps_executed) return;
    _time_slots[_next_time_slot].execute_swaps();
    _swaps_executed = true;
    
}

void InstructionTable::execute_stores_next_time_slot(std::uint32_t time_slot_duration_nanoseconds) {
    if (!_swaps_executed)
        throw "Swaps not executed! Do not compute Stores.\n";
    _time_slots[_next_time_slot].execute_stores(time_slot_duration_nanoseconds);
    ++_next_time_slot;
    if (_next_time_slot >= _time_slots.size())
        _next_time_slot = 0;
    _swaps_executed = false;
}

void InstructionTable::reset(const std::vector<TimeSlot>& new_time_slots) {
    _next_time_slot = 0;
    _time_slots.clear();
    _time_slots.reserve(new_time_slots.size());
    for (const TimeSlot& time_slot : new_time_slots)
        _time_slots.push_back(time_slot);
}

void InstructionTable::mutate(const std::function<double(void)>& rnd01) {
    double action = rnd01();
    if (action < 0.4) {
        //Mutate time_slots
        for (TimeSlot& time_slot : _time_slots)
            if (rnd01() < 0.2)
                time_slot.mutate(rnd01);
    } else if (action < 0.7) {
        //Add a new random time slot
        std::vector<SwapPair> swap_instructions;
        std::vector<std::shared_ptr<EntanglementInterface>> interfaces = _context->get_interfaces();

        while (interfaces.size() > 1) {
            std::shared_ptr<EntanglementInterface> interface = interfaces[static_cast<std::size_t>(rnd01() * interfaces.size())];
            interfaces.erase(std::find(interfaces.begin(), interfaces.end(), interface));
            if (rnd01() > 0.5) { //We create a Swap
                std::shared_ptr<EntanglementInterface> interface2 = interfaces[static_cast<std::size_t>(rnd01() * interfaces.size())];
                interfaces.erase(std::find(interfaces.begin(), interfaces.end(), interface2));
                swap_instructions.push_back(SwapPair(interface, interface2));
            }
        }

        _time_slots.push_back(TimeSlot(_context, swap_instructions));
    } else {
        //We erase a time slot at random
        if (_time_slots.size() == 1) {
            _time_slots.clear();
            _time_slots.push_back(TimeSlot(_context, std::vector<SwapPair>()));
        } else 
            _time_slots.erase(std::find(_time_slots.begin(), _time_slots.end(), _time_slots[static_cast<std::size_t>(rnd01() * _time_slots.size())]));
    }
       
}

void InstructionTable::print() const {
    for(const TimeSlot& time_slot : _time_slots) {
        std::cout << "[";
        time_slot.print();
        std::cout << "]\n";
    }
}
