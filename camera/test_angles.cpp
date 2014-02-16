#include <iostream>
#include <cmath>
#include <string>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

/*#define BOOST_TEST_MODULE angle_test
#include <boost/test/included/unit_test.hpp>*/

using std::cout;
using std::string;
using std::setw;

inline float to_deg(float rad) {return (180.0/M_PI)*rad;}
inline float to_rad(float deg) {return (M_PI/180.0)*deg;}

void dump_mat4(glm::mat4 const & m, string const & name);
void dump_vec3(glm::vec3 const & v, string const & name);


glm::mat4 compute_view(float yaw, float pitch, glm::vec3 const & eye)
{
	glm::mat4 R = glm::yawPitchRoll(yaw, pitch, 0.0f);
	glm::vec3 look = glm::vec3(R*glm::vec4(0, 0, 1, 0));
	glm::vec3 up = glm::vec3(R*glm::vec4(0, 1, 0, 0));
	glm::vec3 right = glm::cross(look, up);

	glm::vec3 center = eye + look;
	return glm::lookAt(eye, center, up);
}

glm::vec3 compute_rotation(glm::vec3 const & eye, glm::vec3 const & center)
{
	glm::vec3 diff = eye - center;	
	float yaw = atan2(diff.x, diff.z);
	float pitch = atan2(diff.y, sqrt(diff.x*diff.x + diff.z*diff.z));
	return glm::vec3(yaw, pitch, 0.0f);
}

/*BOOST_AUTO_TEST_CASE(rotation)
{
	glm::vec3 eye = glm::vec3(2, 2, sqrt(8));
	glm::vec3 rot = compute_rotation(eye, glm::vec3(0, 0, 0));
	BOOST_CHECK_CLOSE(rot.x, to_rad(45), to_rad(35.264), 0.001);
}*/

void test_angle_xz()
{
	// (?) o aky uhol musim otocit kameru, aby som sa s pozicie (1,0,1) pozeral na (0,0,0) ?
	glm::vec3 eye(1,0,1);

	glm::vec3 up(0,1,0);
	glm::mat4 V = glm::lookAt(glm::vec3(1,0,1), glm::vec3(0,0,0), up);

	glm::mat4 R = glm::yawPitchRoll(to_rad(45), .0f, .0f);
	glm::vec3 look = glm::vec3(R*glm::vec4(0,0,-1,0));
	glm::vec3 center = eye + look;
	// R*vec4(0,1,0,0) == up
	glm::mat4 V_ = glm::lookAt(eye, center, up);

	dump_mat4(V, "V");

	dump_mat4(R, "R");
	dump_vec3(look, "look");
	dump_vec3(center, "center");
	dump_mat4(V_, "V_");

	glm::vec3 v_ = glm::vec3(V*glm::vec4(1,1,1,0));
	dump_vec3(v_, "v_");
}

void test_angle()
{

}


int main(int argc, char * argv[])
{
/*
	glm::vec3 eye = glm::vec3(2, 2, 2);
	glm::vec3 look = glm::vec3(0, 0, 0);
	
	glm::mat4 V_expect = glm::lookAt(eye, look, glm::vec3(0, 1, 0));
	dump_mat4(V_expect, "V_expect");

	glm::vec3 rot = compute_rotation(eye, look);
	cout << "yaw: " << to_deg(rot.x) << " deg, pitch:" << to_deg(rot.y) 
		<< " deg\n";
	// ocakavam, ze yaw == 45, pitch == 

	glm::mat4 V = compute_view(rot.x, rot.y, eye);
	dump_mat4(V, "V");
*/

	test_angle_xz();

	return 0;
}

void dump_mat4(glm::mat4 const & m, string const & name)
{
	cout << name << " = [\n";
	for (int i = 0; i < 4; ++i)
	{
		cout << "  ";
		for (int j = 0; j < 4; ++j)
			cout << setw(10) << m[i][j] << " ";
		cout << "\n";
	}
	cout << "]\n";
}

void dump_vec3(glm::vec3 const & v, string const & name)
{
	cout << name << " = ["
		<< v[0] << " " << v[1] << " " << v[2] << "]\n";
}

