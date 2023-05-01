#ifndef _CHATROOM_HPP_
#define _CHATROOM_HPP_

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include "json.hpp"

using json = nlohmann::json;

enum class msg_type {C, D};

struct message {
    msg_type type;
    std::string code;
    std::string to;
    std::string from;
    std::string text;
};

void inline to_json(json& j, const message& m)
{
    j = json {{"type", m.type}, {"code", m.code}, {"to", m.to}, {"from", m.from}, {"text", m.text} };
}

void inline from_json(const json& j, message& m)
{
    j.at("type").get_to(m.type);
    j.at("code").get_to(m.code);
    j.at("to").get_to(m.to);
    j.at("from").get_to(m.from);
    j.at("text").get_to(m.text);
}





#endif