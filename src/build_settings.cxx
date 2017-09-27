#include <build_settings.hxx>

auto build_settings::data_path()
	-> path_type
{
	return { ROGUELIKE_DATA_PATH };
}

auto build_settings::use_home()
	-> bool
{
	return ROGUELIKE_USE_HOME;
}
