#include "stdafx.h"

#include "filesystem.h"

bool g_check_current_dir;
bool g_fractal_overwrite;

int fr_find_first(char *path)
{
	return 0;
}

int expand_dirname(char *, char *)
{
	return 0;
}

SimpleString StringFrom(const fs::path &path)
{
	return SimpleString(path.string().c_str());
}

TEST(filesystem, ensure_extension1)
{
	fs::path p = "foo";
	ensure_extension(p, ".gif");
	CHECK_EQUAL("foo.gif", p);
}

TEST(filesystem, ensure_extension2)
{
	fs::path p = "foo.gif";
	ensure_extension(p, ".gif");
	CHECK_EQUAL("foo.gif", p);
}

TEST(filesystem, ensure_extension3)
{
	fs::path p = "foo.gif";
	ensure_extension(p, ".map");
	CHECK_EQUAL("foo.gif", p);
}

TEST(filesystem, ensure_extension4)
{
	char filename[80] = "foo";
	ensure_extension(filename, ".gif");
	CHECK_EQUAL(std::string("foo.gif"), std::string(filename));
}

TEST(filesystem, ensure_extension5)
{
	char filename[80] = "foo.gif";
	ensure_extension(filename, ".gif");
	CHECK_EQUAL(std::string("foo.gif"), std::string(filename));
}

TEST(filesystem, ensure_extension6)
{
	char filename[80] = "foo.gif";
	ensure_extension(filename, ".map");
	CHECK_EQUAL(std::string("foo.gif"), std::string(filename));
}
