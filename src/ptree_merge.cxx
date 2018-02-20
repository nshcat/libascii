#include <queue>
#include <string>
#include <algorithm>
#include <application_layer/config/ptree_merge.hxx>


namespace application_layer::config::internal
{
	// Merges given source tree into the given destination tree.
	// Already existing keys are overridden.
	// Returns a reference to the destination tree.
	auto merge(boost::property_tree::ptree& p_dest, const boost::property_tree::ptree& p_src)
		-> boost::property_tree::ptree&
	{
		using ref_type = ::std::reference_wrapper<const boost::property_tree::ptree>;
		using pair_type = ::std::pair<::std::string, ref_type>;
		
		// This Queue contains subtrees that yet need to be visited and their absolute paths
		::std::queue<pair_type> t_trees{ };
		
		// Initial node is the root node of the source tree.
		// Its absolute path is just the empty string.
		t_trees.push(pair_type{ "", ::std::ref(p_src) });
		
		// Keep iterating as long as there are subtrees left
		while(!t_trees.empty())
		{
			// Retrieve current subtree and its path
			auto [t_path, t_subtree] = t_trees.front();
			t_trees.pop();
			
			// Iterate over all children of subtree
			for(const auto& t_child: t_subtree.get())
			{
				// Build absolute path to this particular child.
				// It will be needed in both of the following branches.
				const auto t_childPath = (t_path.empty() ?
						t_child.first.data() :  //< Children of the root node do not receive a leading period.
						t_path + '.' + t_child.first.data());
			
				// Child is a leaf node
				if(t_child.second.size() == 0)
				{
					// Put value into destination tree.
					// `ptree::put` will overwrite existing values.
					p_dest.put(t_childPath, t_child.second.data());
				}
				else // Child is a subtree
				{
					// Push subtree and its path to be visited later
					t_trees.push(pair_type{ t_childPath, ::std::ref(t_child.second) });
				}
			}
		}
		
		return p_dest;
	}
		
	// This overload will copy the destination tree to be able to modify it.
	// A copy of the modified instance is returned.
	auto merge(const boost::property_tree::ptree& p_dest, const boost::property_tree::ptree& p_src)
		-> boost::property_tree::ptree
	{
		// Copy destination tree to obtain mutable instance
		auto t_dest = p_dest;
		
		// Apply merge to it
		merge(t_dest, p_src);
		
		return t_dest;
	}
}
