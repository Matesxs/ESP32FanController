//
// Created by Martin on 6. 7. 2025.
//

#ifndef HELPERS_H
#define HELPERS_H

#include <cstdint>

void yieldIfNecessary(uint64_t& lastYield, uint64_t timeout = 2000);
void yieldIfNecessary();

#endif //HELPERS_H
