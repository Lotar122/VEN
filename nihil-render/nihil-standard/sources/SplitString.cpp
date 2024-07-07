#include "../headers/SpliString.hpp"

std::vector<std::string> nihil::nstd::splitString(const std::string& input, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream tokenStream(input);
    std::string token;

    while (std::getline(tokenStream, token, delimiter)) {
         tokens.push_back(token);
    }

    return tokens;
}