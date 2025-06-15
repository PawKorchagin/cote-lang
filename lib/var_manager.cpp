//
// Created by motya on 06.06.2025.
//
#include "var_manager.h"


void parser::VarManager::new_scope() {
    sp.push_back(sp.back());
    locals.emplace_back();
}

int parser::VarManager::push_var(const std::string &name) {
    var_names[name] = sp.back();//to access it
    locals.back().push_back(name);//to remove it from names later
    return push_var();
}

void parser::VarManager::close_scope() {
    sp.pop_back();
    locals.pop_back();
}

int parser::VarManager::get_var(std::string name) {
    auto it = var_names.find(name);
    if (it == var_names.end())
        return -1;
    return it->second;
}

parser::LoopManager::LoopManager(int labelStart, int labelEnd) : label_start(labelStart), label_end(labelEnd) {}
