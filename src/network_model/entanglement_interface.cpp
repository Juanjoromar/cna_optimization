#include "network_model/entanglement_interface.hpp"

void EntanglementInterface::reset_entangled_pair() {
    _entangled_pair = _fibre_pair;
    _still_entangled_probability = 1;
}

void pair(std::shared_ptr<EntanglementInterface> e1, std::shared_ptr<EntanglementInterface> e2) {
    e1->_fibre_pair = e2;
    e2->_fibre_pair = e1;
}
void entangle(std::shared_ptr<EntanglementInterface> e1, std::shared_ptr<EntanglementInterface> e2) {
    e1->_entangled_pair = e2;
    e2->_entangled_pair = e1;
}