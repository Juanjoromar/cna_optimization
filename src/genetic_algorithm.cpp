#include "genetic_algorithm/definitions.hpp"
#include "network_model/network.hpp"

std::string network_topology_config_file = "../network_config/star_topology.json";

int main(void) {
    
    std::string config_file_name;

    output_file.open("./result.txt");
	output_file
		<<"step"<<"\t"
		<<"cost_avg"<<"\t"
		<<"cost_best"<<"\t"
		<<"x_best0"<<"\t"
		<<"x_best1"<<"\t"
		<<"x_best2"<<"\t"
		<<"x_best3"<<"\t"
		<<"x_best4"
		<<"\n";

	EA::Chronometer timer;
	timer.tic();

	GA_Type ga_obj;
	ga_obj.problem_mode = EA::GA_MODE::SOGA;
	ga_obj.multi_threading = true;
	ga_obj.idle_delay_us = 1; // switch between threads quickly
	ga_obj.verbose = false;
	ga_obj.population = 200;
	ga_obj.generation_max = 1000;
	ga_obj.calculate_SO_total_fitness = calculate_SO_total_fitness;
	ga_obj.init_genes = init_genes;
	ga_obj.eval_solution = eval_solution;
	ga_obj.mutate = mutate;
	ga_obj.crossover = crossover;
	ga_obj.SO_report_generation = SO_report_generation;
	ga_obj.best_stall_max = 10;
	ga_obj.elite_count = 10;
	ga_obj.crossover_fraction = 0.7;
	ga_obj.mutation_rate = 0.8;
	ga_obj.solve();

    output_file.close();

	std::cout << "The problem is optimized in "<<timer.toc()<<" seconds."<<std::endl;

    std::cout << "\n----------------------------\nAmount of entanglement of the best individual: ";
    double mean_amount_of_entanglement = 0;
    std::random_device rseed;
    std::mt19937 rgen(rseed()); // mersenne_twister
    std::uniform_int_distribution<int> idist(0,500);
    for (std::size_t variation = 0; variation < 15; ++variation) {
        ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.advance_n_time_slots(1000 +  idist(rgen));
        mean_amount_of_entanglement += ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.amount_entanglement();
        ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.reset();
    }
    mean_amount_of_entanglement /= 15;
    //ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.advance_n_time_slots(1000);
    //ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.print();

    std::cout << mean_amount_of_entanglement << ".\n";

    ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.advance_n_time_slots(1000);
    ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.print();

    std::cout << "\n----------------------------\nCompared with default soluction: ";
    Network default_network(network_topology_config_file.c_str());

    mean_amount_of_entanglement = 0;
    for (std::size_t variation = 0; variation < 15; ++variation) {
        default_network.advance_n_time_slots(10000 +  idist(rgen));
        mean_amount_of_entanglement += default_network.amount_entanglement();
        default_network.reset();
    }
    mean_amount_of_entanglement /= 15;
    std::cout << mean_amount_of_entanglement << ".\n";

    default_network.advance_n_time_slots(1000);
    default_network.print();

    output_file.open("best_individual_cgf.json");

    output_file << ga_obj.last_generation.chromosomes[ga_obj.last_generation.best_chromosome_index].genes.to_json().dump();

    return 0;
}