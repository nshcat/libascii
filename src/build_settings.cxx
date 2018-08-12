#include <ut/platform.hxx>
#include <build_settings.hxx>

auto build_settings::data_path_prefix()
	-> path_type
{
#ifdef LIBUT_IS_WINDOWS
	return { "ROGUELIKE_DATA_PATH" };
#else
	return { ROGUELIKE_DATA_PATH };
#endif
}

auto build_settings::use_home()
	-> bool
{
	return ROGUELIKE_USE_HOME;
}
