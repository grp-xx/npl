#include <cstdlib>
#include <iostream>
#include <json.hpp>
#include <string>

using nlohmann::json;

struct Person {
    int id;
    std::string name;
};

void to_json(json& j, const Person& p)
{
    j = {{"id", p.id}, {"name", p.name}};
}

void from_json(const json& j, Person& p)
{
    j.at("id").get_to(p.id);
    j.at("name").get_to(p.name);
}


int main()
{
    json human1;

    human1["id"] = 42;
    human1["name"] = "Paul";

    Person pp1 = human1;

    Person pp2 = {.id = 10, .name = "John"};

    json human2 = pp2;
    std::string s1 = human1.dump();


    json hh = json::parse(s1);

    std::cout << hh.at("name").get<std::string>() << std::endl;

    return EXIT_SUCCESS;
}
