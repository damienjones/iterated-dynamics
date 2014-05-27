/*
 * intro.cpp
 *
 * Iterated Dynamics intro screen (authors & credits)
 *
 *
 */
#include <cassert>
#include <vector>

#include "port.h"
#include "prototyp.h"
#include "helpdefs.h"

#include "AbstractInput.h"
#include "drivers.h"
#include "idhelp.h"
#include "intro.h"
#include "realdos.h"
#include "TextColors.h"

#ifdef XFRACT
extern int slowdisplay;
#endif

class IteratedDynamicsDialog : public AbstractDialog
{
public:
	IteratedDynamicsDialog(AbstractDriver *driver) : AbstractDialog(driver), m_contexts()
	{
	}

	virtual ~IteratedDynamicsDialog()
	{
		assert(m_contexts.size() == 0);
	}

	void PushContext(AbstractInputContext *context)	{ m_contexts.push_back(context); }
	void PopContext()								{ m_contexts.pop_back(); }

	virtual bool ProcessWaitingKey(int key);
	virtual bool ProcessIdle();

private:
	std::vector<AbstractInputContext *> m_contexts;
};

bool IteratedDynamicsDialog::ProcessWaitingKey(int key)
{
	return (m_contexts.size() > 0) ? m_contexts.back()->ProcessWaitingKey(key) : true;
}

bool IteratedDynamicsDialog::ProcessIdle()
{
	return (m_contexts.size() > 0) ? m_contexts.back()->ProcessIdle() : true;
}

class Introduction : public IteratedDynamicsDialog
{
public:
	Introduction(AbstractDriver *driver);
	~Introduction();

	void Show();

private:
	void GetCredits();
	virtual bool ProcessWaitingKey(int key);
	virtual bool ProcessIdle();
	void PaintScreen();

	int m_authors[100];
	char m_credits[32768];
	char m_screen_text[32768];
	bool m_paused;
	int m_top_row;
	int m_bottom_row;
	int m_idle_count;
	int m_current_author;
	int m_delay_max;
	int m_num_authors;
};

Introduction::Introduction(AbstractDriver *driver)
	: IteratedDynamicsDialog(driver),
	m_paused(false),
	m_top_row(0),
	m_bottom_row(0),
	m_idle_count(0),
	m_current_author(0),
	m_delay_max(0),
	m_num_authors(0)
{
	for (int i = 0; i < NUM_OF(m_authors); i++)
	{
		m_authors[i] = 0;
	}
	std::fill(m_credits, m_credits + NUM_OF(m_credits), 0);
	std::fill(m_screen_text, m_screen_text + NUM_OF(m_screen_text), 0);
	PushContext(this);
}

Introduction::~Introduction()
{
	PopContext();
}

void Introduction::GetCredits()
{
	int i = 32767 + read_help_topic(INTRO_AUTHORS, 0, 32767, m_screen_text);
	m_screen_text[i++] = '\0';
	i = 32767 + read_help_topic(INTRO_CREDITS, 0, 32767, m_credits);
	m_credits[i++] = '\0';

	m_num_authors = 0;
	m_authors[m_num_authors] = 0;              // find the start of each credit-line
	for (i = 0; m_credits[i] != 0; i++)
	{
		if (m_credits[i] == 10)
		{
			m_authors[++m_num_authors] = i + 1;
		}
	}
	m_authors[m_num_authors + 1] = i;
}

void Introduction::PaintScreen()
{
	help_title();
	int const END_MAIN_AUTHOR = 4;
	m_top_row = END_MAIN_AUTHOR + 1;
	m_bottom_row = 21;
	put_string_center(1, 0, 80, C_TITLE, "Press ENTER for main menu, F1 for help.");
	driver_put_string(2, 0, C_CONTRIB, m_screen_text);
	driver_set_attr(2, 0, C_AUTHDIV1, 80);
	driver_set_attr(END_MAIN_AUTHOR, 0, C_AUTHDIV1, 80);
	driver_set_attr(22, 0, C_AUTHDIV2, 80);
	driver_set_attr(3, 0, C_PRIMARY, 80*(END_MAIN_AUTHOR-3));
	driver_set_attr(23, 0, C_TITLE_LOW, 160);
	for (int i = 3; i < END_MAIN_AUTHOR; ++i)
	{
		driver_set_attr(i, 21, C_CONTRIB, 58);
	}
	driver_set_attr(m_top_row, 0, C_CONTRIB, (21-END_MAIN_AUTHOR)*80);
	m_current_author = m_bottom_row - m_top_row;
	srand((unsigned int) clock_ticks());
	int j = rand() % (m_num_authors - m_current_author); // first to use
	m_current_author += j; // last to use

	char oldchar = m_credits[m_authors[m_current_author + 1]];
	m_credits[m_authors[m_current_author + 1]] = 0;
	driver_put_string(m_top_row, 0, C_CONTRIB, &m_credits[m_authors[j]]);
	m_credits[m_authors[m_current_author + 1]] = oldchar;

	driver_hide_text_cursor();

	HelpModeSaver saved_help(IDHELP_MENU);
}

void Introduction::Show()
{
	g_timer_start -= clock_ticks();                // "time out" during help
	MouseModeSaver saved_mouse(LOOK_MOUSE_NONE);                     // de-activate full mouse checking

	GetCredits();

	PaintScreen();

	m_idle_count = 0;
	m_delay_max = 10;

	ProcessInput();
}

bool Introduction::ProcessWaitingKey(int key)
{
	// spacebar pauses
	if (IDK_SPACE == key)
	{
		driver_get_key();
		m_paused = !m_paused;
	}
	// any other key and we're done
	return (IDK_SPACE != key);
}

bool Introduction::ProcessIdle()
{
	m_idle_count++;
	if (m_idle_count < m_delay_max)
	{
		driver_delay(100);
		return false;
	}

	m_idle_count = 0;
	m_delay_max = 15;
#ifdef XFRACT
	if (slowdisplay)
	{
		m_delay_max *= 15;
	}
#endif
	driver_scroll_up(m_top_row, m_bottom_row);
	m_current_author++;
	if (m_credits[m_authors[m_current_author]] == 0)
	{
		m_current_author = 0;
	}
	char oldchar = m_credits[m_authors[m_current_author + 1]];
	m_credits[m_authors[m_current_author + 1]] = 0;
	driver_put_string(m_bottom_row, 0, C_CONTRIB, &m_credits[m_authors[m_current_author]]);
	driver_set_attr(m_bottom_row, 0, C_CONTRIB, 80);
	m_credits[m_authors[m_current_author + 1]] = oldchar;
	driver_hide_text_cursor(); // turn it off
	return false;
}

void intro()
{
	Introduction(DriverManager::current()).Show();
}