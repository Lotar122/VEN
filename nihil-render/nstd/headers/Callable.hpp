#pragma once
namespace nihil {
	namespace nstd{
		/*
			* @brief A callable function. Use structs for input and output for best expierience.
			*
			*
			* @tparam cT - the CallableTemplate with the required arguments.
		*/
		template<typename cT>
		class Callable {
		public:
			typename cT::fT function;

			/*
			* @brief call this to execute your function
			*
			* @param input - the input data of type iT
			* @return output of type oT
			*/
			inline typename cT::oT execute(typename cT::iT input) {
				return function(input);
			}

			Callable()
			{}

			Callable(typename cT::fT _function)
			{
				function = _function;
			}
		};

		/*
			* @brief The template parameters for the Callable class.
			*
			*
			* @tparam fT - the type of the function, let the type deduction do it
			* @tparam oT - the outputs type
			* @tparam iT - the inputs type
		*/
		template<typename PiT, typename PoT, typename PfT>
		struct CallableTemplate {
			using iT = PiT;
			using oT = PoT;
			using fT = PfT;
		};
	}
}