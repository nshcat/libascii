#pragma once

#include <utility>
#include <algorithm>
#include <vector>
#include <random>
#include <functional>
#include <type_traits>
#include <ut/small_vector.hxx>
#include <ut/observer_ptr.hxx>
#include <ut/cast.hxx>
#include <ut/type_traits.hxx>
#include <ut/small_vector.hxx>

#include <weighted_distribution.hxx>

#include "enum.hxx"
#include "node_base.hxx"
#include "automaton_base.hxx"
#include "node.hxx"
#include "utility.hxx"
#include "edge_base.hxx"
#include "action.hxx"
#include "condition.hxx"

#include "edge.hxx"
#include "probability.hxx"
#include "probabilistic_edge.hxx"


namespace utility::pnfa
{
	template<
		typename 	Tinput,	//< Later: automaton_traits, this class being internal, different public aliases that fill the traits
		typename... Tstate  //< Later: Custom hash function for node ids!
	>
	class automaton // Main automaton class
		: public internal::automaton_base<Tinput, Tstate...>
	{
		// Type aliases that are commonly used in this class
		using input_type = Tinput;		
		using node_view = ut::observer_ptr<internal::node_base>;
		using node_ptr = ::std::unique_ptr<internal::node_base>;
		using edge_ptr = ::std::unique_ptr<internal::edge_base<Tinput, Tstate...>>;
		using node_id = ::std::size_t;	//< Later: Custom type for this!
		using cond_fn = ::std::function<internal::condition_fn_t<Tinput, Tstate...>>;
		using action_fn = ::std::function<internal::action_fn_t<Tinput, Tstate...>>;
		
		// TODO optimization: store node_views in the adjacency_list. This saves unnecessary lookups.
		using node_container = ::std::unordered_map<node_id, node_ptr>;
		using node_edge_pair = ::std::pair<node_id, edge_ptr>;
		using node_edge_pair_view = ut::observer_ptr<node_edge_pair>;
		using adjacency_list = ::std::unordered_map<node_id, ::std::vector<node_edge_pair>>;
			
			
		friend class internal::automaton_base<Tinput, Tstate...>;
			
		public:
			constexpr static node_id no_node = ::std::numeric_limits<node_id>::max();
			
		public:
			automaton()
				: m_RNG{::std::random_device{}()}
			{
				
			}
			
			automaton(automaton&&) = default;
			automaton& operator=(automaton&&) = default;
			
			
			// TODO: copy is not trivial! Deep copy edges and nodes => clone() method in node_base and edge
			// => random cannot be copied, it needs to constructed freshly.
			
		public:
			auto current_state() const
				-> automaton_state
			{
				return m_State;
			}
			
			template<	typename T = node_id,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto current_node() const
				-> T
			{
				return static_cast<T>(m_CurrentNode);
			}
			
		public:
			// Reset the automaton. This will cause the state to change to stopped
			// and the current node set to the starting node
			auto reset()
				-> void
			{
				if(m_StartNode == no_node)
					throw ::std::runtime_error("automaton::reset: no starting node set");
					
				m_State = automaton_state::stopped;
				m_CurrentNode = m_StartNode;
			}
			
		public:
			// Set existing node with given identifier to be a final (aka
			// accepting) node
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto set_accepting(const T& p_id)
				-> void
			{
				const auto t_id = static_cast<node_id>(p_id);
				
				if(m_Nodes.count(t_id) == 0U)
					throw ::std::runtime_error("automaton::set_accepting: node not found");
				else
				{
					m_Nodes[t_id]->set_accepting(true);
				}
			}
			
			
			// Set existing node with given identifier to be the start node of this automaton
			// If there already is a start node defined, it will be overwritten.
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto set_start(const T& p_id)
				-> void
			{	
				const auto t_id = static_cast<node_id>(p_id);
				
				if(m_Nodes.count(t_id) == 0U)
					throw ::std::runtime_error("automaton::set_start: node not found");
				else
					m_StartNode = t_id;
			}
		
		
			// Add a new node with given identifier
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_node(const T& p_id)
				-> void
			{	
				const auto t_id = static_cast<node_id>(p_id);			
				this->add_node(::std::make_unique<internal::node<Tinput, Tstate...>>(t_id));
			}
			
			
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_accepting_node(const T& p_id)
				-> void
			{
				const auto t_id = static_cast<node_id>(p_id);			
				this->add_node(::std::make_unique<internal::node<Tinput, Tstate...>>(t_id));
				this->set_accepting(t_id);
			}
			
			
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_start_node(const T& p_id)
				-> void
			{
				const auto t_id = static_cast<node_id>(p_id);			
				this->add_node(::std::make_unique<internal::node<Tinput, Tstate...>>(t_id));
				this->set_start(t_id);
			}
			
			
			// Add a non-probabilistic edge with given action attached to it
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_edge(const T& p_from, const T& p_to, cond_fn&& p_cond, action_fn&& p_action)
				-> void
			{
				const auto t_from = static_cast<node_id>(p_from);
				const auto t_to = static_cast<node_id>(p_to);
				
				this->add_edge(t_from, t_to, ::std::make_unique<internal::edge<Tinput, Tstate...>>(
					::std::move(p_cond), ::std::move(p_action))
				);
			}
			
			// Add a non-probabilistic edge without an action attached to it
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_edge(const T& p_from, const T& p_to, cond_fn&& p_cond)
				-> void
			{
				this->add_edge(p_from, p_to, ::std::move(p_cond), nothing());
			}		
			
			// Add a non-probabilistic edge without condition and action
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_edge(const T& p_from, const T& p_to)
				-> void
			{
				this->add_edge(p_from, p_to, always_true(), nothing());
			}
			
			// Add a probabilistic edge
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_edge(const T& p_from, const T& p_to, internal::probability p_prob, cond_fn&& p_cond, action_fn&& p_action)
				-> void
			{
				const auto t_from = static_cast<node_id>(p_from);
				const auto t_to = static_cast<node_id>(p_to);
				
				this->add_edge(t_from, t_to, ::std::make_unique<internal::probabilistic_edge<Tinput, Tstate...>>(
					::std::move(p_cond), ::std::move(p_action), p_prob)
				);
			}
			
			// Add a probabilistic edge without an action attached to it
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_edge(const T& p_from, const T& p_to, internal::probability p_prob, cond_fn&& p_cond)
				-> void
			{
				this->add_edge(p_from, p_to, p_prob, ::std::move(p_cond), nothing());
			}
			
			// Add a probabilistic edge without a condition and action attached to it
			template<	typename T,
						typename = ::std::enable_if_t<ut::is_static_castable_v<T, node_id>>
			>
			auto add_edge(const T& p_from, const T& p_to, internal::probability p_prob)
				-> void
			{
				this->add_edge(p_from, p_to, p_prob, always_true(), nothing());
			}		
				
		protected:
			// This method is used as the base of all step() methods.
			// Even though step() does not take any input if the user chose
			// to use `no_input`, but to avoid duplicate code we just expect
			// an empty instance of `no_input` that we do nothing with in that case.
			// Note that the user is responsible to stop the automaton after all symbols
			// have been fed into this method, since it cannot know how long the remaining
			// input is.	
			auto step_impl(const input_type& p_in, Tstate&... p_state)
				-> automaton_result
			{			
				// Check if the automaton is already running
				// TODO: handle in_sub_automaton
				if(m_State == automaton_state::stopped)
				{
					// Start the automaton
					if(m_StartNode == no_node)
						throw ::std::runtime_error("automaton::step: no start node defined");
						
					m_CurrentNode = m_StartNode;
					m_State = automaton_state::running;
				}
		
				// Whether we actually have input values or not
				constexpr bool t_hasInput = not ::std::is_same_v<input_type, no_input>;
				
				// Check that the current node is set to something valid
				if(m_CurrentNode == no_node)
					throw ::std::runtime_error("automaton::step: No current node");
				
				// Retrieve list of edges starting from current node
				auto& t_outgoing = m_Edges.at(m_CurrentNode);	
				
				// Set of transitions that can be taken
				// Note that saving pointers to elements of the data structure is
				// valid here, since we are not modifying it in any way in this
				// method, so no reallocation/rehash is ever triggered here.
				using edge_container = ut::small_vector<node_edge_pair_view, 16U>;
				edge_container t_edges{ };
				
				// Find all edges that can be taken
				for(auto& t_pair: t_outgoing)
				{
					auto& t_edge = t_pair.second;
					
					// Check if the edge is traversable with given input
					if constexpr(t_hasInput)
					{
						if(t_edge->condition()(p_in, p_state...))
							t_edges.push_back({ &t_pair });
					}
					else
					{
						if(t_edge->condition()(p_state...))
							t_edges.push_back({ &t_pair });
					}
				}
				
				// If there are no edges to traverse, the input is rejected.
				if(t_edges.empty())
					return automaton_result::rejected;
					
				// Chosen edge. Will be initialized later.
				node_edge_pair_view t_chosenEdge{ };
				
				// Determine if there are any non-probabilistic edges in the set 
				// of valid transitions. If there are, the automaton will only decide
				// between those. This is the defined action.
				// Only if there are only probabilistic edges they will be chosen based
				// on their respective probabilities.
				// This design decision is based on the notion of non-probabilistic edges
				// having an implicit probability of 1.
				const auto t_filter = [](const auto& t_entry){ return !(t_entry->second->is_probabilistic()); };								
						
				if(::std::any_of(t_edges.begin(), t_edges.end(), t_filter))
				{
					// Filter out all probabilistic edges
					const auto t_begin = ::std::remove_if(t_edges.begin(), t_edges.end(), ::std::not_fn(t_filter));	
					t_edges.erase(t_begin, t_edges.end());
					
					// Pick random number in [0, N-1], where N is the amount of edges left
					// and use it to pick transition
					::std::uniform_int_distribution<::std::size_t> t_distr{0, t_edges.size() - 1U};
					t_chosenEdge = {t_edges.at(t_distr(m_RNG))};			
				}
				else // Select transition based on probabilities
				{
					// We are just saving the index here.
					weighted_distribution<::std::size_t> t_distr{ };
					
					// Retrieve container reference
					auto& t_distrCont = t_distr.container();
					
					// Fill it with entries. We are not using a range-based for loop
					// here since we need the indices of the elements.
					for(::std::size_t t_ix = 0; t_ix < t_edges.size(); ++t_ix)
					{
						auto t_pair = t_edges[t_ix];
						
						if(t_pair->second->is_probabilistic())
						{
							 auto t_probEdge = dynamic_cast<internal::probabilistic_edge<Tinput, Tstate...>*>(
							 	t_pair->second.get()
							 );
						
							t_distrCont.push_back({t_ix, t_probEdge->probability()});
						}
					}
					
					// Pick one entry randomly
					t_chosenEdge = t_edges[t_distr(m_RNG)];				
				}		
				
				// Call action. This needs to be done using static if since the signatures differ
				// based on whether we expect a real input value or not.
				if constexpr(t_hasInput)	
					t_chosenEdge->second->action()(p_in, p_state...);
				else
					t_chosenEdge->second->action()(p_state...);
					
				// Switch current node to chosen node
				m_CurrentNode = t_chosenEdge->first;
				
				// Check if we arrived at an accepting node
				if(m_Nodes.at(m_CurrentNode)->is_accepting())
				{
					return automaton_result::accepted;
				}
				else
				{
					// We are still expecting input
					return automaton_result::running;
				}				
			}
			
		protected:
			// TODO maybe these two methods should throw if they are called when the automaton is not in
			// `stopped` state
			auto add_edge(node_id p_from, node_id p_to, edge_ptr&& p_edge)
				-> void
			{
				if(!m_Nodes.count(p_from) || !m_Nodes.count(p_to))
					throw ::std::runtime_error("automaton::add_edge: unknown node");
			
				m_Edges[p_from].push_back(node_edge_pair{ p_to, ::std::move(p_edge) });
			}
			
			auto add_node(node_ptr&& p_node)
				-> void
			{
				const auto t_id = p_node->id();
				
				if(m_Nodes.count(t_id) == 0U)
				{
					m_Nodes[t_id] = ::std::move(p_node);
				}
				else throw ::std::runtime_error("automaton::add_node: non-unique node id detected");
			}
			
		protected:
			automaton_state m_State{ automaton_state::stopped };
			node_id m_CurrentNode{ no_node };
			node_id m_StartNode{ no_node };
			node_container m_Nodes{ };
			adjacency_list m_Edges{ };
			::std::default_random_engine m_RNG{ };	//< PRNG
	};
}

// Include implementation of automaton_base
#include "automaton_base_impl.hxx"
