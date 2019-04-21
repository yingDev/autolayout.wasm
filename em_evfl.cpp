#include <string>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/version.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/emscripten.h>
#include "evfl/ast.hpp"
#include "evfl/syntax.hpp"
#include "evfl/visit.hpp"
#include "autolayout/constraint_def.h"

using namespace emscripten;

//opaque pointer
size_t parse_evfl(std::string input, val defPrio)
{
    namespace x3 = boost::spirit::x3;
    namespace ascii = x3::ascii;

	auto* output = new std::vector<evfl::ast::ConstraintDef>{};
	output->reserve(16);

	auto begin = input.begin();
	auto end = input.end();
	evfl::ast::MultiExtendedVisualFormat ast;
	auto ok = x3::parse(begin, end, evfl::multiExtendedVisualFormat, ast);

	if(!ok || begin != end)
		emscripten_log(EM_LOG_ERROR, "%s: error parsing at %d: %s", __func__, begin-input.begin(), input.data());

	evfl::visit::visitMultiEvfl(ast, *output);

	if(!defPrio.isUndefined())
	{
		double prio = defPrio.as<unsigned>();
		for(auto& c : *output)
		{
			if(c.priority)
				continue;
			c.priority = prio;
		}
	}

	return (size_t)(void*)output;
}

EMSCRIPTEN_BINDINGS(evfl)
{
    function("parse_evfl", &parse_evfl, allow_raw_pointers());
}
