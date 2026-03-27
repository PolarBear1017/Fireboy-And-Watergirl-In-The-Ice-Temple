#ifndef LEVEL_PARSER_HPP
#define LEVEL_PARSER_HPP

#include "LevelDefinition.hpp"
#include <string>
#include <vector>

LevelDefinition BuildLevelDefinitionFromChars(
    const std::vector<std::string>& mapData);

#endif
