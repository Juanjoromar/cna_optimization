#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <cstdint>
#include <memory>
#include <string>

class EntanglementInterface;

struct SwapPair {
    std::weak_ptr<EntanglementInterface> e1;
    std::weak_ptr<EntanglementInterface> e2;

    SwapPair(std::shared_ptr<EntanglementInterface> e1, std::shared_ptr<EntanglementInterface> e2):
        e1(e1), e2(e2) {}
    
    bool operator==(const SwapPair& other) const {
        return (e1.lock() == other.e1.lock()) && (e2.lock() == other.e2.lock());
    }

    std::string to_string() const;

};

class Swap{
public:
    Swap(const SwapPair& swap_pair): _swap_pair(swap_pair) {};

    void execute();

    SwapPair swap_pair() const { return _swap_pair; }

    bool operator==(const Swap& other) const { 
        return _swap_pair == other._swap_pair;
    }

private:
    SwapPair _swap_pair;
};

class Store{
public:
    Store(std::weak_ptr<EntanglementInterface> e): _e(e) {};

    void execute(std::uint32_t time_duration_nanoseconds);

    bool operator==(const Store& other) const{
        return _e.lock() == other._e.lock();
    }

    std::weak_ptr<EntanglementInterface> interface() const { return _e; }
private:
    std::weak_ptr<EntanglementInterface> _e;
};

#endif //INSTRUCTION_HPP