#include <boost/filesystem/operations.hpp>

#include <palette_loader.hxx>
#include <global_state.hxx>

auto asset_loader<palette>::load_asset(const ::std::string& p_name) const
	-> palette
{
	// Retrieve data path
	const auto t_dataPath = global_state().path_manager().data_path();
	
	// Build path to requested palette file
	const auto t_palPath = (t_dataPath / "palettes" / (p_name + ".json"));
	
	// Check if that file exists and is, in fact, a file
	if(boost::filesystem::exists(t_palPath) &&
		boost::filesystem::is_regular_file(t_palPath))
	{
		return palette{ t_palPath.string() };
	}
	else
		throw ::std::runtime_error("asset_loader<palette>: File not found");
}
