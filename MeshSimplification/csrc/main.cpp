#include "mesh.h"

int main(int argc, char **argv) {
    std::string input_path;
    std::string output_path;
    double ratio;

    if (argc < 4) {
        std::cout << "Usage: simplify <input> <output> <ratio>" << std::endl;
        exit(0);
    }

    input_path = argv[1];
    output_path = argv[2];
    ratio = atof(argv[3]);

    time_t st = clock();

    Mesh mesh;

    mesh.load(input_path);
    time_t ed1 = clock();

    mesh.calculateQ();
    time_t ed2 = clock();

    mesh.selectValidPairs(0.01); // threshold
    time_t ed3 = clock();

    mesh.simplify(ratio);
    time_t ed4 = clock();

    double error = mesh.evaluate();

    // mesh.store(output_path);

    std::cout << "Evaluated Error: " << error << std::endl;

    std::cout << "Total Running Time: " << double(ed4 - st) / CLOCKS_PER_SEC
              << " (s)" << std::endl;
    std::cout << "Load Mesh Time: " << double(ed1 - st) / CLOCKS_PER_SEC
              << " (s)" << std::endl;
    std::cout << "Calculate Q Time: " << double(ed2 - ed1) / CLOCKS_PER_SEC
              << " (s)" << std::endl;
    std::cout << "Select Valid Pairs Time: "
              << double(ed3 - ed2) / CLOCKS_PER_SEC << " (s)" << std::endl;
    std::cout << "Simplify Time: " << double(ed4 - ed3) / CLOCKS_PER_SEC
              << " (s)" << std::endl;
    return 0;
}
