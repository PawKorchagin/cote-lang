//
// Created by motya on 06.06.2025.
//
#include "var_manager.h"


void parser::VarManager::new_scope() {
    levels.emplace_back(levels.back().sp);
}

int parser::VarManager::push_var(const std::string &name) {
    var_names[name] = std::make_pair(levels.back().sp, levels.size());//to access it
    levels.back().vars.push_back(name);//to remove it from names later
    return push_var();
}

void parser::VarManager::close_scope() {
    levels.pop_back();
}

//returns -1 if not found
int parser::VarManager::get_var(std::string name) {
    auto it = var_names.find(name);
    if (it == var_names.end())
        return -1;
    return it->second.first;
}

bool parser::VarManager::add_func(std::string name, int fid) {
    if (natives.contains(name)) return false;
    return functions.emplace(name, fid).second;
}

int parser::VarManager::get_func(std::string name) {
    if (natives.contains(name)) return -2;
    auto it = functions.find(name);
    return it == functions.end() ? -1 : it->second;
}

int parser::VarManager::get_native(std::string name) {
    auto it = natives.find(name);
    return it == natives.end() ? -1 : it->second;
}

int parser::VarManager::add_native(std::string name) {
    natives[name] = natives.size();
    return natives.size() - 1;
}

parser::LoopManager::LoopManager(int labelStart, int labelEnd) : label_start(labelStart), label_end(labelEnd) {}
