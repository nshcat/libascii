#pragma once

#include <stdexcept>

namespace utility::pnfa::internal
{
	// An enumeration used to dynamically identify the actual, concrete
	// type of a node_base instance.
	// Since automaton<...> knows its template parameters, it can then
	// cast the base class to the corresponding derived class, since
	// the node instances used will always have the same template parameters
	// as the parent automaton that uses them.
	enum class node_type
	{
		node,
		sub_automaton
	};
	
	// Abstract class representing any kind of node in the automaton.
	class node_base
	{
		protected:
			using node_id = ::std::size_t;
	
		public:
			node_base(node_type p_type, node_id p_id)
				: m_Type{p_type}, m_Id{p_id}
			{
			}
	
			virtual ~node_base() = 0;
	
			node_base(const node_base&) = delete;
			node_base& operator=(const node_base&) = delete;
			
		public:
			virtual auto clone() const
				-> ::std::unique_ptr<node_base> = 0;
	
		public:
			auto type() const
				-> node_type
			{
				return m_Type;
			}
			
			auto id() const
				-> const node_id&
			{
				return m_Id;
			}
			
			auto is_accepting() const
				-> bool
			{
				return m_IsAccepting;
			}
			
		public:
			auto set_accepting(bool p_val)
				-> void
			{
				if(type() == node_type::sub_automaton)
					throw ::std::runtime_error("node_base::set_accepting: Sub automaton cannot be marked as accepting");
				
				m_IsAccepting = p_val;
			}
	
		protected:
			bool m_IsAccepting{false};
			node_type m_Type;
			node_id m_Id;
	};
	
	node_base::~node_base()
	{
	}
}
