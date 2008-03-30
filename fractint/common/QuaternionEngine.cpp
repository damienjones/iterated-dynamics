#include <string>

#include "port.h"
#include "cmplx.h"
#include "id.h"
#include "externs.h"

#include "Externals.h"
#include "QuaternionEngine.h"

// These are for quaternions 
QuaternionD g_c_quaternion(0.0, 0.0, 0.0, 0.0);

int quaternion_julia_per_pixel_fp()
{
	g_old_z.x = g_externs.DxPixel();
	g_old_z.y = g_externs.DyPixel();
	g_float_parameter->x = g_parameters[4];
	g_float_parameter->y = g_parameters[5];
	g_c_quaternion = QuaternionD(g_parameters[0], g_parameters[1], g_parameters[2], g_parameters[3]);
	return 0;
}

int quaternion_per_pixel_fp()
{
	g_old_z.x = 0;
	g_old_z.y = 0;
	g_float_parameter->x = 0;
	g_float_parameter->y = 0;
	g_c_quaternion = QuaternionD(g_externs.DxPixel(), g_externs.DyPixel(), g_parameters[2], g_parameters[3]);
	return 0;
}

int quaternion_orbit_fp()
{
	QuaternionD a(g_old_z.x, g_old_z.y, g_float_parameter->x, g_float_parameter->y);
	a *= a;
	a += g_c_quaternion;
	double a0 = g_old_z.x;
	double a1 = g_old_z.y;
	double a2 = g_float_parameter->x;
	double a3 = g_float_parameter->y;

	double n0 = a0*a0-a1*a1-a2*a2-a3*a3 + g_c_quaternion.real();
	double n1 = 2*a0*a1 + g_c_quaternion.R_component_2();
	double n2 = 2*a0*a2 + g_c_quaternion.R_component_3();
	double n3 = 2*a0*a3 + g_c_quaternion.R_component_4();
	// Check bailout 
	g_magnitude = a0*a0 + a1*a1 + a2*a2 + a3*a3;
	if (g_magnitude > g_rq_limit)
	{
		return 1;
	}
	g_old_z.x = n0;
	g_new_z.x = n0;
	g_old_z.y = n1;
	g_new_z.y = n1;
	g_float_parameter->x = n2;
	g_float_parameter->y = n3;
	return 0;
}
