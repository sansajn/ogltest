/* rutiny pouzivane k debugovaniu */
#pragma once
#include <string>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

void print_vec3(glm::vec3 const v, std::string const & name);
void print_quat(glm::quat const & q, std::string const & name);
