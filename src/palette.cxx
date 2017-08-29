#include <stdexcept>
#include <algorithm>
#include <string_view>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <ut/throwf.hxx>
#include <ut/utility.hxx>
#include <ut/integral.hxx>

#include <palette.hxx>

using namespace ut::literals;
namespace pt = boost::property_tree;

static const ::std::array<::std::string_view, palette::num_colors> g_ColorLabels{
	"black",
	"light_gray",
	"red",
	"green",
	"yellow",
	"blue",
	"magenta",
	"cyan",
	"dark_gray",
	"white",
	"light_red",
	"light_green",
	"light_yellow",
	"light_blue",
	"light_magenta",
	"light_cyan"
};

palette::palette(ut::array_view<color_type> p_view)
{
	if(p_view.size() != m_ColorData.size())
		throw ::std::runtime_error("palette: tried to initialize with color array of non-matching length");
		
	::std::copy(p_view.begin(), p_view.end(), m_ColorData.begin());
}

palette::palette(const ::std::string& p_path)
{
	try
	{
		// Read palette configuration file
		pt::ptree t_tree;
		
		pt::read_json(p_path, t_tree);
		
		// Retrieve all colors
		for(::std::size_t t_ix = 0; t_ix < num_colors; ++t_ix)
		{
			const auto& t_entry = t_tree.get_child(g_ColorLabels[t_ix].data());
			
			color_type t_color{ };
			t_color.r = ::std::clamp(t_entry.get<::std::uint32_t>("r"), 0_u32, 255_u32);
			t_color.g = ::std::clamp(t_entry.get<::std::uint32_t>("g"), 0_u32, 255_u32);
			t_color.b = ::std::clamp(t_entry.get<::std::uint32_t>("b"), 0_u32, 255_u32);
			
			m_ColorData[t_ix] = t_color;
		}
	}
	catch(const ::std::exception& p_ex)
	{
		ut::throwf<::std::runtime_error>("palette: failed to load palette file \"%s\": %s",
			p_path,
			p_ex.what()
		);
	}
}

auto palette::lookup(color p_clr) const
	-> const color_type&
{
	const auto t_index = ut::enum_cast(p_clr);
	
	if(t_index >= num_colors)
		throw ::std::runtime_error("palette::lookup: invalid color");

	return m_ColorData[t_index];
}
