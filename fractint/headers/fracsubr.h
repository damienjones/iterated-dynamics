#if !defined(FRAC_SUB_R_H)
#define FRAC_SUB_R_H

extern void calculate_fractal_initialize();
extern void adjust_corner();
#ifndef USE_VARARGS
extern int put_resume(int, ...);
extern int get_resume(int, ...);
#else
extern int put_resume();
extern int get_resume();
#endif
extern int alloc_resume(int, int);
extern int start_resume();
extern void end_resume();
extern void sleep_ms(long);
extern void reset_clock();
extern void plot_orbit_i(long, long, int);
extern void plot_orbit(double, double, int);
extern void orbit_scrub();
extern int work_list_add(int, int, int, int, int, int, int, int);
extern void work_list_tidy();
extern void get_julia_attractor(double, double);
extern int solid_guess_block_size();
extern void _fastcall symPIplot(int, int, int);
extern void _fastcall symPIplot2J(int, int, int);
extern void _fastcall symPIplot4J(int, int, int);
extern void _fastcall symplot2(int, int, int);
extern void _fastcall symplot2Y(int, int, int);
extern void _fastcall symplot2J(int, int, int);
extern void _fastcall symplot4(int, int, int);
extern void _fastcall symplot2basin(int, int, int);
extern void _fastcall symplot4basin(int, int, int);
extern void _fastcall noplot(int, int, int);
extern void fractal_float_to_bf();
extern void adjust_corner_bf();

#endif