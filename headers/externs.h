#ifndef EXTERNS_H
#define EXTERNS_H
#include <vector>
// keep var names in column 30 for sorting via sort /+30 <in >out
extern int                   g_adapter;         // index into g_video_table[]
extern AlternateMath         alternatemath[];   // alternate math function pointers
extern int                   Ambient;           // Ambient= parameter value
extern int                   g_and_color;       // AND mask for iteration to get color index
extern struct MP             Ans;
extern int                   Ap1deg;
extern int                   AplusOne;
extern bool                  askvideo;
extern float                 aspectdrift;
extern int                   attractors;
extern int                   attrperiod[];
extern DComplex              attr[];
extern bool                  autobrowse;
extern char                  autoname[];
extern char                  autoshowdot;
extern int                   AutoStereo_depth;
extern double                AutoStereo_width;
extern BYTE                  back_color[];
extern int                   g_bad_config;
extern int                   bad_code_count;
extern bool                  bad_outside;
extern int                   bad_value;
extern long                  bailout;
extern enum bailouts         bailoutest;
extern int                   basehertz;
extern int                   basin;
extern int                   bf_save_len;
extern int                   bfdigits;
extern int                   biomorph;
extern unsigned int          bits;
extern int                   bitshift;
extern int                   bitshiftless1;
extern BYTE                  block[];
extern int                   blue_bright;
extern int                   blue_crop_left;
extern int                   blue_crop_right;
extern int                   boxcolor;
extern int                   boxcount;
extern int                   boxvalues[];
extern int                   boxx[];
extern int                   boxy[];
extern bool                  BRIEF;
extern char                  browsemask[13];
extern char                  browsename[];
extern bool                  browsing;
extern bool                  brwscheckparms;
extern bool                  brwschecktype;
extern bool                  busy;
extern long                  calctime;
extern int                 (*calctype)();
extern int                   calc_status;
extern char                  calibrate;
extern bool                  checkcurdir;
extern long                  c_imag;
extern double                closenuff;
extern double                closeprox;
extern DComplex              coefficient;
extern int                   col;
extern int                   color;
extern char                  colorfile[];
extern long                  coloriter;
extern bool                  colorpreloaded;
extern int                   colors;
extern int                   colorstate;
extern int                   g_color_bright;    // brightest color in palette
extern int                   g_color_dark;      // darkest color in palette
extern int                   g_color_medium;    // nearest to medbright grey in palette
extern char                  CommandComment[4][MAXCMT];
extern char                  CommandFile[FILE_MAX_PATH];
extern char                  CommandName[ITEMNAMELEN + 1];
extern bool                  comparegif;
extern long                  con;
extern double                cosx;
extern long                  c_real;
extern int                   curcol;
extern int                   curpass;
extern int                   currow;
extern int                   cyclelimit;
extern int                   c_exp;
extern double                d1overd;
extern BYTE                  g_dac_box[256][3];
extern int                   g_dac_count;
extern bool                  g_dac_learn;
extern double                ddelmin;
extern int                   debugflag;
extern int                   decimals;
extern BYTE                  decoderline[];
extern int                   decomp[];
extern int                   degree;
extern long                  delmin;
extern long                  delx2;
extern long                  delx;
extern LDBL                  delxx2;
extern LDBL                  delxx;
extern long                  dely2;
extern long                  dely;
extern LDBL                  delyy2;
extern LDBL                  delyy;
extern float                 depthfp;
extern unsigned long         dif_counter;
extern unsigned long         dif_limit;
extern bool                  disk16bit;
extern bool                  g_disk_flag;       // disk video active flag
extern bool                  disktarga;
extern int                   display3d;
extern long                  distest;
extern int                   distestwidth;
extern float                 distfp;
extern int                   Distribution;
extern bool                  dither_flag;
extern bool                  dontreadcolor;
extern int                   dotmode;
extern bool                  doublecaution;
extern double                dpx;
extern double                dpy;
extern char                  drawmode;
extern BYTE                  dstack[];
extern std::vector<double>   dx0;
extern std::vector<double>   dx1;
extern double              (*dxpixel)();
extern double                d_x_size;
extern std::vector<double>   dy0;
extern std::vector<double>   dy1;
extern double              (*dypixel)();
extern double                d_y_size;
extern bool                  escape_exit;
extern BYTE                  exitmode;
extern int                   evolving;
extern U16                   evolve_handle;
extern int                   g_eye_separation;
extern float                 eyesfp;
extern bool                  fastrestore;
extern long                  FgHalf;
extern double                fgLimit;
extern long                  FgOne;
extern long                  FgTwo;
extern int                   gridsz;
extern double                fiddlefactor;
extern double                fiddle_reduction;
extern float                 fileaspectratio;
extern int                   filecolors;
extern int                   filetype;
extern int                   filexdots;
extern int                   fileydots;
extern char                  file_name_stack[16][13];
extern int                   fillcolor;
extern float                 finalaspectratio;
extern bool                  finattract;
extern int                   finishrow;
extern bool                  first_init;
extern bool                  floatflag;
extern double                floatmax;
extern double                floatmin;
extern DComplex *            floatparm;
extern int                   fm_attack;
extern int                   fm_decay;
extern int                   fm_release;
extern int                   fm_sustain;
extern int                   fm_wavetype;
extern int                   fm_vol;            // volume of OPL-3 soundcard output
extern int                   forcesymmetry;
extern char                  FormFileName[];
extern char                  FormName[];
extern int                   fractype;
extern const char *          fract_dir1;
extern const char *          fract_dir2;
extern long                  fudge;
extern bool                  functionpreloaded;
extern double                f_at_rad;
extern double                f_radius;
extern double                f_xcenter;
extern double                f_ycenter;
extern U16                   gene_handle;
extern int                   get_corners();
extern bool                  gif87a_flag;
extern char                  gifmask[];
extern char                  Glasses1Map[];
extern int                   g_glasses_type;
extern bool                  g_good_mode;       // video mode ok?
extern bool                  g_got_real_dac;    // loaddac worked, really got a dac
extern int                   got_status;
extern bool                  grayflag;
extern char                  GreyFile[];
extern bool                  hasinverse;
extern int                   haze;
extern unsigned int          height;
extern float                 heightfp;
extern int                   helpmode;
extern int                   hi_atten;
extern U16                   history;
extern char                  IFSFileName[];
extern char                  IFSName[];
extern std::vector<float>    ifs_defn;
extern bool                  ifs_type;
extern int                   imgboxcount;
extern U16                   imgboxhandle;
extern char                  image_map;
extern int                   init3d[20];
extern DComplex              init;
extern int                   initbatch;
extern int                   initcyclelimit;
extern int                   g_init_mode;
extern DComplex              initorbit;
extern int                   initsavetime;
extern int                   inside;
extern char                  insufficient_ifs_mem[];
extern int                   integerfractal;
extern double                inversion[];
extern int                   invert;
extern bool                  g_is_true_color;
extern bool                  ismand;
extern int                   ixstart;
extern int                   ixstop;
extern int                   iystart;
extern int                   iystop;
extern const char *          JIIMleftright[];
extern const char *          JIIMmethod[];
extern int                   juli3Dmode;
extern const char *          juli3Doptions[];
extern bool                  julibrot;
extern int                   kbdcount;
extern bool                  keep_scrn_coords;
extern long                  l16triglim;
extern int                   LastInitOp;
extern unsigned              LastOp;
extern int                   lastorbittype;
extern LComplex              lattr[];
extern long                  lclosenuff;
extern LComplex              lcoefficient;
extern bool                  ldcheck;
extern char                  LFileName[];
extern char                  light_name[];
extern BYTE *                line_buff;
extern LComplex              linit;
extern LComplex              linitorbit;
extern long                  linitx;
extern long                  linity;
extern long                  llimit2;
extern long                  llimit;
extern long                  lmagnitud;
extern char                  LName[];
extern LComplex              lnew;
extern bool                  loaded3d;
extern int                   LodPtr;
extern bool                  Log_Auto_Calc;
extern bool                  Log_Calc;
extern int                   Log_Fly_Calc;
extern long                  LogFlag;
extern std::vector<BYTE>     LogTable;
extern LComplex              lold;
extern LComplex *            longparm;
extern int                   lookatmouse;
extern LComplex              lparm2;
extern LComplex              lparm;
extern long                  ltempsqrx;
extern long                  ltempsqry;
extern LComplex              ltmp;
extern std::vector<long>     lx0;
extern std::vector<long>     lx1;
extern long                (*lxpixel)();
extern std::vector<long>     ly0;
extern std::vector<long>     ly1;
extern long                (*lypixel)();
extern int                   lzw[2];
extern long                  l_at_rad;
extern MATRIX                m;
extern double                magnitude;
extern enum Major            major_method;
extern BYTE *                mapdacbox;
extern bool                  mapset;
extern char                  MAP_name[];
extern double                math_tol[2];
extern int                   maxcolor;
extern long                  maxct;
extern char                  maxfn;
extern long                  maxit;
extern int                   maxlinelength;
extern long                  MaxLTSize;
extern unsigned              Max_Args;
extern unsigned              Max_Ops;
extern long                  maxptr;
extern int                   max_colors;
extern int                   max_kbdcount;
extern int                   maxhistory;
extern int                   max_rhombus_depth;
extern int                   minbox;
extern enum Minor            minor_method;
extern int                   minstack;
extern int                   minstackavail;
extern MOREPARAMS            moreparams[];
extern MP                    mpAp1deg;
extern MP                    mpAplusOne;
extern struct MPC            MPCone;
extern std::vector<MPC>      MPCroots;
extern MPC                   mpctmpparm;
extern MP                    mpd1overd;
extern MP                    mpone;
extern int                   MPOverflow;
extern MP                    mproverd;
extern MP                    mpt2;
extern MP                    mpthreshold;
extern MP                    mptmpparm2x;
extern double                mxmaxfp;
extern double                mxminfp;
extern double                mymaxfp;
extern double                myminfp;
extern int                   name_stack_ptr;
extern DComplex              g_new;
extern char                  newodpx;
extern char                  newodpy;
extern double                newopx;
extern double                newopy;
extern int                   neworbittype;
extern int                   nextsavedincr;
extern bool                  no_sub_images;
extern bool                  no_mag_calc;
extern bool                  nobof;
extern int                   numaffine;
extern unsigned              numcolors;
extern const int             numtrigfn;
extern int                   num_fractal_types;
extern int                   num_worklist;
extern bool                  nxtscreenflag;
extern int                   Offset;
extern DComplex              old;
extern long                  oldcoloriter;
extern BYTE                  olddacbox[256][3];
extern U16                   oldhistory_handle;
extern bool                  old_demm_colors;
extern char                  old_stdcalcmode;
extern char                  odpx;
extern char                  odpy;
extern double                opx;
extern double                opy;
extern int                   orbitsave;
extern int                   orbit_color;
extern int                   orbit_delay;
extern long                  orbit_interval;
extern int                   orbit_ptr;
extern char                  orgfrmdir[];
extern bool                  orgfrmsearch;
extern float                 originfp;
extern int                 (*outln)(BYTE *, int);
extern void                (*outln_cleanup)();
extern int                   outside;
extern bool                  overflow;
extern bool                  overlay3d;
extern bool                  fract_overwrite;
extern double                ox3rd;
extern double                oxmax;
extern double                oxmin;
extern double                oy3rd;
extern double                oymax;
extern double                oymin;
extern double                param[];
extern double                paramrangex;
extern double                paramrangey;
extern double                parmzoom;
extern DComplex              parm2;
extern DComplex              parm;
extern int                   passes;
extern int                   g_patch_level;
extern int                   periodicitycheck;
struct fls;
extern std::vector<fls>      pfls;
extern int                   pixelpi;
extern void                (*plot)(int,int,int);
extern double                plotmx1;
extern double                plotmx2;
extern double                plotmy1;
extern double                plotmy2;
extern int                   polyphony;
extern unsigned              posp;
extern bool                  pot16bit;
extern bool                  potflag;
extern double                potparam[];
extern bool                  preview;
extern int                   previewfactor;
extern int                   px;
extern int                   py;
extern U16                   prmboxhandle;
extern int                   prmboxcount;
extern int                   pseudox;
extern int                   pseudoy;
extern void                (*putcolor)(int,int,int);
extern DComplex              pwr;
extern double                qc;
extern double                qci;
extern double                qcj;
extern double                qck;
extern bool                  quick_calc;
extern int                   RANDOMIZE;
extern std::vector<int>      ranges;
extern int                   rangeslen;
extern int                   RAY;
extern char                  ray_name[];
extern char                  readname[];
extern long                  realcoloriter;
extern char                  recordcolors;
extern int                   red_bright;
extern int                   red_crop_left;
extern int                   red_crop_right;
extern int                   g_release;
extern int                   resave_flag;
extern bool                  reset_periodicity;
extern U16                   resume_info;
extern int                   resume_len;
extern bool                  resuming;
extern bool                  rflag;
extern char                  rlebuf[];
extern int                   rhombus_stack[];
extern int                   root;
extern std::vector<DComplex> roots;
extern int                   rotate_hi;
extern int                   rotate_lo;
extern double                roverd;
extern int                   row;
extern int                   g_row_count;       // row-counter for decoder and out_line
extern double                rqlim2;
extern double                rqlim;
extern int                   rseed;
extern long                  savebase;
extern DComplex              SaveC;
extern int                   savedac;
extern char                  savename[];
extern long                  saveticks;
extern int                   save_orbit[];
extern int                   save_release;
extern int                   save_system;
extern int                   scale_map[];
extern float                 screenaspect;
extern char                  scrnfile[];
extern struct SearchPath     searchfor;
extern bool                  set_orbit_corners;
extern bool                  showbox;
extern int                   showdot;
extern int                   showfile;
extern bool                  show_orbit;
extern double                sinx;
extern int                   sizedot;
extern short                 sizeofstring[];
extern short                 skipxdots;
extern short                 skipydots;
extern int                   g_slides;
extern int                   Slope;
extern int                   soundflag;
extern char                  speed_prompt[];
extern void                (*standardplot)(int,int,int);
extern bool                  start_showorbit;
extern bool                  started_resaves;
extern DComplex              staticroots[];
extern char                  stdcalcmode;
extern char                  stereomapname[];
extern int                   StoPtr;
extern int                   stoppass;
extern unsigned int          strlocn[];
extern double                sx3rd;
extern int                   sxdots;
extern double                sxmax;
extern double                sxmin;
extern int                   sxoffs;
extern double                sy3rd;
extern int                   sydots;
extern double                symax;
extern double                symin;
extern int                   symmetry;
extern int                   syoffs;
extern char                  s_makepar[];
extern bool                  tabmode;
extern bool                  taborhelp;
extern bool                  Targa_Out;
extern bool                  Targa_Overlay;
extern char                  temp1[];
extern double                tempsqrx;
extern double                tempsqry;
extern BYTE                  teststring[];
extern int                   g_text_cbase;      // g_text_col is relative to this
extern int                   g_text_col;        // current column in text mode
extern int                   g_text_rbase;      // g_text_row is relative to this
extern int                   g_text_row;        // current row in text mode
extern unsigned int          this_gen_rseed;
extern unsigned *            tga16;
extern long *                tga32;
extern bool                  three_pass;
extern double                threshold;
extern int                   timedsave;
extern bool                  timerflag;
extern long                  timer_interval;
extern long                  timer_start;
extern DComplex              tmp;
extern char                  tempdir[];
extern double                toosmall;
extern int                   totpasses;
extern int                   transparent[];
extern BYTE                  trigndx[];
extern bool                  truecolor;
extern int                   truemode;
extern double                twopi;
extern char                  useinitorbit;
extern bool                  use_grid;
extern bool                  usemag;
extern bool                  uses_ismand;
extern bool                  uses_p1;
extern bool                  uses_p2;
extern bool                  uses_p3;
extern bool                  uses_p4;
extern bool                  uses_p5;
extern bool                  use_old_distest;
extern bool                  use_old_period;
extern bool                  using_jiim;
extern int                   usr_biomorph;
extern long                  usr_distest;
extern bool                  usr_floatflag;
extern int                   usr_periodicitycheck;
extern char                  usr_stdcalcmode;
extern int                   g_vesa_x_res;
extern int                   g_vesa_y_res;
extern VIDEOINFO             g_video_entry;
extern VIDEOINFO             g_video_table[];
extern int                   g_video_table_len;
extern bool                  video_cutboth;
extern bool                  g_video_scroll;
extern int                   g_video_start_x;
extern int                   g_video_start_y;
extern int                   g_video_type;      // video adapter type
extern VECTOR                view;
extern bool                  viewcrop;
extern float                 viewreduction;
extern bool                  viewwindow;
extern int                   viewxdots;
extern int                   viewydots;
extern bool                  g_virtual_screens;
extern unsigned              vsp;
extern int                   g_vxdots;
extern int                   g_which_image;
extern float                 widthfp;
extern char                  workdir[];
extern WORKLIST              worklist[MAXCALCWORK];
extern int                   workpass;
extern int                   worksym;
extern long                  x3rd;
extern int                   xadjust;
extern double                xcjul;
extern int                   xdots;
extern long                  xmax;
extern long                  xmin;
extern int                   xshift1;
extern int                   xshift;
extern int                   xtrans;
extern double                xx3rd;
extern int                   xxadjust1;
extern int                   xxadjust;
extern double                xxmax;
extern double                xxmin;
extern long                  XXOne;
extern int                   xxstart;
extern int                   xxstop;
extern long                  y3rd;
extern int                   yadjust;
extern double                ycjul;
extern int                   ydots;
extern long                  ymax;
extern long                  ymin;
extern int                   yshift1;
extern int                   yshift;
extern int                   ytrans;
extern double                yy3rd;
extern int                   yyadjust1;
extern int                   yyadjust;
extern int                   yyadjust;
extern double                yymax;
extern double                yymin;
extern int                   yystart;
extern int                   yystop;
extern double                zbx;
extern double                zby;
extern double                zdepth;
extern int                   zdots;
extern bool                  zoomoff;
extern int                   zrotate;
extern bool                  zscroll;
extern double                zskew;
extern double                zwidth;
#if defined(XFRACT)
extern bool                  fake_lut;
extern bool                  XZoomWaiting;
#endif
#endif
