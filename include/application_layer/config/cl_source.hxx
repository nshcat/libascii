#pragma once

#include <cl/command_base.hxx>
#include "config_source.hxx"
#include "config_scheme.hxx"

namespace application_layer::config
{
	// A configuration source that uses a configuration scheme to retrieve
	// values from the commandline argument parser
	class cl_source
		: public config_source
	{
		public:
			// The handler is only saved by reference.
			cl_source(const config_scheme& p_scheme, const cl::internal::command_base& p_handler);
			
			cl_source(const cl_source&) = default;
			cl_source(cl_source&&) = default;
			
			cl_source& operator=(const cl_source&) = default;
			cl_source& operator=(cl_source&&) = default;
	
		public:
			virtual auto populate(boost::property_tree::ptree&)
				-> boost::property_tree::ptree& override;
				
		protected:
			config_scheme m_Scheme{ };						//< We do store a copy of the scheme
			const cl::internal::command_base& m_Handler;	//< Reference to commandline argument parser
	};
}
