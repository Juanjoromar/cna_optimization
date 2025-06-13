#ifndef GENETIC_OPERATIONS_HPP
#define GENETIC_OPERATIONS_HPP

#include <fstream>

#include "openGA.hpp"
#include "network_model/network.hpp"

typedef EA::Genetic<Network, double> GA_Type;
typedef EA::GenerationType<Network, double> Generation_Type;

void init_genes(Network& network, const std::function<double(void)> &rnd01) {
    network.init_network_random_instruction_tables(rnd01);
    //network.advance_n_time_slots(1000);
}

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType& X) {
    return X.middle_costs;
}

bool eval_solution(const Network& network, double& amount_of_entanglement) {
    double mean_amount_of_entanglement = 0;
    std::random_device rseed;
    std::mt19937 rgen(rseed()); // mersenne_twister
    std::uniform_int_distribution<int> idist(0,500);
    for (std::size_t variation = 0; variation < 20; ++variation) {
        network.advance_n_time_slots(1000 +  idist(rgen));
        mean_amount_of_entanglement -= network.amount_entanglement();
        network.reset();
    }
	amount_of_entanglement = mean_amount_of_entanglement / 20;
    return true; // genes are accepted
}

Network mutate(const Network& base_network, const std::function<double(void)>& rnd01, double shrink_scale) {
    Network new_network(base_network.to_json());
    new_network.mutate(rnd01);
    //new_network.advance_n_time_slots(1000);
    
    return new_network;
}

Network crossover(const Network& network1, const Network& network2, const std::function<double(void)>& rnd01) {
    using json = nlohmann::json;

    json network_config1 = network1.to_json();
    json network_config2 = network2.to_json();

    for (auto& element : network_config1["nodes"].items())
        if (rnd01() > 0.5)
            network_config1["nodes"][element.key()]["time_slots"] = network_config2["nodes"][element.key()]["time_slots"];
    Network new_network(network_config1);
    //new_network.advance_n_time_slots(1000);

    return new_network;
}

std::ofstream output_file;

void SO_report_generation(int generation_number, const Generation_Type& last_generation, const Network& best_genes) {
	std::cout
		<<"Generation ["<<generation_number<<"], "
		<<"Best="<<last_generation.best_total_cost<<", "
		<<"Average="<<last_generation.average_cost<<", "
        <<"Exe_time="<<last_generation.exe_time << "\n";

    //best_genes.print();

	output_file
		<<generation_number<<"\t";
		output_file << best_genes.to_json().dump();

}

#endif //GENETIC_OPERATIONS_HPP