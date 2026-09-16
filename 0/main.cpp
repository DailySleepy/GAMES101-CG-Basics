#include <cmath>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <iostream>

using namespace std;

int main() 
{
	double PI = acos(-1);

	Eigen::Vector3f p(2, 1, 1);
	Eigen::Matrix3f r,t;
	double angle = 45 * (PI / 180);
	r << 
		cos(angle), -sin(angle), 0,
		sin(angle), cos(angle), 0,
		0, 0, 1;
	t <<
		1, 0, 1,
		0, 1, 2,
		0, 0, 1;
	cout << t * (r * p) << endl;

	return 0;
}