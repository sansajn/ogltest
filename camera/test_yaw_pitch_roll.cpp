#include <string>
#include <iostream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

using std::string;
using std::cout;
using std::setw;


inline float to_rad(float deg) {return M_PI/180.0f*deg;}

void dump_mat4(glm::mat4 const & m, string const & name);
void dump_vec4(glm::vec4 const & v, string const & name);


int main(int argc, char * argv[])
{

	float yaw = to_rad(45);
	glm::mat4 R = glm::yawPitchRoll(yaw, 0.0f, 0.0f);
	glm::vec4 x(1, 0, 0, 0);
	glm::vec4 x_ = R*x;
	cout << "yaw:\n";
	dump_mat4(R, "R_yaw");
	cout << "\n";
	dump_vec4(x_, "x_");

	float pitch = to_rad(45);
	R = glm::yawPitchRoll(0.0f, pitch, 0.0f);
	glm::vec4 y(0, 1, 0, 0);
	glm::vec4 y_ = R*y;
	cout << "\npitch:\n";
	dump_mat4(R, "R_pitch");
	cout << "\n";
	dump_vec4(y_, "y_");

	cout << "\nroll:\n";
	float roll = to_rad(45);
	R = glm::yawPitchRoll(.0f, .0f, roll);
	y_ = R*y;
	cout << "\nroll\n";
	dump_mat4(R, "R_roll");
	cout << "\n";
	dump_vec4(y_, "y_");

	// R_yawPitchRoll = R_yaw*R_pitch*R_roll

	glm::mat4 R_yaw = glm::yawPitchRoll(yaw, .0f, .0f);
	glm::mat4 R_pitch = glm::yawPitchRoll(.0f, pitch, .0f);
	glm::mat4 R_roll = glm::yawPitchRoll(.0f, .0f, roll);
	glm::mat4 R_composed = R_yaw*R_pitch*R_roll;
	R = glm::yawPitchRoll(yaw, pitch, roll);
	cout << "\nyaw-pitch-roll compose:\n";
	dump_mat4(R_composed, "R_composed");
	cout << "\n";
	dump_mat4(R, "R");

	// na akej suradnici bude z po transformacii 45,45,45 ?
	R = glm::yawPitchRoll(to_rad(45), to_rad(45), to_rad(45));
	glm::vec4 z(0, 0, 1, 0);
	glm::vec4 z_ = R*z;
	cout << "\n45,45,45 transform\n";
	dump_vec4(z_, "z_");

	cout << "\npostupna transformacia:\n";
	dump_vec4(R_roll*z, "z_roll");
	dump_vec4(R_pitch*R_roll*z, "z_pitch");
	dump_vec4(R_yaw*R_pitch*R_roll*z, "z_yaw");

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

void dump_vec4(glm::vec4 const & v, string const & name)
{
	cout << name << " = ["
		<< v[0] << " " << v[1] << " " << v[2] << " " << v[3] << "]\n";
}
