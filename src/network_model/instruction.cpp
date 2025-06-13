#include "network_model/instruction.hpp"
#include "network_model/entanglement_interface.hpp"
#include "network_model/node.hpp"
#include "network_model/network.hpp"

std::string SwapPair::to_string() const {
    return std::to_string(e1.lock()->n_interface())+"-"+std::to_string(e2.lock()->n_interface());
}

void Swap::execute() {
    std::shared_ptr<EntanglementInterface>
        tmp_e1 = _swap_pair.e1.lock(),
        tmp_e2 = _swap_pair.e2.lock(),
        tmp_e1_pair = tmp_e1->entangled_pair().lock(),
        tmp_e2_pair = tmp_e2->entangled_pair().lock();
        
    //Perform reference_swap
    entangle(tmp_e1, tmp_e2);
    entangle(tmp_e1_pair, tmp_e2_pair);

    tmp_e1->_still_entangled_probability = 0;
    tmp_e2->_still_entangled_probability = 0;

    tmp_e1_pair->_still_entangled_probability *= tmp_e1_pair->context()->swap_probability();
    tmp_e2_pair->_still_entangled_probability *= tmp_e2_pair->context()->swap_probability();
}


void Store::execute(std::uint32_t time_duration_nanoseconds) {
    std::shared_ptr<EntanglementInterface> e = _e.lock();
    double entanglement = e->still_entangled_probability() * e->context()->expected_swaps_per_nanosecond() * time_duration_nanoseconds;
    e->context()->network()->add_shared_entanglement(NodePair{e->context(), e->entangled_pair().lock()->context()}, entanglement/(2e9));
}