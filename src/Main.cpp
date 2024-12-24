#include "../lib/JobRunner.h"
#include <fstream>

int main() {
    JobRunner jobRunner;

    try {

        std::ifstream inputFile("../input.txt");
      
        jobRunner.processInput(inputFile);
        jobRunner.validateAndPrepareJobs();
        jobRunner.findChains();
        jobRunner.printChains();

        
    } catch (const std::logic_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}