/*
        Various routines that prompt for things.
*/
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <string.h>
#ifdef   XFRACT
#include <sys/stat.h>
#include <sys/types.h>
#endif
#if !defined(_WIN32)
#include <malloc.h>
#endif

#include "port.h"
#include "prototyp.h"
#include "fractype.h"
#include "helpdefs.h"
#include "helpcom.h"

#ifdef __hpux
#include <sys/param.h>
#define getwd(a) getcwd(a, MAXPATHLEN)
#endif

#include "drivers.h"

// Routines used in prompts2.c

int prompt_checkkey(int curkey);
int prompt_checkkey_scroll(int curkey);
long get_file_entry(int type, const char *title, char *fmask,
                    char *filename, char *entryname);

// Routines in this module

int prompt_valuestring(char *buf, fullscreenvalues *val);
static  int input_field_list(int attr, char *fld, int vlen, const char **list, int llen,
                             int row, int col, int (*checkkey)(int));
static  fractal_type select_fracttype(fractal_type t);
static  int sel_fractype_help(int curkey, int choice);
static bool select_type_params(fractal_type newfractype, fractal_type oldfractype);
void set_default_parms();
static  long gfe_choose_entry(int, const char *, char *, char *);
static  int check_gfe_key(int curkey, int choice);
static  void load_entry_text(FILE *entfile, char *buf, int maxlines, int startrow, int startcol);
static  void format_parmfile_line(int, char *);
static  bool get_light_params();
static  bool check_mapfile();
static  bool get_funny_glasses_params();

#define GETFORMULA 0
#define GETLSYS    1
#define GETIFS     2
#define GETPARM    3

static char funnyglasses_map_name[16];
char ifsmask[13]     = {"*.ifs"};
char formmask[13]    = {"*.frm"};
char lsysmask[13]    = {"*.l"};
char Glasses1Map[] = "glasses1.map";
char MAP_name[FILE_MAX_DIR] = "";
bool mapset = false;
bool julibrot = false;                  // flag for julibrot

// ---------------------------------------------------------------------

int promptfkeys;

// These need to be global because F6 exits fullscreen_prompt()
int scroll_row_status;    /* will be set to first line of extra info to
                             be displayed ( 0 = top line) */
int scroll_column_status; /* will be set to first column of extra info to
                             be displayed ( 0 = leftmost column )*/

int fullscreen_prompt(      // full-screen prompting routine
    const char *hdg,        // heading, lines separated by \n
    int numprompts,         // there are this many prompts (max)
    const char **prompts,   // array of prompting pointers
    fullscreenvalues *values, // array of values
    int fkeymask,           // bit n on if Fn to cause return
    char *extrainfo         // extra info box to display, \n separated
)
{
    const char *hdgscan;
    int titlelines, titlewidth, titlerow;
    int maxpromptwidth, maxfldwidth, maxcomment;
    int boxrow, boxlines;
    int boxcol, boxwidth;
    int extralines, extrawidth, extrarow;
    int instrrow;
    int promptrow, promptcol, valuecol;
    int curchoice = 0;
    int done;
    int savelookatmouse;
    int curtype, curlen;
    char buf[81];

    // scrolling related variables
    FILE * scroll_file = nullptr;     // file with extrainfo entry to scroll
    long scroll_file_start = 0;    // where entry starts in scroll_file
    bool in_scrolling_mode = false; // will be true if need to scroll extrainfo
    int lines_in_entry = 0;        // total lines in entry to be scrolled
    int vertical_scroll_limit = 0; // don't scroll down if this is top line
    int widest_entry_line = 0;     // length of longest line in entry
    bool rewrite_extrainfo = false; // if true: rewrite extrainfo to text box
    char blanks[78];               // used to clear text box

    savelookatmouse = lookatmouse;
    lookatmouse = 0;
    promptfkeys = fkeymask;
    memset(blanks, ' ', 77);   // initialize string of blanks
    blanks[77] = (char) 0;

    /* If applicable, open file for scrolling extrainfo. The function
       find_file_item() opens the file and sets the file pointer to the
       beginning of the entry.
    */
    if (extrainfo && *extrainfo)
    {
        if (fractype == fractal_type::FORMULA || fractype == fractal_type::FFORMULA)
        {
            find_file_item(FormFileName, FormName, &scroll_file, 1);
            in_scrolling_mode = true;
            scroll_file_start = ftell(scroll_file);
        }
        else if (fractype == fractal_type::LSYSTEM)
        {
            find_file_item(LFileName, LName, &scroll_file, 2);
            in_scrolling_mode = true;
            scroll_file_start = ftell(scroll_file);
        }
        else if (fractype == fractal_type::IFS || fractype == fractal_type::IFS3D)
        {
            find_file_item(IFSFileName, IFSName, &scroll_file, 3);
            in_scrolling_mode = true;
            scroll_file_start = ftell(scroll_file);
        }
    }

    // initialize widest_entry_line and lines_in_entry
    if (in_scrolling_mode && scroll_file != nullptr)
    {
        bool comment = false;
        int c = 0;
        int widthct = 0;
        while ((c = fgetc(scroll_file)) != EOF && c != '\032')
        {
            if (c == ';')
                comment = true;
            else if (c == '\n')
            {
                comment = false;
                lines_in_entry++;
                widthct =  -1;
            }
            else if (c == '\t')
                widthct += 7 - widthct % 8;
            else if (c == '\r')
                continue;
            if (++widthct > widest_entry_line)
                widest_entry_line = widthct;
            if (c == '}' && !comment)
            {
                lines_in_entry++;
                break;
            }
        }
        if (c == EOF || c == '\032')
        { // should never happen
            fclose(scroll_file);
            in_scrolling_mode = false;
        }
    }



    helptitle();                        // clear screen, display title line
    driver_set_attr(1, 0, C_PROMPT_BKGRD, 24*80);  // init rest of screen to background


    hdgscan = hdg;                      // count title lines, find widest
    titlewidth = 0;
    titlelines = 1;
    {
        int i = 0;
        while (*hdgscan)
        {
            if (*(hdgscan++) == '\n')
            {
                ++titlelines;
                i = -1;
            }
            if (++i > titlewidth)
                titlewidth = i;
        }
    }
    extrawidth = 0;
    extralines = extrawidth;
    {
        hdgscan = extrainfo;
        if (hdgscan != nullptr)
        {
            if (*hdgscan == 0)
                extrainfo = nullptr;
            else
            { // count extra lines, find widest
                extralines = 3;
                int i = 0;
                while (*hdgscan)
                {
                    if (*(hdgscan++) == '\n')
                    {
                        if (extralines + numprompts + titlelines >= 20)
                        {
                            assert(!"heading overflow");
                            break;
                        }
                        ++extralines;
                        i = -1;
                    }
                    if (++i > extrawidth)
                        extrawidth = i;
                }
            }
        }
    }

    // if entry fits in available space, shut off scrolling
    if (in_scrolling_mode && scroll_row_status == 0
            && lines_in_entry == extralines - 2
            && scroll_column_status == 0
            && strchr(extrainfo, '\021') == nullptr)
    {
        in_scrolling_mode = false;
        fclose(scroll_file);
        scroll_file = nullptr;
    }

    /*initialize vertical scroll limit. When the top line of the text
      box is the vertical scroll limit, the bottom line is the end of the
      entry, and no further down scrolling is necessary.
    */
    if (in_scrolling_mode)
        vertical_scroll_limit = lines_in_entry - (extralines - 2);

    // work out vertical positioning
    {
        int i = numprompts + titlelines + extralines + 3; // total rows required
        int j = (25 - i) / 2;                   // top row of it all when centered
        j -= j / 4;                         // higher is better if lots extra
        boxlines = numprompts;
        titlerow = 1 + j;
    }
    boxrow = titlerow + titlelines;
    promptrow = boxrow;
    if (titlerow > 2)
    {                 // room for blank between title & box?
        --titlerow;
        --boxrow;
        ++boxlines;
    }
    instrrow = boxrow+boxlines;
    if (instrrow + 3 + extralines < 25)
    {
        ++boxlines;    // blank at bottom of box
        ++instrrow;
        if (instrrow + 3 + extralines < 25)
            ++instrrow; // blank before instructions
    }
    extrarow = instrrow + 2;
    if (numprompts > 1) // 3 instructions lines
        ++extrarow;
    if (extrarow + extralines < 25)
        ++extrarow;

    if (in_scrolling_mode)  // set box to max width if in scrolling mode
        extrawidth = 76;

    // work out horizontal positioning
    maxcomment = 0;
    maxpromptwidth = maxcomment;
    maxfldwidth = maxpromptwidth;
    bool anyinput = false;
    for (int i = 0; i < numprompts; i++)
    {
        if (values[i].type == 'y')
        {
            static const char *noyes[2] = {"no", "yes"};
            values[i].type = 'l';
            values[i].uval.ch.vlen = 3;
            values[i].uval.ch.list = noyes;
            values[i].uval.ch.llen = 2;
        }
        int j = (int) strlen(prompts[i]);
        if (values[i].type == '*')
        {
            if (j > maxcomment)
                maxcomment = j;
        }
        else
        {
            anyinput = true;
            if (j > maxpromptwidth)
                maxpromptwidth = j;
            j = prompt_valuestring(buf, &values[i]);
            if (j > maxfldwidth)
                maxfldwidth = j;
        }
    }
    boxwidth = maxpromptwidth + maxfldwidth + 2;
    if (maxcomment > boxwidth)
        boxwidth = maxcomment;
    if ((boxwidth += 4) > 80)
        boxwidth = 80;
    boxcol = (80 - boxwidth) / 2;       // center the box
    promptcol = boxcol + 2;
    valuecol = boxcol + boxwidth - maxfldwidth - 2;
    if (boxwidth <= 76)
    {               // make margin a bit wider if we can
        boxwidth += 2;
        --boxcol;
    }
    {
        int j = titlewidth;
        if (j < extrawidth)
            j = extrawidth;
        int i = j + 4 - boxwidth;
        if (i > 0)
        {   // expand box for title/extra
            if (boxwidth + i > 80)
                i = 80 - boxwidth;
            boxwidth += i;
            boxcol -= i / 2;
        }
    }
    {
        int i = (90 - boxwidth) / 20;
        boxcol    -= i;
        promptcol -= i;
        valuecol  -= i;
    }

    // display box heading
    for (int i = titlerow; i < boxrow; ++i)
        driver_set_attr(i, boxcol, C_PROMPT_HI, boxwidth);

    {
        char buffer[256], *hdgline = buffer;
        // center each line of heading independently
        int i;
        strcpy(hdgline, hdg);
        for (i = 0; i < titlelines-1; i++)
        {
            char *next = strchr(hdgline, '\n');
            if (next == nullptr)
                break; // shouldn't happen
            *next = '\0';
            titlewidth = (int) strlen(hdgline);
            g_text_cbase = boxcol + (boxwidth - titlewidth) / 2;
            driver_put_string(titlerow+i, 0, C_PROMPT_HI, hdgline);
            *next = '\n';
            hdgline = next+1;
        }
        // add scrolling key message, if applicable
        if (in_scrolling_mode)
        {
            *(hdgline + 31) = (char) 0;   // replace the ')'
            strcat(hdgline, ". CTRL+(direction key) to scroll text.)");
        }

        titlewidth = (int) strlen(hdgline);
        g_text_cbase = boxcol + (boxwidth - titlewidth) / 2;
        driver_put_string(titlerow+i, 0, C_PROMPT_HI, hdgline);
    }

    // display extra info
    if (extrainfo)
    {
#ifndef XFRACT
#define S1 '\xC4'
#define S2 "\xC0"
#define S3 "\xD9"
#define S4 "\xB3"
#define S5 "\xDA"
#define S6 "\xBF"
#else
#define S1 '-'
#define S2 "+" // ll corner
#define S3 "+" // lr corner
#define S4 "|"
#define S5 "+" // ul corner
#define S6 "+" // ur corner
#endif
        memset(buf, S1, 80);
        buf[boxwidth-2] = 0;
        g_text_cbase = boxcol + 1;
        driver_put_string(extrarow, 0, C_PROMPT_BKGRD, buf);
        driver_put_string(extrarow+extralines-1, 0, C_PROMPT_BKGRD, buf);
        --g_text_cbase;
        driver_put_string(extrarow, 0, C_PROMPT_BKGRD, S5);
        driver_put_string(extrarow+extralines-1, 0, C_PROMPT_BKGRD, S2);
        g_text_cbase += boxwidth - 1;
        driver_put_string(extrarow, 0, C_PROMPT_BKGRD, S6);
        driver_put_string(extrarow+extralines-1, 0, C_PROMPT_BKGRD, S3);

        g_text_cbase = boxcol;

        for (int i = 1; i < extralines-1; ++i)
        {
            driver_put_string(extrarow+i, 0, C_PROMPT_BKGRD, S4);
            driver_put_string(extrarow+i, boxwidth-1, C_PROMPT_BKGRD, S4);
        }
        g_text_cbase += (boxwidth - extrawidth) / 2;
        driver_put_string(extrarow+1, 0, C_PROMPT_TEXT, extrainfo);
    }

    g_text_cbase = 0;

    // display empty box
    for (int i = 0; i < boxlines; ++i)
        driver_set_attr(boxrow+i, boxcol, C_PROMPT_LO, boxwidth);

    // display initial values
    for (int i = 0; i < numprompts; i++)
    {
        driver_put_string(promptrow+i, promptcol, C_PROMPT_LO, prompts[i]);
        prompt_valuestring(buf, &values[i]);
        driver_put_string(promptrow+i, valuecol, C_PROMPT_LO, buf);
    }


    if (!anyinput)
    {
        putstringcenter(instrrow++, 0, 80, C_PROMPT_BKGRD,
                        "No changeable parameters;");
        putstringcenter(instrrow, 0, 80, C_PROMPT_BKGRD,
                (helpmode > 0) ?
                "Press ENTER to exit, ESC to back out, " FK_F1 " for help"
                : "Press ENTER to exit");
        driver_hide_text_cursor();
        g_text_cbase = 2;
        while (1)
        {
            if (rewrite_extrainfo)
            {
                rewrite_extrainfo = false;
                fseek(scroll_file, scroll_file_start, SEEK_SET);
                load_entry_text(scroll_file, extrainfo, extralines - 2,
                                scroll_row_status, scroll_column_status);
                for (int i = 1; i <= extralines-2; i++)
                    driver_put_string(extrarow+i, 0, C_PROMPT_TEXT, blanks);
                driver_put_string(extrarow+1, 0, C_PROMPT_TEXT, extrainfo);
            }
            // TODO: rework key interaction to blocking wait
            while (!driver_key_pressed())
            {
            }
            done = driver_get_key();
            switch (done)
            {
            case FIK_ESC:
                done = -1;
            case FIK_ENTER:
            case FIK_ENTER_2:
                goto fullscreen_exit;
            case FIK_CTL_DOWN_ARROW:    // scrolling key - down one row
                if (in_scrolling_mode && scroll_row_status < vertical_scroll_limit)
                {
                    scroll_row_status++;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_CTL_UP_ARROW:      // scrolling key - up one row
                if (in_scrolling_mode && scroll_row_status > 0)
                {
                    scroll_row_status--;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_CTL_LEFT_ARROW:    // scrolling key - left one column
                if (in_scrolling_mode && scroll_column_status > 0)
                {
                    scroll_column_status--;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_CTL_RIGHT_ARROW:   // scrolling key - right one column
                if (in_scrolling_mode && strchr(extrainfo, '\021') != nullptr)
                {
                    scroll_column_status++;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_CTL_PAGE_DOWN:   // scrolling key - down one screen
                if (in_scrolling_mode && scroll_row_status < vertical_scroll_limit)
                {
                    scroll_row_status += extralines - 2;
                    if (scroll_row_status > vertical_scroll_limit)
                        scroll_row_status = vertical_scroll_limit;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_CTL_PAGE_UP:     // scrolling key - up one screen
                if (in_scrolling_mode && scroll_row_status > 0)
                {
                    scroll_row_status -= extralines - 2;
                    if (scroll_row_status < 0)
                        scroll_row_status = 0;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_CTL_END:         // scrolling key - to end of entry
                if (in_scrolling_mode)
                {
                    scroll_row_status = vertical_scroll_limit;
                    scroll_column_status = 0;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_CTL_HOME:        // scrolling key - to beginning of entry
                if (in_scrolling_mode)
                {
                    scroll_column_status = 0;
                    scroll_row_status = scroll_column_status;
                    rewrite_extrainfo = true;
                }
                break;
            case FIK_F2:
            case FIK_F3:
            case FIK_F4:
            case FIK_F5:
            case FIK_F6:
            case FIK_F7:
            case FIK_F8:
            case FIK_F9:
            case FIK_F10:
                if (promptfkeys & (1 << (done+1-FIK_F1)))
                    goto fullscreen_exit;
            }
        }
    }


    // display footing
    if (numprompts > 1)
        putstringcenter(instrrow++, 0, 80, C_PROMPT_BKGRD,
                        "Use " UPARR1 " and " DNARR1 " to select values to change");
    putstringcenter(instrrow+1, 0, 80, C_PROMPT_BKGRD,
            (helpmode > 0) ?
            "Press ENTER when finished, ESCAPE to back out, or " FK_F1 " for help"
            : "Press ENTER when finished (or ESCAPE to back out)");

    done = 0;
    while (values[curchoice].type == '*')
        ++curchoice;

    while (!done)
    {
        if (rewrite_extrainfo)
        {
            int j = g_text_cbase;
            g_text_cbase = 2;
            fseek(scroll_file, scroll_file_start, SEEK_SET);
            load_entry_text(scroll_file, extrainfo, extralines - 2,
                            scroll_row_status, scroll_column_status);
            for (int i = 1; i <= extralines-2; i++)
                driver_put_string(extrarow+i, 0, C_PROMPT_TEXT, blanks);
            driver_put_string(extrarow+1, 0, C_PROMPT_TEXT, extrainfo);
            g_text_cbase = j;
        }

        curtype = values[curchoice].type;
        curlen = prompt_valuestring(buf, &values[curchoice]);
        if (!rewrite_extrainfo)
            putstringcenter(instrrow, 0, 80, C_PROMPT_BKGRD,
                    (curtype == 'l') ?
                    "Use " LTARR1 " or " RTARR1 " to change value of selected field"
                    : "Type in replacement value for selected field");
        else
            rewrite_extrainfo = false;
        driver_put_string(promptrow+curchoice, promptcol, C_PROMPT_HI, prompts[curchoice]);

        int i;
        if (curtype == 'l')
        {
            i = input_field_list(
                    C_PROMPT_CHOOSE, buf, curlen,
                    values[curchoice].uval.ch.list, values[curchoice].uval.ch.llen,
                    promptrow+curchoice, valuecol, in_scrolling_mode ? prompt_checkkey_scroll : prompt_checkkey);
            int j;
            for (j = 0; j < values[curchoice].uval.ch.llen; ++j)
                if (strcmp(buf, values[curchoice].uval.ch.list[j]) == 0)
                    break;
            values[curchoice].uval.ch.val = j;
        }
        else
        {
            int j = 0;
            if (curtype == 'i')
                j = INPUTFIELD_NUMERIC | INPUTFIELD_INTEGER;
            if (curtype == 'L')
                j = INPUTFIELD_NUMERIC | INPUTFIELD_INTEGER;
            if (curtype == 'd')
                j = INPUTFIELD_NUMERIC | INPUTFIELD_DOUBLE;
            if (curtype == 'D')
                j = INPUTFIELD_NUMERIC | INPUTFIELD_DOUBLE | INPUTFIELD_INTEGER;
            if (curtype == 'f')
                j = INPUTFIELD_NUMERIC;
            i = input_field(j, C_PROMPT_INPUT, buf, curlen,
                            promptrow+curchoice, valuecol, in_scrolling_mode ? prompt_checkkey_scroll : prompt_checkkey);
            switch (values[curchoice].type)
            {
            case 'd':
            case 'D':
                values[curchoice].uval.dval = atof(buf);
                break;
            case 'f':
                values[curchoice].uval.dval = atof(buf);
                roundfloatd(&values[curchoice].uval.dval);
                break;
            case 'i':
                values[curchoice].uval.ival = atoi(buf);
                break;
            case 'L':
                values[curchoice].uval.Lval = atol(buf);
                break;
            case 's':
                strncpy(values[curchoice].uval.sval, buf, 16);
                break;
            default: // assume 0x100+n
                strcpy(values[curchoice].uval.sbuf, buf);
            }
        }

        driver_put_string(promptrow+curchoice, promptcol, C_PROMPT_LO, prompts[curchoice]);
        {
            int j = (int) strlen(buf);
            memset(&buf[j], ' ', 80-j);
        }
        buf[curlen] = 0;
        driver_put_string(promptrow+curchoice, valuecol, C_PROMPT_LO,  buf);

        switch (i)
        {
        case 0:  // enter
            done = 13;
            break;
        case -1: // escape
        case FIK_F2:
        case FIK_F3:
        case FIK_F4:
        case FIK_F5:
        case FIK_F6:
        case FIK_F7:
        case FIK_F8:
        case FIK_F9:
        case FIK_F10:
            done = i;
            break;
        case FIK_PAGE_UP:
            curchoice = -1;
        case FIK_DOWN_ARROW:
            do
            {
                if (++curchoice >= numprompts)
                    curchoice = 0;
            }
            while (values[curchoice].type == '*');
            break;
        case FIK_PAGE_DOWN:
            curchoice = numprompts;
        case FIK_UP_ARROW:
            do
            {
                if (--curchoice < 0)
                    curchoice = numprompts - 1;
            }
            while (values[curchoice].type == '*');
            break;
        case FIK_CTL_DOWN_ARROW:     // scrolling key - down one row
            if (in_scrolling_mode && scroll_row_status < vertical_scroll_limit)
            {
                scroll_row_status++;
                rewrite_extrainfo = true;
            }
            break;
        case FIK_CTL_UP_ARROW:       // scrolling key - up one row
            if (in_scrolling_mode && scroll_row_status > 0)
            {
                scroll_row_status--;
                rewrite_extrainfo = true;
            }
            break;
        case FIK_CTL_LEFT_ARROW:     //scrolling key - left one column
            if (in_scrolling_mode && scroll_column_status > 0)
            {
                scroll_column_status--;
                rewrite_extrainfo = true;
            }
            break;
        case FIK_CTL_RIGHT_ARROW:    // scrolling key - right one column
            if (in_scrolling_mode && strchr(extrainfo, '\021') != nullptr)
            {
                scroll_column_status++;
                rewrite_extrainfo = true;
            }
            break;
        case FIK_CTL_PAGE_DOWN:    // scrolling key - down on screen
            if (in_scrolling_mode && scroll_row_status < vertical_scroll_limit)
            {
                scroll_row_status += extralines - 2;
                if (scroll_row_status > vertical_scroll_limit)
                    scroll_row_status = vertical_scroll_limit;
                rewrite_extrainfo = true;
            }
            break;
        case FIK_CTL_PAGE_UP:      // scrolling key - up one screen
            if (in_scrolling_mode && scroll_row_status > 0)
            {
                scroll_row_status -= extralines - 2;
                if (scroll_row_status < 0)
                    scroll_row_status = 0;
                rewrite_extrainfo = true;
            }
            break;
        case FIK_CTL_END:          // scrolling key - go to end of entry
            if (in_scrolling_mode)
            {
                scroll_row_status = vertical_scroll_limit;
                scroll_column_status = 0;
                rewrite_extrainfo = true;
            }
            break;
        case FIK_CTL_HOME:         // scrolling key - go to beginning of entry
            if (in_scrolling_mode)
            {
                scroll_column_status = 0;
                scroll_row_status = scroll_column_status;
                rewrite_extrainfo = true;
            }
            break;
        }
    }

fullscreen_exit:
    driver_hide_text_cursor();
    lookatmouse = savelookatmouse;
    if (scroll_file)
    {
        fclose(scroll_file);
        scroll_file = nullptr;
    }
    return done;
}

int prompt_valuestring(char *buf, fullscreenvalues *val)
{   // format value into buf, return field width
    int i, ret;
    switch (val->type)
    {
    case 'd':
        ret = 20;
        i = 16;    // cellular needs 16 (was 15)
        while (1)
        {
            sprintf(buf, "%.*g", i, val->uval.dval);
            if ((int)strlen(buf) <= ret)
                break;
            --i;
        }
        break;
    case 'D':
        if (val->uval.dval < 0)
        { // We have to round the right way
            sprintf(buf, "%ld", (long)(val->uval.dval-.5));
        }
        else
        {
            sprintf(buf, "%ld", (long)(val->uval.dval+.5));
        }
        ret = 20;
        break;
    case 'f':
        sprintf(buf, "%.7g", val->uval.dval);
        ret = 14;
        break;
    case 'i':
        sprintf(buf, "%d", val->uval.ival);
        ret = 6;
        break;
    case 'L':
        sprintf(buf, "%ld", val->uval.Lval);
        ret = 10;
        break;
    case '*':
        ret = 0;
        *buf = (char) ret;
        break;
    case 's':
        strncpy(buf, val->uval.sval, 16);
        buf[15] = 0;
        ret = 15;
        break;
    case 'l':
        strcpy(buf, val->uval.ch.list[val->uval.ch.val]);
        ret = val->uval.ch.vlen;
        break;
    default: // assume 0x100+n
        strcpy(buf, val->uval.sbuf);
        ret = val->type & 0xff;
    }
    return ret;
}

int prompt_checkkey(int curkey)
{
    switch (curkey)
    {
    case FIK_PAGE_UP:
    case FIK_DOWN_ARROW:
    case FIK_PAGE_DOWN:
    case FIK_UP_ARROW:
        return curkey;
    case FIK_F2:
    case FIK_F3:
    case FIK_F4:
    case FIK_F5:
    case FIK_F6:
    case FIK_F7:
    case FIK_F8:
    case FIK_F9:
    case FIK_F10:
        if (promptfkeys & (1 << (curkey+1-FIK_F1)))
            return curkey;
    }
    return 0;
}

int prompt_checkkey_scroll(int curkey)
{
    switch (curkey)
    {
    case FIK_PAGE_UP:
    case FIK_DOWN_ARROW:
    case FIK_CTL_DOWN_ARROW:
    case FIK_PAGE_DOWN:
    case FIK_UP_ARROW:
    case FIK_CTL_UP_ARROW:
    case FIK_CTL_LEFT_ARROW:
    case FIK_CTL_RIGHT_ARROW:
    case FIK_CTL_PAGE_DOWN:
    case FIK_CTL_PAGE_UP:
    case FIK_CTL_END:
    case FIK_CTL_HOME:
        return curkey;
    case FIK_F2:
    case FIK_F3:
    case FIK_F4:
    case FIK_F5:
    case FIK_F6:
    case FIK_F7:
    case FIK_F8:
    case FIK_F9:
    case FIK_F10:
        if (promptfkeys & (1 << (curkey+1-FIK_F1)))
            return curkey;
    }
    return 0;
}

static int input_field_list(
    int attr,             // display attribute
    char *fld,            // display form field value
    int vlen,             // field length
    const char **list,          // list of values
    int llen,             // number of entries in list
    int row,              // display row
    int col,              // display column
    int (*checkkey)(int)  // routine to check non data keys, or nullptr
)
{
    int initval, curval;
    char buf[81];
    int curkey;
    int ret, savelookatmouse;
    savelookatmouse = lookatmouse;
    lookatmouse = 0;
    for (initval = 0; initval < llen; ++initval)
        if (strcmp(fld, list[initval]) == 0)
            break;
    if (initval >= llen)
        initval = 0;
    curval = initval;
    ret = -1;
    while (1)
    {
        strcpy(buf, list[curval]);
        {
            int i = (int) strlen(buf);
            while (i < vlen)
                buf[i++] = ' ';
        }
        buf[vlen] = 0;
        driver_put_string(row, col, attr, buf);
        curkey = driver_key_cursor(row, col); // get a keystroke
        switch (curkey)
        {
        case FIK_ENTER:
        case FIK_ENTER_2:
            ret = 0;
            goto inpfldl_end;
        case FIK_ESC:
            goto inpfldl_end;
        case FIK_RIGHT_ARROW:
            if (++curval >= llen)
                curval = 0;
            break;
        case FIK_LEFT_ARROW:
            if (--curval < 0)
                curval = llen - 1;
            break;
        case FIK_F5:
            curval = initval;
            break;
        default:
            if (nonalpha(curkey))
            {
                if (checkkey && (ret = (*checkkey)(curkey)) != 0)
                    goto inpfldl_end;
                break;                                // non alphanum char
            }
            int j = curval;
            for (int i = 0; i < llen; ++i)
            {
                if (++j >= llen)
                    j = 0;
                if ((*list[j] & 0xdf) == (curkey & 0xdf))
                {
                    curval = j;
                    break;
                }
            }
        }
    }
inpfldl_end:
    strcpy(fld, list[curval]);
    lookatmouse = savelookatmouse;
    return ret;
}

int get_fracttype()             // prompt for and select fractal type
{
    fractal_type t;
    int done = -1;
    fractal_type oldfractype = fractype;
    while (1)
    {
        t = select_fracttype(fractype);
        if (t == fractal_type::NOFRACTAL)
            break;
        bool i = select_type_params(t, fractype);
        if (!i)
        { // ok, all done
            done = 0;
            break;
        }
        if (i) // can't return to prior image anymore
            done = 1;
    }
    if (done < 0)
        fractype = oldfractype;
    curfractalspecific = &fractalspecific[static_cast<int>(fractype)];
    return done;
}

struct FT_CHOICE
{
    char name[15];
    int  num;
};
static FT_CHOICE **ft_choices; // for sel_fractype_help subrtn

static fractal_type select_fracttype(fractal_type t) // subrtn of get_fracttype, separated
// so that storage gets freed up
{
    int oldhelpmode;
    int numtypes;
#define MAXFTYPES 200
    char tname[40];
    FT_CHOICE storage[MAXFTYPES] = { 0 };
    FT_CHOICE *choices[MAXFTYPES];
    int attributes[MAXFTYPES];

    // steal existing array for "choices"
    choices[0] = &storage[0];
    attributes[0] = 1;
    for (int i = 1; i < MAXFTYPES; ++i)
    {
        choices[i] = &storage[i];
        attributes[i] = 1;
    }
    ft_choices = &choices[0];

    // setup context sensitive help
    oldhelpmode = helpmode;
    helpmode = HELPFRACTALS;
    if (t == fractal_type::IFS3D)
        t = fractal_type::IFS;
    {
        int i = -1;
        int j = -1;
        while (fractalspecific[++i].name)
        {
            if (julibrot)
                if (!((fractalspecific[i].flags & OKJB) && *fractalspecific[i].name != '*'))
                    continue;
            if (fractalspecific[i].name[0] == '*')
                continue;
            strcpy(choices[++j]->name, fractalspecific[i].name);
            choices[j]->name[14] = 0; // safety
            choices[j]->num = i;      // remember where the real item is
        }
        numtypes = j + 1;
    }
    shell_sort(&choices, numtypes, sizeof(FT_CHOICE *), lccompare); // sort list
    int j = 0;
    for (int i = 0; i < numtypes; ++i) // find starting choice in sorted list
        if (choices[i]->num == static_cast<int>(t)
            || choices[i]->num == static_cast<int>(fractalspecific[static_cast<int>(t)].tofloat))
        {
            j = i;
        }

    tname[0] = 0;
    int done = fullscreen_choice(CHOICE_HELP | CHOICE_INSTRUCTIONS,
            julibrot ? "Select Orbit Algorithm for Julibrot" : "Select a Fractal Type",
            nullptr, "Press " FK_F2 " for a description of the highlighted type", numtypes,
            (const char **)choices, attributes, 0, 0, 0, j, nullptr, tname, nullptr, sel_fractype_help);
    fractal_type result = fractal_type::NOFRACTAL;
    if (done >= 0)
    {
        result = static_cast<fractal_type>(choices[done]->num);
        if ((result == fractal_type::FORMULA || result == fractal_type::FFORMULA) && !strcmp(FormFileName, CommandFile))
            strcpy(FormFileName, searchfor.frm);
        if (result == fractal_type::LSYSTEM && !strcmp(LFileName, CommandFile))
            strcpy(LFileName, searchfor.lsys);
        if ((result == fractal_type::IFS || result == fractal_type::IFS3D) && !strcmp(IFSFileName, CommandFile))
            strcpy(IFSFileName, searchfor.ifs);
    }


    helpmode = oldhelpmode;
    return result;
}

static int sel_fractype_help(int curkey, int choice)
{
    if (curkey == FIK_F2)
    {
        int oldhelpmode = helpmode;
        helpmode = fractalspecific[(*(ft_choices+choice))->num].helptext;
        help(0);
        helpmode = oldhelpmode;
    }
    return 0;
}

bool select_type_params( // prompt for new fractal type parameters
    fractal_type newfractype,        // new fractal type
    fractal_type oldfractype         // previous fractal type
)
{
    bool ret;

    int oldhelpmode = helpmode;

sel_type_restart:
    ret = false;
    fractype = newfractype;
    curfractalspecific = &fractalspecific[static_cast<int>(fractype)];

    if (fractype == fractal_type::LSYSTEM)
    {
        helpmode = HT_LSYS;
        if (get_file_entry(GETLSYS, "L-System", lsysmask, LFileName, LName) < 0)
        {
            ret = true;
            goto sel_type_exit;
        }
    }
    if (fractype == fractal_type::FORMULA || fractype == fractal_type::FFORMULA)
    {
        helpmode = HT_FORMULA;
        if (get_file_entry(GETFORMULA, "Formula", formmask, FormFileName, FormName) < 0)
        {
            ret = true;
            goto sel_type_exit;
        }
    }
    if (fractype == fractal_type::IFS || fractype == fractal_type::IFS3D)
    {
        helpmode = HT_IFS;
        if (get_file_entry(GETIFS, "IFS", ifsmask, IFSFileName, IFSName) < 0)
        {
            ret = true;
            goto sel_type_exit;
        }
    }

    if (((fractype == fractal_type::BIFURCATION) || (fractype == fractal_type::LBIFURCATION)) &&
            !((oldfractype == fractal_type::BIFURCATION) || (oldfractype == fractal_type::LBIFURCATION)))
        set_trig_array(0, "ident");
    if (((fractype == fractal_type::BIFSTEWART) || (fractype == fractal_type::LBIFSTEWART)) &&
            !((oldfractype == fractal_type::BIFSTEWART) || (oldfractype == fractal_type::LBIFSTEWART)))
        set_trig_array(0, "ident");
    if (((fractype == fractal_type::BIFLAMBDA) || (fractype == fractal_type::LBIFLAMBDA)) &&
            !((oldfractype == fractal_type::BIFLAMBDA) || (oldfractype == fractal_type::LBIFLAMBDA)))
        set_trig_array(0, "ident");
    if (((fractype == fractal_type::BIFEQSINPI) || (fractype == fractal_type::LBIFEQSINPI)) &&
            !((oldfractype == fractal_type::BIFEQSINPI) || (oldfractype == fractal_type::LBIFEQSINPI)))
        set_trig_array(0, "sin");
    if (((fractype == fractal_type::BIFADSINPI) || (fractype == fractal_type::LBIFADSINPI)) &&
            !((oldfractype == fractal_type::BIFADSINPI) || (oldfractype == fractal_type::LBIFADSINPI)))
        set_trig_array(0, "sin");

    /*
     * Next assumes that user going between popcorn and popcornjul
     * might not want to change function variables
     */
    if (((fractype    == fractal_type::FPPOPCORN) || (fractype    == fractal_type::LPOPCORN) ||
            (fractype    == fractal_type::FPPOPCORNJUL) || (fractype    == fractal_type::LPOPCORNJUL)) &&
            !((oldfractype == fractal_type::FPPOPCORN) || (oldfractype == fractal_type::LPOPCORN) ||
              (oldfractype == fractal_type::FPPOPCORNJUL) || (oldfractype == fractal_type::LPOPCORNJUL)))
        set_function_parm_defaults();

    // set LATOO function defaults
    if (fractype == fractal_type::LATOO && oldfractype != fractal_type::LATOO)
    {
        set_function_parm_defaults();
    }
    set_default_parms();

    if (get_fract_params(0) < 0)
        if (fractype == fractal_type::FORMULA || fractype == fractal_type::FFORMULA ||
                fractype == fractal_type::IFS || fractype == fractal_type::IFS3D ||
                fractype == fractal_type::LSYSTEM)
            goto sel_type_restart;
        else
            ret = true;
    else
    {
        if (newfractype != oldfractype)
        {
            invert = 0;
            inversion[2] = 0;
            inversion[1] = inversion[2];
            inversion[0] = inversion[1];
        }
    }

sel_type_exit:
    helpmode = oldhelpmode;
    return ret;
}

void set_default_parms()
{
    xxmin = curfractalspecific->xmin;
    xxmax = curfractalspecific->xmax;
    yymin = curfractalspecific->ymin;
    yymax = curfractalspecific->ymax;
    xx3rd = xxmin;
    yy3rd = yymin;

    if (viewcrop && finalaspectratio != screenaspect)
        aspectratio_crop(screenaspect, finalaspectratio);
    for (int i = 0; i < 4; i++)
    {
        param[i] = curfractalspecific->paramvalue[i];
        if (fractype != fractal_type::CELLULAR && fractype != fractal_type::FROTH && fractype != fractal_type::FROTHFP &&
                fractype != fractal_type::ANT)
            roundfloatd(&param[i]); // don't round cellular, frothybasin or ant
    }
    int extra = find_extra_param(fractype);
    if (extra > -1)
        for (int i = 0; i < MAXPARAMS-4; i++)
            param[i+4] = moreparams[extra].paramvalue[i];
    if (debugflag != debug_flags::force_arbitrary_precision_math)
        bf_math = bf_math_type::NONE;
    else if (bf_math != bf_math_type::NONE)
        fractal_floattobf();
}

#define MAXFRACTALS 25

int build_fractal_list(int fractals[], int *last_val, const char *nameptr[])
{
    int numfractals = 0;
    for (int i = 0; i < num_fractal_types; i++)
    {
        if ((fractalspecific[i].flags & OKJB) && *fractalspecific[i].name != '*')
        {
            fractals[numfractals] = i;
            if (i == static_cast<int>(neworbittype)
                    || i == static_cast<int>(fractalspecific[static_cast<int>(neworbittype)].tofloat))
                *last_val = numfractals;
            nameptr[numfractals] = fractalspecific[i].name;
            numfractals++;
            if (numfractals >= MAXFRACTALS)
                break;
        }
    }
    return numfractals;
}

const char *juli3Doptions[] = {"monocular", "lefteye", "righteye", "red-blue"};

// JIIM
#ifdef RANDOM_RUN
static char JIIMstr1[] = "Breadth first, Depth first, Random Walk, Random Run?";
const char *JIIMmethod[] =
{
    "breadth", "depth", "walk", "run"
};
#else
static char JIIMstr1[] = "Breadth first, Depth first, Random Walk";
const char *JIIMmethod[] =
{
    "breadth", "depth", "walk"
};
#endif
static char JIIMstr2[] = "Left first or Right first?";
const char *JIIMleftright[] = {"left", "right"};

// The index into this array must correspond to enum trig_fn
trig_funct_lst trigfn[] =
// changing the order of these alters meaning of *.fra file
// maximum 6 characters in function names or recheck all related code
{
    {"sin",   dStkSin,   dStkSin,   dStkSin   },
    {"cosxx", dStkCosXX, dStkCosXX, dStkCosXX },
    {"sinh",  dStkSinh,  dStkSinh,  dStkSinh  },
    {"cosh",  dStkCosh,  dStkCosh,  dStkCosh  },
    {"exp",   dStkExp,   dStkExp,   dStkExp   },
    {"log",   dStkLog,   dStkLog,   dStkLog   },
    {"sqr",   dStkSqr,   dStkSqr,   dStkSqr   },
    {"recip", dStkRecip, dStkRecip, dStkRecip }, // from recip on new in v16
    {"ident", StkIdent,  StkIdent,  StkIdent  },
    {"cos",   dStkCos,   dStkCos,   dStkCos   },
    {"tan",   dStkTan,   dStkTan,   dStkTan   },
    {"tanh",  dStkTanh,  dStkTanh,  dStkTanh  },
    {"cotan", dStkCoTan, dStkCoTan, dStkCoTan },
    {"cotanh", dStkCoTanh, dStkCoTanh, dStkCoTanh},
    {"flip",  dStkFlip,  dStkFlip,  dStkFlip  },
    {"conj",  dStkConj,  dStkConj,  dStkConj  },
    {"zero",  dStkZero,  dStkZero,  dStkZero  },
    {"asin",  dStkASin,  dStkASin,  dStkASin  },
    {"asinh", dStkASinh, dStkASinh, dStkASinh },
    {"acos",  dStkACos,  dStkACos,  dStkACos  },
    {"acosh", dStkACosh, dStkACosh, dStkACosh },
    {"atan",  dStkATan,  dStkATan,  dStkATan  },
    {"atanh", dStkATanh, dStkATanh, dStkATanh },
    {"cabs",  dStkCAbs,  dStkCAbs,  dStkCAbs  },
    {"abs",   dStkAbs,   dStkAbs,   dStkAbs   },
    {"sqrt",  dStkSqrt,  dStkSqrt,  dStkSqrt  },
    {"floor", dStkFloor, dStkFloor, dStkFloor },
    {"ceil",  dStkCeil,  dStkCeil,  dStkCeil  },
    {"trunc", dStkTrunc, dStkTrunc, dStkTrunc },
    {"round", dStkRound, dStkRound, dStkRound },
    {"one",   dStkOne,   dStkOne,   dStkOne   },
};

#define NUMTRIGFN  sizeof(trigfn)/sizeof(trig_funct_lst)

const int numtrigfn = NUMTRIGFN;

char tstack[4096] = { 0 };

// ---------------------------------------------------------------------
int get_fract_params(int caller)        // prompt for type-specific parms
{
    const char *v0 = "From cx (real part)";
    const char *v1 = "From cy (imaginary part)";
    const char *v2 = "To   cx (real part)";
    const char *v3 = "To   cy (imaginary part)";
    const char *juliorbitname = nullptr;
    int numparams, numtrig;
    fullscreenvalues paramvalues[30];
    const char *choices[30];
    long oldbailout = 0L;
    int promptnum;
    char msg[120];
    const char *type_name;
    const char *tmpptr;
    char bailoutmsg[50];
    int ret = 0;
    int oldhelpmode;
    char parmprompt[MAXPARAMS][55];
    static const char *trg[] =
    {
        "First Function", "Second Function", "Third Function", "Fourth Function"
    };
    char *filename, *entryname;
    FILE *entryfile;
    const char *trignameptr[NUMTRIGFN];
#ifdef XFRACT
    static // Can't initialize aggregates on the stack
#endif
    const char *bailnameptr[] = {"mod", "real", "imag", "or", "and", "manh", "manr"};
    fractalspecificstuff *jborbit = nullptr;
    int firstparm = 0;
    int lastparm  = MAXPARAMS;
    double oldparam[MAXPARAMS];
    int fkeymask = 0x40;
    oldbailout = bailout;
    if (fractype == fractal_type::JULIBROT || fractype == fractal_type::JULIBROTFP)
        julibrot = true;
    else
        julibrot = false;
    fractal_type curtype = fractype;
    {
        int i;
        if (curfractalspecific->name[0] == '*'
                && (i = static_cast<int>(curfractalspecific->tofloat)) != static_cast<int>(fractal_type::NOFRACTAL)
                && fractalspecific[i].name[0] != '*')
            curtype = static_cast<fractal_type>(i);
    }
    curfractalspecific = &fractalspecific[static_cast<int>(curtype)];
    tstack[0] = 0;
    int help_formula = curfractalspecific->helpformula;
    if (help_formula < -1)
    {
        if (help_formula == -2)
        { // special for formula
            filename = FormFileName;
            entryname = FormName;
        }
        else if (help_formula == -3)
        {       // special for lsystem
            filename = LFileName;
            entryname = LName;
        }
        else if (help_formula == -4)
        {       // special for ifs
            filename = IFSFileName;
            entryname = IFSName;
        }
        else
        { // this shouldn't happen
            filename = nullptr;
            entryname = nullptr;
        }
        if (find_file_item(filename, entryname, &entryfile, -1-help_formula) == 0)
        {
            load_entry_text(entryfile, tstack, 17, 0, 0);
            fclose(entryfile);
            if (fractype == fractal_type::FORMULA || fractype == fractal_type::FFORMULA)
                frm_get_param_stuff(entryname); // no error check, should be okay, from above
        }
    }
    else if (help_formula >= 0)
    {
        int c, lines;
        read_help_topic(help_formula, 0, 2000, tstack); // need error handling here ??
        tstack[2000-help_formula] = 0;
        int i = 0;
        lines = 0;
        int j = 0;
        int k = 1;
        while ((c = tstack[i++]) != 0)
        {
            // stop at ctl, blank, or line with col 1 nonblank, max 16 lines
            if (k && c == ' ' && ++k <= 5)
            {
            } // skip 4 blanks at start of line
            else
            {
                if (c == '\n')
                {
                    if (k)
                        break; // blank line
                    if (++lines >= 16)
                        break;
                    k = 1;
                }
                else if (c < 16) // a special help format control char
                    break;
                else
                {
                    if (k == 1) // line starts in column 1
                        break;
                    k = 0;
                }
                tstack[j++] = (char)c;
            }
        }
        while (--j >= 0 && tstack[j] == '\n')
        {
        }
        tstack[j+1] = 0;
    }
    fractalspecificstuff *savespecific = curfractalspecific;
    int orbit_bailout;

gfp_top:
    promptnum = 0;
    if (julibrot)
    {
        fractal_type i = select_fracttype(neworbittype);
        if (i == fractal_type::NOFRACTAL)
        {
            if (ret == 0)
                ret = -1;
            julibrot = false;
            goto gfp_exit;
        }
        else
            neworbittype = i;
        jborbit = &fractalspecific[static_cast<int>(neworbittype)];
        juliorbitname = jborbit->name;
    }

    if (fractype == fractal_type::FORMULA || fractype == fractal_type::FFORMULA)
    {
        if (uses_p1)  // set first parameter
            firstparm = 0;
        else if (uses_p2)
            firstparm = 2;
        else if (uses_p3)
            firstparm = 4;
        else if (uses_p4)
            firstparm = 6;
        else
            firstparm = 8; // uses_p5 or no parameter

        if (uses_p5)  // set last parameter
            lastparm = 10;
        else if (uses_p4)
            lastparm = 8;
        else if (uses_p3)
            lastparm = 6;
        else if (uses_p2)
            lastparm = 4;
        else
            lastparm = 2; // uses_p1 or no parameter
    }

    if (julibrot)
    {
        curfractalspecific = jborbit;
        firstparm = 2; // in most case Julibrot does not need first two parms
        if (neworbittype == fractal_type::QUATJULFP     ||   // all parameters needed
                neworbittype == fractal_type::HYPERCMPLXJFP)
        {
            firstparm = 0;
            lastparm = 4;
        }
        if (neworbittype == fractal_type::QUATFP        ||   // no parameters needed
                neworbittype == fractal_type::HYPERCMPLXFP)
            firstparm = 4;
    }
    numparams = 0;
    {
        int j = 0;
        for (int i = firstparm; i < lastparm; i++)
        {
            char tmpbuf[30];
            if (!typehasparm(julibrot ? neworbittype : fractype, i, parmprompt[j]))
            {
                if (curtype == fractal_type::FORMULA || curtype == fractal_type::FFORMULA)
                    if (paramnotused(i))
                        continue;
                break;
            }
            numparams++;
            choices[promptnum] = parmprompt[j++];
            paramvalues[promptnum].type = 'd';

            if (choices[promptnum][0] == '+')
            {
                choices[promptnum]++;
                paramvalues[promptnum].type = 'D';
            }
            else if (choices[promptnum][0] == '#')
                choices[promptnum]++;
            sprintf(tmpbuf, "%.17g", param[i]);
            paramvalues[promptnum].uval.dval = atof(tmpbuf);
            oldparam[i] = paramvalues[promptnum++].uval.dval;
        }
    }

    /* The following is a goofy kludge to make reading in the formula
     * parameters work.
     */
    if (curtype == fractal_type::FORMULA || curtype == fractal_type::FFORMULA)
        numparams = lastparm - firstparm;

    numtrig = (curfractalspecific->flags >> 6) & 7;
    if (curtype == fractal_type::FORMULA || curtype == fractal_type::FFORMULA)
    {
        numtrig = maxfn;
    }

    for (int i = NUMTRIGFN-1; i >= 0; --i)
        trignameptr[i] = trigfn[i].name;
    for (int i = 0; i < numtrig; i++)
    {
        paramvalues[promptnum].type = 'l';
        paramvalues[promptnum].uval.ch.val  = static_cast<int>(trigndx[i]);
        paramvalues[promptnum].uval.ch.llen = NUMTRIGFN;
        paramvalues[promptnum].uval.ch.vlen = 6;
        paramvalues[promptnum].uval.ch.list = trignameptr;
        choices[promptnum++] = (char *)trg[i];
    }
    type_name = curfractalspecific->name;
    if (*type_name == '*')
        ++type_name;

    orbit_bailout = curfractalspecific->orbit_bailout;
    if (orbit_bailout != 0 && curfractalspecific->calctype == StandardFractal &&
            (curfractalspecific->flags & BAILTEST))
    {
        paramvalues[promptnum].type = 'l';
        paramvalues[promptnum].uval.ch.val  = static_cast<int>(bailoutest);
        paramvalues[promptnum].uval.ch.llen = 7;
        paramvalues[promptnum].uval.ch.vlen = 6;
        paramvalues[promptnum].uval.ch.list = bailnameptr;
        choices[promptnum++] = "Bailout Test (mod, real, imag, or, and, manh, manr)";
    }

    if (orbit_bailout)
    {
        if (potparam[0] != 0.0 && potparam[2] != 0.0)
        {
            paramvalues[promptnum].type = '*';
            choices[promptnum++] = "Bailout: continuous potential (Y screen) value in use";
        }
        else
        {
            choices[promptnum] = "Bailout value (0 means use default)";
            paramvalues[promptnum].type = 'L';
            oldbailout = bailout;
            paramvalues[promptnum++].uval.Lval = oldbailout;
            paramvalues[promptnum].type = '*';
            tmpptr = type_name;
            if (usr_biomorph != -1)
            {
                orbit_bailout = 100;
                tmpptr = "biomorph";
            }
            sprintf(bailoutmsg, "    (%s default is %d)", tmpptr, orbit_bailout);
            choices[promptnum++] = bailoutmsg;
        }
    }
    if (julibrot)
    {
        switch (neworbittype)
        {
        case fractal_type::QUATFP:
        case fractal_type::HYPERCMPLXFP:
            v0 = "From cj (3rd dim)";
            v1 = "From ck (4th dim)";
            v2 = "To   cj (3rd dim)";
            v3 = "To   ck (4th dim)";
            break;
        case fractal_type::QUATJULFP:
        case fractal_type::HYPERCMPLXJFP:
            v0 = "From zj (3rd dim)";
            v1 = "From zk (4th dim)";
            v2 = "To   zj (3rd dim)";
            v3 = "To   zk (4th dim)";
            break;
        default:
            v0 = "From cx (real part)";
            v1 = "From cy (imaginary part)";
            v2 = "To   cx (real part)";
            v3 = "To   cy (imaginary part)";
            break;
        }

        curfractalspecific = savespecific;
        paramvalues[promptnum].uval.dval = mxmaxfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = v0;
        paramvalues[promptnum].uval.dval = mymaxfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = v1;
        paramvalues[promptnum].uval.dval = mxminfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = v2;
        paramvalues[promptnum].uval.dval = myminfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = v3;
        paramvalues[promptnum].uval.ival = zdots;
        paramvalues[promptnum].type = 'i';
        choices[promptnum++] = "Number of z pixels";

        paramvalues[promptnum].type = 'l';
        paramvalues[promptnum].uval.ch.val  = juli3Dmode;
        paramvalues[promptnum].uval.ch.llen = 4;
        paramvalues[promptnum].uval.ch.vlen = 9;
        paramvalues[promptnum].uval.ch.list = juli3Doptions;
        choices[promptnum++] = "3D Mode";

        paramvalues[promptnum].uval.dval = eyesfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = "Distance between eyes";
        paramvalues[promptnum].uval.dval = originfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = "Location of z origin";
        paramvalues[promptnum].uval.dval = depthfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = "Depth of z";
        paramvalues[promptnum].uval.dval = heightfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = "Screen height";
        paramvalues[promptnum].uval.dval = widthfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = "Screen width";
        paramvalues[promptnum].uval.dval = distfp;
        paramvalues[promptnum].type = 'f';
        choices[promptnum++] = "Distance to Screen";
    }

    if (curtype == fractal_type::INVERSEJULIA || curtype == fractal_type::INVERSEJULIAFP)
    {
        choices[promptnum] = JIIMstr1;
        paramvalues[promptnum].type = 'l';
        paramvalues[promptnum].uval.ch.list = JIIMmethod;
        paramvalues[promptnum].uval.ch.vlen = 7;
#ifdef RANDOM_RUN
        paramvalues[promptnum].uval.ch.llen = 4;
#else
        paramvalues[promptnum].uval.ch.llen = 3; // disable random run
#endif
        paramvalues[promptnum++].uval.ch.val  = static_cast<int>(major_method);

        choices[promptnum] = JIIMstr2;
        paramvalues[promptnum].type = 'l';
        paramvalues[promptnum].uval.ch.list = JIIMleftright;
        paramvalues[promptnum].uval.ch.vlen = 5;
        paramvalues[promptnum].uval.ch.llen = 2;
        paramvalues[promptnum++].uval.ch.val  = static_cast<int>(minor_method);
    }

    if ((curtype == fractal_type::FORMULA || curtype == fractal_type::FFORMULA) && uses_ismand)
    {
        choices[promptnum] = "ismand";
        paramvalues[promptnum].type = 'y';
        paramvalues[promptnum++].uval.ch.val = ismand ? 1 : 0;
    }

    if (caller                           // <z> command ?
            //      && (display3d > 0 || promptnum == 0))
            && (display3d > 0))
    {
        stopmsg(STOPMSG_INFO_ONLY | STOPMSG_NO_BUZZER, "Current type has no type-specific parameters");
        goto gfp_exit;
    }
    if (julibrot)
        sprintf(msg, "Julibrot Parameters (orbit=%s)", juliorbitname);
    else
        sprintf(msg, "Parameters for fractal type %s", type_name);
    if (bf_math == bf_math_type::NONE)
    {
        strcat(msg, "\n(Press " FK_F6 " for corner parameters)");
    }
    else
        fkeymask = 0;
    scroll_row_status = 0; // make sure we start at beginning of entry
    scroll_column_status = 0;
    while (1)
    {
        oldhelpmode = helpmode;
        helpmode = curfractalspecific->helptext;
        int i = fullscreen_prompt(msg, promptnum, choices, paramvalues, fkeymask, tstack);
        helpmode = oldhelpmode;
        if (i < 0)
        {
            if (julibrot)
                goto gfp_top;
            if (ret == 0)
                ret = -1;
            goto gfp_exit;
        }
        if (i != FIK_F6)
            break;
        if (bf_math == bf_math_type::NONE)
            if (get_corners() > 0)
                ret = 1;
    }
    promptnum = 0;
    for (int i = firstparm; i < numparams+firstparm; i++)
    {
        if (curtype == fractal_type::FORMULA || curtype == fractal_type::FFORMULA)
            if (paramnotused(i))
                continue;
        if (oldparam[i] != paramvalues[promptnum].uval.dval)
        {
            param[i] = paramvalues[promptnum].uval.dval;
            ret = 1;
        }
        ++promptnum;
    }

    for (int i = 0; i < numtrig; i++)
    {
        if (paramvalues[promptnum].uval.ch.val != (int)trigndx[i])
        {
            set_trig_array(i, trigfn[paramvalues[promptnum].uval.ch.val].name);
            ret = 1;
        }
        ++promptnum;
    }

    if (julibrot)
    {
        curfractalspecific = jborbit;
    }

    orbit_bailout = curfractalspecific->orbit_bailout;
    if (orbit_bailout != 0 && curfractalspecific->calctype == StandardFractal &&
            (curfractalspecific->flags & BAILTEST))
    {
        if (paramvalues[promptnum].uval.ch.val != static_cast<int>(bailoutest))
        {
            bailoutest = static_cast<bailouts>(paramvalues[promptnum].uval.ch.val);
            ret = 1;
        }
        promptnum++;
    }
    else
        bailoutest = bailouts::Mod;
    setbailoutformula(bailoutest);

    if (orbit_bailout)
    {
        if (potparam[0] != 0.0 && potparam[2] != 0.0)
            promptnum++;
        else
        {
            bailout = paramvalues[promptnum++].uval.Lval;
            if (bailout != 0 && (bailout < 1 || bailout > 2100000000L))
                bailout = oldbailout;
            if (bailout != oldbailout)
                ret = 1;
            promptnum++;
        }
    }

    if (julibrot)
    {
        mxmaxfp    = paramvalues[promptnum++].uval.dval;
        mymaxfp    = paramvalues[promptnum++].uval.dval;
        mxminfp    = paramvalues[promptnum++].uval.dval;
        myminfp    = paramvalues[promptnum++].uval.dval;
        zdots      = paramvalues[promptnum++].uval.ival;
        juli3Dmode = paramvalues[promptnum++].uval.ch.val;
        eyesfp     = (float)paramvalues[promptnum++].uval.dval;
        originfp   = (float)paramvalues[promptnum++].uval.dval;
        depthfp    = (float)paramvalues[promptnum++].uval.dval;
        heightfp   = (float)paramvalues[promptnum++].uval.dval;
        widthfp    = (float)paramvalues[promptnum++].uval.dval;
        distfp     = (float)paramvalues[promptnum++].uval.dval;
        ret = 1;  // force new calc since not resumable anyway
    }
    if (curtype == fractal_type::INVERSEJULIA || curtype == fractal_type::INVERSEJULIAFP)
    {
        if (paramvalues[promptnum].uval.ch.val != static_cast<int>(major_method) ||
                paramvalues[promptnum+1].uval.ch.val != static_cast<int>(minor_method))
            ret = 1;
        major_method = static_cast<Major>(paramvalues[promptnum++].uval.ch.val);
        minor_method = static_cast<Minor>(paramvalues[promptnum++].uval.ch.val);
    }
    if ((curtype == fractal_type::FORMULA || curtype == fractal_type::FFORMULA) && uses_ismand)
    {
        if (ismand != (paramvalues[promptnum].uval.ch.val != 0))
        {
            ismand = (paramvalues[promptnum].uval.ch.val != 0);
            ret = 1;
        }
        ++promptnum;
    }
gfp_exit:
    curfractalspecific = &fractalspecific[static_cast<int>(fractype)];
    return ret;
}

int find_extra_param(fractal_type type)
{
    int i, ret;
    fractal_type curtyp;
    ret = -1;
    i = -1;

    if (fractalspecific[static_cast<int>(type)].flags & MORE)
    {
        while ((curtyp = moreparams[++i].type) != type && curtyp != fractal_type::NOFRACTAL);
        if (curtyp == type)
            ret = i;
    }
    return ret;
}

void load_params(fractal_type fractype)
{
    for (int i = 0; i < 4; ++i)
    {
        param[i] = fractalspecific[static_cast<int>(fractype)].paramvalue[i];
        if (fractype != fractal_type::CELLULAR && fractype != fractal_type::ANT)
            roundfloatd(&param[i]); // don't round cellular or ant
    }
    int extra = find_extra_param(fractype);
    if (extra > -1)
        for (int i = 0; i < MAXPARAMS-4; i++)
            param[i+4] = moreparams[extra].paramvalue[i];
}

bool check_orbit_name(char *orbitname)
{
    int numtypes;
    const char *nameptr[MAXFRACTALS];
    int fractals[MAXFRACTALS];
    int last_val;

    numtypes = build_fractal_list(fractals, &last_val, nameptr);
    bool bad = true;
    for (int i = 0; i < numtypes; i++)
    {
        if (strcmp(orbitname, nameptr[i]) == 0)
        {
            neworbittype = static_cast<fractal_type>(fractals[i]);
            bad = false;
            break;
        }
    }
    return bad;
}

// ---------------------------------------------------------------------

static FILE *gfe_file;

long get_file_entry(int type, const char *title, char *fmask,
                    char *filename, char *entryname)
{
    // Formula, LSystem, etc type structure, select from file
    // containing definitions in the form    name { ... }
    bool firsttry;
    long entry_pointer;
    bool newfile = false;
    while (1)
    {
        firsttry = false;
        // binary mode used here - it is more work, but much faster,
        //     especially when ftell or fgetpos is used
        while (newfile || (gfe_file = fopen(filename, "rb")) == nullptr)
        {
            char buf[60];
            newfile = false;
            if (firsttry)
            {
                sprintf(temp1, "Can't find %s", filename);
                stopmsg(STOPMSG_NONE, temp1);
            }
            sprintf(buf, "Select %s File", title);
            if (getafilename(buf, fmask, filename))
                return -1;

            firsttry = true; // if around open loop again it is an error
        }
        setvbuf(gfe_file, tstack, _IOFBF, 4096); // improves speed when file is big
        newfile = false;
        entry_pointer = gfe_choose_entry(type, title, filename, entryname);
        if (entry_pointer == -2)
        {
            newfile = true; // go to file list,
            continue;    // back to getafilename
        }
        if (entry_pointer == -1)
            return -1;
        switch (type)
        {
        case GETFORMULA:
            if (!RunForm(entryname, true))
                return 0;
            break;
        case GETLSYS:
            if (LLoad() == 0)
                return 0;
            break;
        case GETIFS:
            if (ifsload() == 0)
            {
                fractype = !ifs_type ? fractal_type::IFS : fractal_type::IFS3D;
                curfractalspecific = &fractalspecific[static_cast<int>(fractype)];
                set_default_parms(); // to correct them if 3d
                return 0;
            }
            break;
        case GETPARM:
            return entry_pointer;
        }
    }
}

struct entryinfo
{
    char name[ITEMNAMELEN+2];
    long point; // points to the ( or the { following the name
};
static entryinfo **gfe_choices; // for format_getparm_line
static const char *gfe_title;

// skip to next non-white space character and return it
int skip_white_space(FILE *infile, long *file_offset)
{
    int c;
    do
    {
        c = getc(infile);
        (*file_offset)++;
    }
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    return c;
}

// skip to end of line
int skip_comment(FILE *infile, long *file_offset)
{
    int c;
    do
    {
        c = getc(infile);
        (*file_offset)++;
    }
    while (c != '\n' && c != '\r' && c != EOF && c != '\032');
    return c;
}

#define MAXENTRIES 2000L

int scan_entries(FILE * infile, entryinfo *choices, char *itemname)
{
    /*
    function returns the number of entries found; if a
    specific entry is being looked for, returns -1 if
    the entry is found, 0 otherwise.
    */
    char buf[101];
    int exclude_entry;
    long name_offset, temp_offset;
    long file_offset = -1;
    int numentries = 0;

    while (true)
    {   // scan the file for entry names
        int c, len;
top:
        c = skip_white_space(infile, &file_offset);
        if (c == ';')
        {
            c = skip_comment(infile, &file_offset);
            if (c == EOF || c == '\032')
                break;
            continue;
        }
        temp_offset = file_offset;
        name_offset = temp_offset;
        // next equiv roughly to fscanf(..,"%40[^* \n\r\t({\032]",buf)
        len = 0;
        // allow spaces in entry names in next
        while (c != ' ' && c != '\t' && c != '(' && c != ';'
                && c != '{' && c != '\n' && c != '\r' && c != EOF && c != '\032')
        {
            if (len < 40)
                buf[len++] = (char) c;
            c = getc(infile);
            ++file_offset;
            if (c == '\n' || c == '\r')
                goto top;
        }
        buf[len] = 0;
        while (c != '{' &&  c != EOF && c != '\032')
        {
            if (c == ';')
                c = skip_comment(infile, &file_offset);
            else
            {
                c = getc(infile);
                ++file_offset;
                if (c == '\n' || c == '\r')
                    goto top;
            }
        }
        if (c == '{')
        {
            while (c != '}' && c != EOF && c != '\032')
            {
                if (c == ';')
                    c = skip_comment(infile, &file_offset);
                else
                {
                    if (c == '\n' || c == '\r')     // reset temp_offset to
                        temp_offset = file_offset;  // beginning of new line
                    c = getc(infile);
                    ++file_offset;
                }
                if (c == '{') //second '{' found
                {
                    if (temp_offset == name_offset) //if on same line, skip line
                    {
                        skip_comment(infile, &file_offset);
                        goto top;
                    }
                    else
                    {
                        fseek(infile, temp_offset, SEEK_SET); //else, go back to
                        file_offset = temp_offset - 1;        //beginning of line
                        goto top;
                    }
                }
            }
            if (c != '}')   // i.e. is EOF or '\032'
                break;

            if (strnicmp(buf, "frm:", 4) == 0 ||
                    strnicmp(buf, "ifs:", 4) == 0 ||
                    strnicmp(buf, "par:", 4) == 0)
                exclude_entry = 4;
            else if (strnicmp(buf, "lsys:", 5) == 0)
                exclude_entry = 5;
            else
                exclude_entry = 0;

            buf[ITEMNAMELEN + exclude_entry] = 0;
            if (itemname != nullptr)  // looking for one entry
            {
                if (stricmp(buf, itemname) == 0)
                {
                    fseek(infile, name_offset + (long) exclude_entry, SEEK_SET);
                    return -1;
                }
            }
            else // make a whole list of entries
            {
                if (buf[0] != 0 && stricmp(buf, "comment") != 0 && !exclude_entry)
                {
                    strcpy(choices[numentries].name, buf);
                    choices[numentries].point = name_offset;
                    if (++numentries >= MAXENTRIES)
                    {
                        sprintf(buf, "Too many entries in file, first %ld used", MAXENTRIES);
                        stopmsg(STOPMSG_NONE, buf);
                        break;
                    }
                }
            }
        }
        else if (c == EOF || c == '\032')
            break;
    }
    return numentries;
}

// subrtn of get_file_entry, separated so that storage gets freed up
static long gfe_choose_entry(int type, const char *title, char *filename, char *entryname)
{
#ifdef XFRACT
    const char *o_instr = "Press " FK_F6 " to select file, " FK_F2 " for details, " FK_F4 " to toggle sort ";
    // keep the above line length < 80 characters
#else
    const char *o_instr = "Press " FK_F6 " to select different file, " FK_F2 " for details, " FK_F4 " to toggle sort ";
#endif
    int numentries;
    char buf[101];
    entryinfo storage[MAXENTRIES + 1];
    entryinfo *choices[MAXENTRIES + 1] = { nullptr };
    int attributes[MAXENTRIES + 1] = { 0 };
    void (*formatitem)(int, char *);
    int boxwidth, boxdepth, colwidth;
    char instr[80];

    static bool dosort = true;

    gfe_choices = &choices[0];
    gfe_title = title;

retry:
    for (int i = 0; i < MAXENTRIES+1; i++)
    {
        choices[i] = &storage[i];
        attributes[i] = 1;
    }

    helptitle(); // to display a clue when file big and next is slow

    numentries = scan_entries(gfe_file, &storage[0], nullptr);
    if (numentries == 0)
    {
        stopmsg(STOPMSG_NONE, "File doesn't contain any valid entries");
        fclose(gfe_file);
        return -2; // back to file list
    }
    strcpy(instr, o_instr);
    if (dosort)
    {
        strcat(instr, "off");
        shell_sort((char *) &choices, numentries, sizeof(entryinfo *), lccompare);
    }
    else
    {
        strcat(instr, "on");
    }

    strcpy(buf, entryname); // preset to last choice made
    sprintf(temp1, "%s Selection\nFile: %s", title, filename);
    formatitem = nullptr;
    boxdepth = 0;
    colwidth = boxdepth;
    boxwidth = colwidth;
    if (type == GETPARM)
    {
        formatitem = format_parmfile_line;
        boxwidth = 1;
        boxdepth = 16;
        colwidth = 76;
    }

    int i = fullscreen_choice(CHOICE_INSTRUCTIONS | (dosort ? 0 : CHOICE_NOT_SORTED),
                          temp1, nullptr, instr, numentries, (const char **) choices,
                          attributes, boxwidth, boxdepth, colwidth, 0,
                          formatitem, buf, nullptr, check_gfe_key);
    if (i == -FIK_F4)
    {
        rewind(gfe_file);
        dosort = !dosort;
        goto retry;
    }
    fclose(gfe_file);
    if (i < 0)
    {
        // go back to file list or cancel
        return (i == -FIK_F6) ? -2 : -1;
    }
    strcpy(entryname, choices[i]->name);
    return choices[i]->point;
}


static int check_gfe_key(int curkey, int choice)
{
    char infhdg[60];
    char infbuf[25*80];
    char blanks[79];         // used to clear the entry portion of screen
    memset(blanks, ' ', 78);
    blanks[78] = (char) 0;

    if (curkey == FIK_F6)
        return 0-FIK_F6;
    if (curkey == FIK_F4)
        return 0-FIK_F4;
    if (curkey == FIK_F2)
    {
        int widest_entry_line = 0;
        int lines_in_entry = 0;
        bool comment = false;
        int c = 0;
        int widthct = 0;
        fseek(gfe_file, gfe_choices[choice]->point, SEEK_SET);
        while ((c = fgetc(gfe_file)) != EOF && c != '\032')
        {
            if (c == ';')
                comment = true;
            else if (c == '\n')
            {
                comment = false;
                lines_in_entry++;
                widthct =  -1;
            }
            else if (c == '\t')
                widthct += 7 - widthct % 8;
            else if (c == '\r')
                continue;
            if (++widthct > widest_entry_line)
                widest_entry_line = widthct;
            if (c == '}' && !comment)
            {
                lines_in_entry++;
                break;
            }
        }
        bool in_scrolling_mode = false; // true if entry doesn't fit available space
        if (c == EOF || c == '\032')
        { // should never happen
            fseek(gfe_file, gfe_choices[choice]->point, SEEK_SET);
            in_scrolling_mode = false;
        }
        fseek(gfe_file, gfe_choices[choice]->point, SEEK_SET);
        load_entry_text(gfe_file, infbuf, 17, 0, 0);
        if (lines_in_entry > 17 || widest_entry_line > 74)
            in_scrolling_mode = true;
        strcpy(infhdg, gfe_title);
        strcat(infhdg, " file entry:\n\n");
        // ... instead, call help with buffer?  heading added
        driver_stack_screen();
        helptitle();
        driver_set_attr(1, 0, C_GENERAL_MED, 24*80);

        g_text_cbase = 0;
        driver_put_string(2, 1, C_GENERAL_HI, infhdg);
        g_text_cbase = 2; // left margin is 2
        driver_put_string(4, 0, C_GENERAL_MED, infbuf);
        driver_put_string(-1, 0, C_GENERAL_LO,
            "\n"
            "\n"
            " Use " UPARR1 ", " DNARR1 ", " RTARR1 ", " LTARR1
                ", PgUp, PgDown, Home, and End to scroll text\n"
            "Any other key to return to selection list");

        int top_line = 0;
        int left_column = 0;
        bool done = false;
        bool rewrite_infbuf = false;  // if true: rewrite the entry portion of screen
        while (!done)
        {
            if (rewrite_infbuf)
            {
                rewrite_infbuf = false;
                fseek(gfe_file, gfe_choices[choice]->point, SEEK_SET);
                load_entry_text(gfe_file, infbuf, 17, top_line, left_column);
                for (int i = 4; i < (lines_in_entry < 17 ? lines_in_entry + 4 : 21); i++)
                    driver_put_string(i, 0, C_GENERAL_MED, blanks);
                driver_put_string(4, 0, C_GENERAL_MED, infbuf);
            }
            int i = getakeynohelp();
            if (i == FIK_DOWN_ARROW        || i == FIK_CTL_DOWN_ARROW
                    || i == FIK_UP_ARROW       || i == FIK_CTL_UP_ARROW
                    || i == FIK_LEFT_ARROW     || i == FIK_CTL_LEFT_ARROW
                    || i == FIK_RIGHT_ARROW    || i == FIK_CTL_RIGHT_ARROW
                    || i == FIK_HOME           || i == FIK_CTL_HOME
                    || i == FIK_END            || i == FIK_CTL_END
                    || i == FIK_PAGE_UP        || i == FIK_CTL_PAGE_UP
                    || i == FIK_PAGE_DOWN      || i == FIK_CTL_PAGE_DOWN)
            {
                switch (i)
                {
                case FIK_DOWN_ARROW:
                case FIK_CTL_DOWN_ARROW: // down one line
                    if (in_scrolling_mode && top_line < lines_in_entry - 17)
                    {
                        top_line++;
                        rewrite_infbuf = true;
                    }
                    break;
                case FIK_UP_ARROW:
                case FIK_CTL_UP_ARROW:  // up one line
                    if (in_scrolling_mode && top_line > 0)
                    {
                        top_line--;
                        rewrite_infbuf = true;
                    }
                    break;
                case FIK_LEFT_ARROW:
                case FIK_CTL_LEFT_ARROW:  // left one column
                    if (in_scrolling_mode && left_column > 0)
                    {
                        left_column--;
                        rewrite_infbuf = true;
                    }
                    break;
                case FIK_RIGHT_ARROW:
                case FIK_CTL_RIGHT_ARROW: // right one column
                    if (in_scrolling_mode && strchr(infbuf, '\021') != nullptr)
                    {
                        left_column++;
                        rewrite_infbuf = true;
                    }
                    break;
                case FIK_PAGE_DOWN:
                case FIK_CTL_PAGE_DOWN: // down 17 lines
                    if (in_scrolling_mode && top_line < lines_in_entry - 17)
                    {
                        top_line += 17;
                        if (top_line > lines_in_entry - 17)
                            top_line = lines_in_entry - 17;
                        rewrite_infbuf = true;
                    }
                    break;
                case FIK_PAGE_UP:
                case FIK_CTL_PAGE_UP: // up 17 lines
                    if (in_scrolling_mode && top_line > 0)
                    {
                        top_line -= 17;
                        if (top_line < 0)
                            top_line = 0;
                        rewrite_infbuf = true;
                    }
                    break;
                case FIK_END:
                case FIK_CTL_END:       // to end of entry
                    if (in_scrolling_mode)
                    {
                        top_line = lines_in_entry - 17;
                        left_column = 0;
                        rewrite_infbuf = true;
                    }
                    break;
                case FIK_HOME:
                case FIK_CTL_HOME:     // to beginning of entry
                    if (in_scrolling_mode)
                    {
                        left_column = 0;
                        top_line = left_column;
                        rewrite_infbuf = true;
                    }
                    break;
                default:
                    break;
                }
            }
            else
                done = true;  // a key other than scrolling key was pressed
        }
        g_text_cbase = 0;
        driver_hide_text_cursor();
        driver_unstack_screen();
    }
    return 0;
}

static void load_entry_text(
    FILE *entfile,
    char *buf,
    int maxlines,
    int startrow,
    int startcol)
{
    int linelen;
    bool comment = false;
    int c = 0;
    int tabpos = 7 - (startcol % 8);

    if (maxlines <= 0)
    { // no lines to get!
        *buf = (char) 0;
        return;
    }

    //move down to starting row
    for (int i = 0; i < startrow; i++)
    {
        while ((c = fgetc(entfile)) != '\n' && c != EOF && c != '\032')
        {
            if (c == ';')
                comment = true;
            if (c == '}' && !comment)  // end of entry before start line
                break;                 // this should never happen
        }
        if (c == '\n')
            comment = false;
        else
        {                       // reached end of file or end of entry
            *buf = (char) 0;
            return;
        }
    }

    // write maxlines of entry
    while (maxlines-- > 0)
    {
        comment = false;
        c = 0;
        linelen = c;

        // skip line up to startcol
        int i = 0;
        while (i++ < startcol && (c = fgetc(entfile)) != EOF && c != '\032')
        {
            if (c == ';')
                comment = true;
            if (c == '}' && !comment)
            { //reached end of entry
                *buf = (char) 0;
                return;
            }
            if (c == '\r')
            {
                i--;
                continue;
            }
            if (c == '\t')
                i += 7 - (i % 8);
            if (c == '\n')
            {  //need to insert '\n', even for short lines
                *(buf++) = (char)c;
                break;
            }
        }
        if (c == EOF || c == '\032')
        { // unexpected end of file
            *buf = (char) 0;
            return;
        }
        if (c == '\n')       // line is already completed
            continue;

        if (i > startcol)
        {  // can happen because of <tab> character
            while (i-- > startcol)
            {
                *(buf++) = ' ';
                linelen++;
            }
        }

        //process rest of line into buf
        while ((c = fgetc(entfile)) != EOF && c != '\032')
        {
            if (c == ';')
                comment = true;
            else if (c == '\n' || c == '\r')
                comment = false;
            if (c != '\r')
            {
                if (c == '\t')
                {
                    while ((linelen % 8) != tabpos && linelen < 75)
                    { // 76 wide max
                        *(buf++) = ' ';
                        ++linelen;
                    }
                    c = ' ';
                }
                if (c == '\n')
                {
                    *(buf++) = '\n';
                    break;
                }
                if (++linelen > 75)
                {
                    if (linelen == 76)
                        *(buf++) = '\021';
                }
                else
                    *(buf++) = (char)c;
                if (c == '}' && !comment)
                { //reached end of entry
                    *(buf) = (char) 0;
                    return;
                }
            }
        }
        if (c == EOF || c == '\032')
        { // unexpected end of file
            *buf = (char) 0;
            return;
        }
    }
    if (*(buf-1) == '\n') // specified that buf will not end with a '\n'
        buf--;
    *buf = (char) 0;
}

static void format_parmfile_line(int choice, char *buf)
{
    int c, i;
    char line[80];
    fseek(gfe_file, gfe_choices[choice]->point, SEEK_SET);
    while (getc(gfe_file) != '{')
    {
    }
    do
    {
        c = getc(gfe_file);
    }
    while (c == ' ' || c == '\t' || c == ';');
    i = 0;
    while (i < 56 && c != '\n' && c != '\r' && c != EOF && c != '\032')
    {
        line[i++] = (char)((c == '\t') ? ' ' : c);
        c = getc(gfe_file);
    }
    line[i] = 0;
#ifndef XFRACT
    sprintf(buf, "%-20Fs%-56s", gfe_choices[choice]->name, line);
#else
    sprintf(buf, "%-20s%-56s", gfe_choices[choice]->name, line);
#endif
}

// ---------------------------------------------------------------------

int get_fract3d_params() // prompt for 3D fractal parameters
{
    int i, k, ret, oldhelpmode;
    fullscreenvalues uvalues[20];
    const char *ifs3d_prompts[7] =
    {
        "X-axis rotation in degrees",
        "Y-axis rotation in degrees",
        "Z-axis rotation in degrees",
        "Perspective distance [1 - 999, 0 for no persp]",
        "X shift with perspective (positive = right)",
        "Y shift with perspective (positive = up   )",
        "Stereo (R/B 3D)? (0=no,1=alternate,2=superimpose,3=photo,4=stereo pair)"
    };

    driver_stack_screen();
    k = 0;
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = XROT;
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = YROT;
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = ZROT;
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = ZVIEWER;
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = XSHIFT;
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = YSHIFT;
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = g_glasses_type;

    oldhelpmode = helpmode;
    helpmode = HELP3DFRACT;
    i = fullscreen_prompt("3D Parameters", k, ifs3d_prompts, uvalues, 0, nullptr);
    helpmode = oldhelpmode;
    if (i < 0)
    {
        ret = -1;
        goto get_f3d_exit;
    }

    k = 0;
    ret = k;
    XROT    =  uvalues[k++].uval.ival;
    YROT    =  uvalues[k++].uval.ival;
    ZROT    =  uvalues[k++].uval.ival;
    ZVIEWER =  uvalues[k++].uval.ival;
    XSHIFT  =  uvalues[k++].uval.ival;
    YSHIFT  =  uvalues[k++].uval.ival;
    g_glasses_type = uvalues[k++].uval.ival;
    if (g_glasses_type < 0 || g_glasses_type > 4)
        g_glasses_type = 0;
    if (g_glasses_type)
        if (get_funny_glasses_params() || check_mapfile())
            ret = -1;

get_f3d_exit:
    driver_unstack_screen();
    return ret;
}

// ---------------------------------------------------------------------
// These macros streamline the "save near space" campaign

int get_3d_params()     // prompt for 3D parameters
{
    const char *choices[11];
    int attributes[21];
    int sphere;
    const char *s;
    const char *prompts3d[21];
    fullscreenvalues uvalues[21];
    int k;
    int oldhelpmode;

restart_1:
    if (Targa_Out && overlay3d)
        Targa_Overlay = true;

    k = -1;

    prompts3d[++k] = "Preview Mode?";
    uvalues[k].type = 'y';
    uvalues[k].uval.ch.val = preview ? 1 : 0;

    prompts3d[++k] = "    Show Box?";
    uvalues[k].type = 'y';
    uvalues[k].uval.ch.val = showbox ? 1 : 0;

    prompts3d[++k] = "Coarseness, preview/grid/ray (in y dir)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = previewfactor;

    prompts3d[++k] = "Spherical Projection?";
    uvalues[k].type = 'y';
    sphere = SPHERE;
    uvalues[k].uval.ch.val = sphere;

    prompts3d[++k] = "Stereo (R/B 3D)? (0=no,1=alternate,2=superimpose,";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = g_glasses_type;

    prompts3d[++k] = "                  3=photo,4=stereo pair)";
    uvalues[k].type = '*';

    prompts3d[++k] = "Ray trace out? (0=No, 1=DKB/POVRay, 2=VIVID, 3=RAW,";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = RAY;

    prompts3d[++k] = "                4=MTV, 5=RAYSHADE, 6=ACROSPIN, 7=DXF)";
    uvalues[k].type = '*';

    prompts3d[++k] = "    Brief output?";
    uvalues[k].type = 'y';
    uvalues[k].uval.ch.val = BRIEF ? 1 : 0;

    check_writefile(ray_name, ".ray");
    prompts3d[++k] = "    Output File Name";
    uvalues[k].type = 's';
    strcpy(uvalues[k].uval.sval, ray_name);

    prompts3d[++k] = "Targa output?";
    uvalues[k].type = 'y';
    uvalues[k].uval.ch.val = Targa_Out ? 1 : 0;

    prompts3d[++k] = "Use grayscale value for depth? (if \"no\" uses color number)";
    uvalues[k].type = 'y';
    uvalues[k].uval.ch.val = grayflag ? 1 : 0;

    oldhelpmode = helpmode;
    helpmode = HELP3DMODE;

    k = fullscreen_prompt("3D Mode Selection", k+1, prompts3d, uvalues, 0, nullptr);
    helpmode = oldhelpmode;
    if (k < 0)
    {
        return -1;
    }

    k = 0;
    preview = uvalues[k++].uval.ch.val != 0;
    showbox = uvalues[k++].uval.ch.val != 0;
    previewfactor  = uvalues[k++].uval.ival;
    sphere = uvalues[k++].uval.ch.val;
    g_glasses_type = uvalues[k++].uval.ival;
    k++;
    RAY = uvalues[k++].uval.ival;
    k++;
    {
        if (RAY == 1)
            stopmsg(STOPMSG_NONE,
                "DKB/POV-Ray output is obsolete but still works. See \"Ray Tracing Output\" in\n"
                "the online documentation.");
    }
    BRIEF = uvalues[k++].uval.ch.val != 0;

    strcpy(ray_name, uvalues[k++].uval.sval);

    Targa_Out = uvalues[k++].uval.ch.val != 0;
    grayflag  = uvalues[k++].uval.ch.val != 0;

    // check ranges
    if (previewfactor < 2)
        previewfactor = 2;
    if (previewfactor > 2000)
        previewfactor = 2000;

    if (sphere && !SPHERE)
    {
        SPHERE = TRUE;
        set_3d_defaults();
    }
    else if (!sphere && SPHERE)
    {
        SPHERE = FALSE;
        set_3d_defaults();
    }

    if (g_glasses_type < 0)
        g_glasses_type = 0;
    if (g_glasses_type > 4)
        g_glasses_type = 4;
    if (g_glasses_type)
        g_which_image = 1;

    if (RAY < 0)
        RAY = 0;
    if (RAY > 7)
        RAY = 7;

    if (!RAY)
    {
        k = 0;
        choices[k++] = "make a surface grid";
        choices[k++] = "just draw the points";
        choices[k++] = "connect the dots (wire frame)";
        choices[k++] = "surface fill (colors interpolated)";
        choices[k++] = "surface fill (colors not interpolated)";
        choices[k++] = "solid fill (bars up from \"ground\")";
        if (SPHERE)
        {
            choices[k++] = "light source";
        }
        else
        {
            choices[k++] = "light source before transformation";
            choices[k++] = "light source after transformation";
        }
        for (int i = 0; i < k; ++i)
            attributes[i] = 1;
        helpmode = HELP3DFILL;
        int i = fullscreen_choice(CHOICE_HELP, "Select 3D Fill Type",
                nullptr, nullptr, k, (const char **)choices, attributes,
                0, 0, 0, FILLTYPE+1, nullptr, nullptr, nullptr, nullptr);
        helpmode = oldhelpmode;
        if (i < 0)
            goto restart_1;
        FILLTYPE = i-1;

        if (g_glasses_type)
        {
            if (get_funny_glasses_params())
                goto restart_1;
        }
        if (check_mapfile())
            goto restart_1;
    }
restart_3:

    if (SPHERE)
    {
        k = -1;
        prompts3d[++k] = "Longitude start (degrees)";
        prompts3d[++k] = "Longitude stop  (degrees)";
        prompts3d[++k] = "Latitude start  (degrees)";
        prompts3d[++k] = "Latitude stop   (degrees)";
        prompts3d[++k] = "Radius scaling factor in pct";
    }
    else
    {
        k = -1;
        if (!RAY)
        {
            prompts3d[++k] = "X-axis rotation in degrees";
            prompts3d[++k] = "Y-axis rotation in degrees";
            prompts3d[++k] = "Z-axis rotation in degrees";
        }
        prompts3d[++k] = "X-axis scaling factor in pct";
        prompts3d[++k] = "Y-axis scaling factor in pct";
    }
    k = -1;
    if (!(RAY && !SPHERE))
    {
        uvalues[++k].uval.ival   = XROT    ;
        uvalues[k].type = 'i';
        uvalues[++k].uval.ival   = YROT    ;
        uvalues[k].type = 'i';
        uvalues[++k].uval.ival   = ZROT    ;
        uvalues[k].type = 'i';
    }
    uvalues[++k].uval.ival   = XSCALE    ;
    uvalues[k].type = 'i';

    uvalues[++k].uval.ival   = YSCALE    ;
    uvalues[k].type = 'i';

    prompts3d[++k] = "Surface Roughness scaling factor in pct";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = ROUGH     ;

    prompts3d[++k] = "'Water Level' (minimum color value)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = WATERLINE ;

    if (!RAY)
    {
        prompts3d[++k] = "Perspective distance [1 - 999, 0 for no persp])";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = ZVIEWER     ;

        prompts3d[++k] = "X shift with perspective (positive = right)";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = XSHIFT    ;

        prompts3d[++k] = "Y shift with perspective (positive = up   )";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = YSHIFT    ;

        prompts3d[++k] = "Image non-perspective X adjust (positive = right)";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = xtrans    ;

        prompts3d[++k] = "Image non-perspective Y adjust (positive = up)";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = ytrans    ;

        prompts3d[++k] = "First transparent color";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = transparent[0];

        prompts3d[++k] = "Last transparent color";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = transparent[1];
    }

    prompts3d[++k] = "Randomize Colors      (0 - 7, '0' disables)";
    uvalues[k].type = 'i';
    uvalues[k++].uval.ival = RANDOMIZE;

    if (SPHERE)
        s = "Sphere 3D Parameters\n"
            "Sphere is on its side; North pole to right\n"
            "Long. 180 is top, 0 is bottom; Lat. -90 is left, 90 is right";
    else
        s = "Planar 3D Parameters\n"
            "Pre-rotation X axis is screen top; Y axis is left side\n"
            "Pre-rotation Z axis is coming at you out of the screen!";

    helpmode = HELP3DPARMS;
    k = fullscreen_prompt(s, k, prompts3d, uvalues, 0, nullptr);
    helpmode = oldhelpmode;
    if (k < 0)
        goto restart_1;

    k = 0;
    if (!(RAY && !SPHERE))
    {
        XROT    = uvalues[k++].uval.ival;
        YROT    = uvalues[k++].uval.ival;
        ZROT    = uvalues[k++].uval.ival;
    }
    XSCALE     = uvalues[k++].uval.ival;
    YSCALE     = uvalues[k++].uval.ival;
    ROUGH      = uvalues[k++].uval.ival;
    WATERLINE  = uvalues[k++].uval.ival;
    if (!RAY)
    {
        ZVIEWER = uvalues[k++].uval.ival;
        XSHIFT     = uvalues[k++].uval.ival;
        YSHIFT     = uvalues[k++].uval.ival;
        xtrans     = uvalues[k++].uval.ival;
        ytrans     = uvalues[k++].uval.ival;
        transparent[0] = uvalues[k++].uval.ival;
        transparent[1] = uvalues[k++].uval.ival;
    }
    RANDOMIZE  = uvalues[k++].uval.ival;
    if (RANDOMIZE >= 7)
        RANDOMIZE = 7;
    if (RANDOMIZE <= 0)
        RANDOMIZE = 0;

    if (Targa_Out || ILLUMINE || RAY)
        if (get_light_params())
            goto restart_3;
    return 0;
}

// ---------------------------------------------------------------------
static bool get_light_params()
{
    const char *prompts3d[13];
    fullscreenvalues uvalues[13];

    int k;
    int oldhelpmode;

    // defaults go here

    k = -1;

    if (ILLUMINE || RAY)
    {
        prompts3d[++k] = "X value light vector";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = XLIGHT    ;

        prompts3d[++k] = "Y value light vector";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = YLIGHT    ;

        prompts3d[++k] = "Z value light vector";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = ZLIGHT    ;

        if (!RAY)
        {
            prompts3d[++k] = "Light Source Smoothing Factor";
            uvalues[k].type = 'i';
            uvalues[k].uval.ival = LIGHTAVG  ;

            prompts3d[++k] = "Ambient";
            uvalues[k].type = 'i';
            uvalues[k].uval.ival = Ambient;
        }
    }

    if (Targa_Out && !RAY)
    {
        prompts3d[++k] = "Haze Factor        (0 - 100, '0' disables)";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = haze;

        if (!Targa_Overlay)
            check_writefile(light_name, ".tga");
        prompts3d[++k] = "Targa File Name  (Assume .tga)";
        uvalues[k].type = 's';
        strcpy(uvalues[k].uval.sval, light_name);

        prompts3d[++k] = "Back Ground Color (0 - 255)";
        uvalues[k].type = '*';

        prompts3d[++k] = "   Red";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = (int)back_color[0];

        prompts3d[++k] = "   Green";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = (int)back_color[1];

        prompts3d[++k] = "   Blue";
        uvalues[k].type = 'i';
        uvalues[k].uval.ival = (int)back_color[2];

        prompts3d[++k] = "Overlay Targa File? (Y/N)";
        uvalues[k].type = 'y';
        uvalues[k].uval.ch.val = Targa_Overlay ? 1 : 0;

    }

    prompts3d[++k] = "";

    oldhelpmode = helpmode;
    helpmode = HELP3DLIGHT;
    k = fullscreen_prompt("Light Source Parameters", k, prompts3d, uvalues, 0, nullptr);
    helpmode = oldhelpmode;
    if (k < 0)
        return true;

    k = 0;
    if (ILLUMINE)
    {
        XLIGHT   = uvalues[k++].uval.ival;
        YLIGHT   = uvalues[k++].uval.ival;
        ZLIGHT   = uvalues[k++].uval.ival;
        if (!RAY)
        {
            LIGHTAVG = uvalues[k++].uval.ival;
            Ambient  = uvalues[k++].uval.ival;
            if (Ambient >= 100)
                Ambient = 100;
            if (Ambient <= 0)
                Ambient = 0;
        }
    }

    if (Targa_Out && !RAY)
    {
        haze  =  uvalues[k++].uval.ival;
        if (haze >= 100)
            haze = 100;
        if (haze <= 0)
            haze = 0;
        strcpy(light_name, uvalues[k++].uval.sval);
        /* In case light_name conflicts with an existing name it is checked
                again in line3d */
        k++;
        back_color[0] = (char)(uvalues[k++].uval.ival % 255);
        back_color[1] = (char)(uvalues[k++].uval.ival % 255);
        back_color[2] = (char)(uvalues[k++].uval.ival % 255);
        Targa_Overlay = uvalues[k].uval.ch.val != 0;
    }
    return false;
}

// ---------------------------------------------------------------------


static bool check_mapfile()
{
    bool askflag = false;
    int i, oldhelpmode;
    if (dontreadcolor)
        return false;
    strcpy(temp1, "*");
    if (mapset)
        strcpy(temp1, MAP_name);
    if (!(g_glasses_type == 1 || g_glasses_type == 2))
        askflag = true;
    else
        merge_pathnames(temp1, funnyglasses_map_name, cmd_file::AT_CMD_LINE);

    while (1)
    {
        if (askflag)
        {
            oldhelpmode = helpmode;
            helpmode = -1;
            i = field_prompt("Enter name of .MAP file to use,\n"
                             "or '*' to use palette from the image to be loaded.",
                             nullptr, temp1, 60, nullptr);
            helpmode = oldhelpmode;
            if (i < 0)
                return true;
            if (temp1[0] == '*')
            {
                mapset = false;
                break;
            }
        }
        memcpy(olddacbox, g_dac_box, 256*3); // save the DAC
        bool valid = ValidateLuts(temp1);
        memcpy(g_dac_box, olddacbox, 256*3); // restore the DAC
        if (valid) // Oops, somethings wrong
        {
            askflag = true;
            continue;
        }
        mapset = true;
        merge_pathnames(MAP_name, temp1, cmd_file::AT_CMD_LINE);
        break;
    }
    return false;
}

static bool get_funny_glasses_params()
{
    const char *prompts3d[10];

    fullscreenvalues uvalues[10];

    int k;
    int oldhelpmode;

    // defaults
    if (ZVIEWER == 0)
        ZVIEWER = 150;
    if (g_eye_separation == 0)
    {
        if (fractype == fractal_type::IFS3D || fractype == fractal_type::LLORENZ3D || fractype == fractal_type::FPLORENZ3D)
        {
            g_eye_separation =  2;
            xadjust       = -2;
        }
        else
        {
            g_eye_separation =  3;
            xadjust       =  0;
        }
    }

    if (g_glasses_type == 1)
        strcpy(funnyglasses_map_name, Glasses1Map);
    else if (g_glasses_type == 2)
    {
        if (FILLTYPE == -1)
            strcpy(funnyglasses_map_name, "grid.map");
        else
        {
            strcpy(funnyglasses_map_name, Glasses1Map);
            funnyglasses_map_name[7] = '2';
        }
    }

    k = -1;
    prompts3d[++k] = "Interocular distance (as % of screen)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = g_eye_separation;

    prompts3d[++k] = "Convergence adjust (positive = spread greater)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = xadjust;

    prompts3d[++k] = "Left  red image crop (% of screen)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = red_crop_left;

    prompts3d[++k] = "Right red image crop (% of screen)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = red_crop_right;

    prompts3d[++k] = "Left  blue image crop (% of screen)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = blue_crop_left;

    prompts3d[++k] = "Right blue image crop (% of screen)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = blue_crop_right;

    prompts3d[++k] = "Red brightness factor (%)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = red_bright;

    prompts3d[++k] = "Blue brightness factor (%)";
    uvalues[k].type = 'i';
    uvalues[k].uval.ival = blue_bright;

    if (g_glasses_type == 1 || g_glasses_type == 2)
    {
        prompts3d[++k] = "Map File name";
        uvalues[k].type = 's';
        strcpy(uvalues[k].uval.sval, funnyglasses_map_name);
    }

    oldhelpmode = helpmode;
    helpmode = HELP3DGLASSES;
    k = fullscreen_prompt("Funny Glasses Parameters", k+1, prompts3d, uvalues, 0, nullptr);
    helpmode = oldhelpmode;
    if (k < 0)
        return true;

    k = 0;
    g_eye_separation   =  uvalues[k++].uval.ival;
    xadjust         =  uvalues[k++].uval.ival;
    red_crop_left   =  uvalues[k++].uval.ival;
    red_crop_right  =  uvalues[k++].uval.ival;
    blue_crop_left  =  uvalues[k++].uval.ival;
    blue_crop_right =  uvalues[k++].uval.ival;
    red_bright      =  uvalues[k++].uval.ival;
    blue_bright     =  uvalues[k++].uval.ival;

    if (g_glasses_type == 1 || g_glasses_type == 2)
        strcpy(funnyglasses_map_name, uvalues[k].uval.sval);
    return false;
}

void setbailoutformula(bailouts test)
{
    switch (test)
    {
    case bailouts::Mod:
    default:
        if (debugflag != debug_flags::prevent_287_math)
            floatbailout = asmfpMODbailout;
        else
            floatbailout = fpMODbailout;
        if (debugflag != debug_flags::prevent_386_math)
            longbailout = asm386lMODbailout;
        else
            longbailout = asmlMODbailout;
        bignumbailout = bnMODbailout;
        bigfltbailout = bfMODbailout;
        break;

    case bailouts::Real:
        if (debugflag != debug_flags::prevent_287_math)
            floatbailout = asmfpREALbailout;
        else
            floatbailout = fpREALbailout;
        if (debugflag != debug_flags::prevent_386_math)
            longbailout = asm386lREALbailout;
        else
            longbailout = asmlREALbailout;
        bignumbailout = bnREALbailout;
        bigfltbailout = bfREALbailout;
        break;

    case bailouts::Imag:
        if (debugflag != debug_flags::prevent_287_math)
            floatbailout = asmfpIMAGbailout;
        else
            floatbailout = fpIMAGbailout;
        if (debugflag != debug_flags::prevent_386_math)
            longbailout = asm386lIMAGbailout;
        else
            longbailout = asmlIMAGbailout;
        bignumbailout = bnIMAGbailout;
        bigfltbailout = bfIMAGbailout;
        break;

    case bailouts::Or:
        if (debugflag != debug_flags::prevent_287_math)
            floatbailout = asmfpORbailout;
        else
            floatbailout = fpORbailout;
        if (debugflag != debug_flags::prevent_386_math)
            longbailout = asm386lORbailout;
        else
            longbailout = asmlORbailout;
        bignumbailout = bnORbailout;
        bigfltbailout = bfORbailout;
        break;

    case bailouts::And:
        if (debugflag != debug_flags::prevent_287_math)
            floatbailout = asmfpANDbailout;
        else
            floatbailout = fpANDbailout;
        if (debugflag != debug_flags::prevent_386_math)
            longbailout = asm386lANDbailout;
        else
            longbailout = asmlANDbailout;
        bignumbailout = bnANDbailout;
        bigfltbailout = bfANDbailout;
        break;

    case bailouts::Manh:
        if (debugflag != debug_flags::prevent_287_math)
            floatbailout = asmfpMANHbailout;
        else
            floatbailout = fpMANHbailout;
        if (debugflag != debug_flags::prevent_386_math)
            longbailout = asm386lMANHbailout;
        else
            longbailout = asmlMANHbailout;
        bignumbailout = bnMANHbailout;
        bigfltbailout = bfMANHbailout;
        break;

    case bailouts::Manr:
        if (debugflag != debug_flags::prevent_287_math)
            floatbailout = asmfpMANRbailout;
        else
            floatbailout = fpMANRbailout;
        if (debugflag != debug_flags::prevent_386_math)
            longbailout = asm386lMANRbailout;
        else
            longbailout = asmlMANRbailout;
        bignumbailout = bnMANRbailout;
        bigfltbailout = bfMANRbailout;
        break;
    }
}
