#pragma once

#include "node_base.hxx"


namespace utility::pnfa::internal
{
	// Abstract class representing any kind of node in the automaton.
	template< typename Tinput, typename... Tstate >
	class node
		: public node_base
	{
		using this_type = node<Tinput, Tstate...>;
	
		protected:
			using node_id = typename node_base::node_id;
			
		public:
			node(node_id p_id)
				: node_base(node_type::node, p_id)
			{
			}
			
		public:
			virtual auto clone() const
				-> ::std::unique_ptr<node_base>
			{
				return ::std::make_unique<this_type>(this->id());
			}
	};
}
