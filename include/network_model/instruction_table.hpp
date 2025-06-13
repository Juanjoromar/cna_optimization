#ifndef INSTRUCTION_TABLE_HPP
#define INSTRUCTION_TABLE_HPP

#include "network_model/entanglement_interface.hpp"
#include "network_model/instruction.hpp"

#include <nlohmann/json.hpp>

#include <vector>

class Node;

class TimeSlot {
public:

    TimeSlot(Node* context, const std::vector<SwapPair>& swap_instructions);

    ~TimeSlot() = default;

    void execute_swaps();
    void execute_stores(std::uint32_t time_slot_duration_nanoseconds);

    void mutate(const std::function<double(void)>& rnd01);

    bool operator==(const TimeSlot& other) const {
        return _context == other._context
           && _swap_instructions == other._swap_instructions
           && _store_instructions == other._store_instructions;
    }

    void print() const;

    Node* context() const { return _context; }
    std::vector<Swap> swap_instructions() const { return _swap_instructions; }

private:
    Node* _context;
    std::vector<Swap> _swap_instructions;
    std::vector<Store> _store_instructions;
};

class InstructionTable {
public:

    InstructionTable() = default;
    
    void set_instruction_table(Node* context, nlohmann::json time_slots);

    ~InstructionTable() = default;

    void execute_swaps_next_time_slot();
    void execute_stores_next_time_slot(std::uint32_t time_slot_duration_nanoseconds);

    void mutate(const std::function<double(void)>& rnd01);

    void reset(const std::vector<TimeSlot>& new_time_slots);

    void print() const;

    Node* context() const { return _context; }

    std::vector<TimeSlot> time_slots() { return _time_slots; }
private:
    Node* _context;
    std::vector<TimeSlot> _time_slots;
    std::size_t _next_time_slot = 0;
    bool _swaps_executed = false;
};

#endif //INSTRUCTION_TABLE_HPP