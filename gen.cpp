#include <iostream>
#include <fstream>
#include <random>
#include <filesystem>
#include <unordered_set>

#ifndef SEED
constexpr size_t SEED = 998244353;
#endif

std::mt19937 rng(SEED);
std::uniform_int_distribution<int> uniform(0,1e9);

int main() {
    int testQuantities, testSize, searchSize;
    std::cout << "Quantos testes deverão ser gerados?" << std::endl;
    std::cin >> testQuantities;
    std::cout << "Quantos valores cada teste terá?" << std::endl;
    std::cin >> testSize;
    std::cout << "Quantos valores serão buscados em cada teste?" << std::endl;
    std::cin >> searchSize;

    namespace fs = std::filesystem;
    std::string testPath = "tests/";
    if(fs::is_directory(testPath)) 
        fs::remove_all(testPath);
    if(!fs::create_directories("tests/")) {
        std::cerr << "Não foi possível criar o diretório de testes. Abortando..." << std::endl;
        return -1; 
    }

    std::unordered_set<int> used;
    std::vector<int> vec;
    used.reserve(testSize);
    vec.reserve(testSize);

    std::string fileName = "000.in";

    for(int test = 0; test < testQuantities; ++test) {
        int carry = 1;
        for(int i = 2; i >= 0; --i) {
            int soma = fileName[i] - '0' + carry;
            carry = soma / 10;
            fileName[i] = soma % 10 + '0';
        }
        std::ofstream out("tests/" + fileName);
        std::cout << "Gerando testes para o arquivo tests/" << fileName << "..." << std::endl;
        out << testSize << std::endl;
        while (vec.size() < testSize) {
            int x = uniform(rng);
            if(!used.count(x)) {
                used.emplace(x);
                vec.emplace_back(x);
                out << x << ' ';
            }
        }

        out << std::endl;
        out << searchSize << std::endl;
        for(int i = 0; i < searchSize; ++i) 
            out << vec[uniform(rng) % vec.size()] << ' ';
        out << std::endl;
        out << searchSize << std::endl;
        for(int i = 0; i < searchSize; ++i) {
            int x = uniform(rng);
            if(!used.count(x)) {
                out << x << ' ';
            }else
                --i;
        }
        out << std::endl;

        used.clear();
        vec.clear();
    }
}