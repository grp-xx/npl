#ifndef _2022_07_18_HPP 
#define _2022_07_18_HPP

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

struct message {
    std::string type; // REQ/REP
    std::string text;
};

void inline to_json(json& j, const message& m)
{
    j = json {{"type", m.type}, {"text", m.text} };
}

void inline from_json(const json& j, message& m)
{
    j.at("type").get_to(m.type);
    j.at("text").get_to(m.text);
}


inline std::string get_fortune() 
{
    const std::string filename("1.txt");
    std::string command = "fortune > " + filename;
    system(command.c_str());

    std::ifstream outfile(filename.c_str());
    auto res  =  std::string((std::istreambuf_iterator<char>(outfile)), std::istreambuf_iterator<char>());
    remove(filename.c_str());
    return res;
}

#endif