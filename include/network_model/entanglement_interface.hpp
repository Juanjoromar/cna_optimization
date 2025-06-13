#ifndef ENTANGLEMENT_INTERFACE_HPP
#define ENTANGLEMENT_INTERFACE_HPP

#include <memory>

class Node;

class EntanglementInterface {
public:
    friend class Swap;
    friend class Store;

    EntanglementInterface(Node* context, std::size_t n_interface):
        _context(context),
        _n_interface(n_interface),
        _fibre_pair(),
        _entangled_pair(),
        _still_entangled_probability(0)
        {};

    ~EntanglementInterface() = default;

    Node* context() const { return _context; }
    std::size_t n_interface() const { return _n_interface; }
    std::weak_ptr<EntanglementInterface> entangled_pair() { return _entangled_pair; }
    std::weak_ptr<EntanglementInterface> fibre_pair() { return _fibre_pair; }
    double still_entangled_probability() const { return _still_entangled_probability; }
    
    void reset_entangled_pair();
    friend void pair(std::shared_ptr<EntanglementInterface> e1, std::shared_ptr<EntanglementInterface> e2);
    friend void entangle(std::shared_ptr<EntanglementInterface> e1, std::shared_ptr<EntanglementInterface> e2);

private:
    Node* _context;
    std::size_t _n_interface;
    std::weak_ptr<EntanglementInterface> _fibre_pair;
    std::weak_ptr<EntanglementInterface> _entangled_pair;
    double _still_entangled_probability;
};

#endif //ENTANGLEMENT_INTERFACE_HPP