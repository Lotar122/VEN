#pragma once

#include <iostream>
#include <fstream>

namespace nihil::nstd {
    uint64_t generateChecksum(const std::string path);
    uint64_t loadChecksum(const std::string path);
    bool checkForChanges(const std::string path);
    void saveChecksum(const std::string path, uint64_t checksum);
}