#ifndef LEVEL_LOADER_HPP
#define LEVEL_LOADER_HPP

#include "LevelDefinition.hpp"
#include <string>

// 從 JSON 檔案讀取關卡，並轉成程式內部使用的 LevelDefinition。
LevelDefinition LoadLevelDefinitionFromJsonFile(const std::string& path);

#endif
