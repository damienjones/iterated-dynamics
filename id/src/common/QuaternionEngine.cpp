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
	g_old_z = g_externs.DPixel();
	g_float_parameter->real(g_parameters[P3_REAL]);
	g_float_parameter->imag(g_parameters[P3_IMAG]);
	g_c_quaternion = QuaternionD(g_parameters[P1_REAL], g_parameters[P1_IMAG], g_parameters[P2_REAL], g_parameters[P2_IMAG]);
	return 0;
}

int quaternion_per_pixel_fp()
{
	g_old_z.real(0);
	g_old_z.imag(0);
	g_float_parameter->real(0);
	g_float_parameter->imag(0);
	g_c_quaternion = QuaternionD(g_externs.DxPixel(), g_externs.DyPixel(), g_parameters[P2_REAL], g_parameters[P2_IMAG]);
	return 0;
}

int quaternion_orbit_fp()
{
	QuaternionD a(g_old_z.real(), g_old_z.imag(), g_float_parameter->real(), g_float_parameter->imag());
	a *= a;
	a += g_c_quaternion;
	double a0 = g_old_z.real();
	double a1 = g_old_z.imag();
	double a2 = g_float_parameter->real();
	double a3 = g_float_parameter->imag();

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
	g_old_z.real(n0);
	g_new_z.real(n0);
	g_old_z.imag(n1);
	g_new_z.imag(n1);
	g_float_parameter->real(n2);
	g_float_parameter->imag(n3);
	return 0;
}