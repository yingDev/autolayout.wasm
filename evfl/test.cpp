#include <string>
#include <iostream>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/version.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include "ast.hpp"
#include "syntax.hpp"
#include "../autolayout/constraint_def.h"
#include "../autolayout/view.h"
#include "visit.hpp"

using namespace std::string_literals;

namespace evfl::test
{
    namespace x3 = boost::spirit::x3;
    namespace ascii = x3::ascii;
    namespace ast = evfl::ast;
    using ascii::space;
    using x3::uint_;
    using x3::char_;
    using x3::lit;

    void multiplier()
    {
        const int N = 4;
        const double val = 123.45;
        const std::string inputs[N] = {"*-3", "/10", "*1", "/-2"};
        const double answers[N] = {val*-3, val/10, val*1, val/-2};

        for(auto i=0; i<N; i++)
        {
            auto input = inputs[i];
            auto answer = answers[i];

            auto begin = input.begin();
            auto end = input.end();
            ast::Multiplier out;
            auto ok = (x3::phrase_parse(begin, end, evfl::multiplier, space, out));
            assert(ok);
            assert(begin == end);

            auto result = out.value() * val;
            assert( abs(result - answer) < std::numeric_limits<double>::epsilon());
        }
    }

    void constant_expr()
    {
        const int N = 2;
        const double val = 10;
        const std::string inputs[N] = {"-1.23", "+10"};
        const double answers[N] = {val - 1.23, val + 10};

        for(auto i=0; i<N; i++)
        {
            auto input = inputs[i];
            auto answer = answers[i];

            auto begin = input.begin();
            auto end = input.end();
            ast::ConstantExpr out;
            auto ok = (x3::phrase_parse(begin, end, evfl::constantExpr, space, out));
            assert(ok);
            assert(begin == end);

            auto result = val + out;
            assert( abs(result - answer) < std::numeric_limits<double>::epsilon());
        }
    }


    void percent()
	{
    	auto const input = "50%+123"s;
    	auto begin = input.begin();
    	auto end = input.end();
    	ast::Percentage out;

    	auto ok = x3::phrase_parse(begin, end, evfl::percent, space, out);
    	assert(ok);
    	assert(begin == end);
    	assert(out.value() == 0.5);
    	assert(out.constExpr.value() == 123);
	}

    void predicate()
    {
        {
            auto input = ">=asdf.left*10+3@999"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::Predicate out;
            auto ok = (x3::phrase_parse(begin, end, evfl::predicate, space, out));
            assert(ok);
            assert(begin == end);
            assert(out.relation.value() == ast::REL_GEQ);
            auto* pred = boost::get<ast::ViewPredicate>(&out.elements);
            assert(pred->viewName == "asdf");
            assert(pred->constantExpr.value() == 3);
            assert(pred->multiplier.value().value() == 10);
            assert(pred->attribute.value() == ast::ATTR_LEFT);
            assert(out.priority.value() == 999);
        }
    }

    void predicateList()
    {
        {
            auto input = "(a.left*10+3,<=b.right-1)"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::PredicateList out;
            auto ok = (x3::phrase_parse(begin, end, evfl::predicateList, space, out));
            assert(ok);
            assert(begin == end);
        }

        {
            auto input = "123"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::PredicateList out;
            auto ok = (x3::phrase_parse(begin, end, evfl::predicateList, space, out));
            assert(ok);
            assert(begin == end);
        }
    }

    void connection()
    {
        {
            auto input = "-"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::Connection out;
            auto ok = (x3::phrase_parse(begin, end, evfl::connection, space, out));
            assert(ok);
            assert(begin == end);
            assert(out.connector == ast::CONNECTOR_HYPHEN && !out.predicates.has_value());
        }

        {
            auto input = "~"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::Connection out;
            auto ok = (x3::phrase_parse(begin, end, evfl::connection, space, out));
            assert(ok);
            assert(begin == end);
            assert(out.connector == ast::CONNECTOR_TILDE && !out.predicates.has_value());
        }

        {
            auto input = "->"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::Connection out;
            auto ok = (x3::phrase_parse(begin, end, evfl::connection, space, out));
            assert(ok);
            assert(begin == end);
            assert(out.connector == ast::CONNECTOR_ARROW);
        }

        {
            auto input = "-123-"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::Connection out;
            auto ok = (x3::phrase_parse(begin, end, evfl::connection, space, out));
            assert(ok);
            assert(begin == end);
            assert(out.connector == ast::CONNECTOR_HYPHEN && boost::get<ast::Constant>(out.predicates.value()) == 123);
        }

        {
            auto input = "~123~"s;
            auto begin = input.begin();
            auto end = input.end();
            ast::Connection out;
            auto ok = (x3::phrase_parse(begin, end, evfl::connection, space, out));
            assert(ok);
            assert(begin == end);
            assert(out.connector == ast::CONNECTOR_TILDE && boost::get<ast::Constant>(out.predicates.value()) == 123);
        }
    }

    void cascadedViews()
    {
        auto input = ":-[hello,shit]"s;
        auto begin = input.begin();
        auto end = input.end();
        ast::CascadedViews out;
        auto ok = (x3::phrase_parse(begin, end, evfl::cascadedViews, space, out));
        assert(ok);
        assert(begin == end);
    }

    void view()
    {
        auto input = "hello"s;
        auto begin = input.begin();
        auto end = input.end();
        ast::View out;
        auto ok = (x3::phrase_parse(begin, end, evfl::view, space, out));
        assert(ok);
        assert(begin == end);
    }

    void viewGroup()
    {
        auto input = "[a,b,c,d]"s;

        auto begin = input.begin();
        auto end = input.end();
        ast::ViewGroup out;
        auto ok = (x3::phrase_parse(begin, end, evfl::viewGroup, space, out));
        assert(ok);
        assert(begin == end);
        assert(out.size() == 4);
    }

    void extendedVisualFormat()
    {
//        auto input = "H:|-[asdf,hello(>=123@345)]-99-[jjjj(asdf*100)]-(32%)-|"s;
//        auto begin = input.begin();
//        auto end = input.end();
//        ast::ExtendedVisualFormat out;
//        auto ok = (x3::phrase_parse(begin, end, evfl::extendedVisualFormat, space, out));
//        assert(ok);
//        assert(begin == end);
    }

    void mevfl()
	{
		auto input = "H:|[asdf]| [b(123)] [c]-(444@555)-|"
					 "V:|-[a]-55%-[b]-|"
					 "C:a.width(100)"
					 "HV:|[x]|"
					 "C:[a,b,c].centerX(100%+123)"s;

    	auto begin = input.begin();
    	auto end = input.end();
    	ast::MultiExtendedVisualFormat out;
    	auto ok = x3::parse(begin, end, evfl::multiExtendedVisualFormat, out);
    	assert(ok);
    	assert(begin == end);

		{
			auto* row0 = boost::get<ast::MultiVisualFormatRow>(&out[0]);
			assert(row0);
			assert(row0->orientation == ast::ORIENT_H);
			assert(row0->items.size() == 3);
		}

		{
			auto* row1 = boost::get<ast::MultiVisualFormatRow>(&out[1]);
			assert(row1);
			assert(row1->orientation == ast::ORIENT_V);
			assert(row1->items.size() == 1);
		}

		{
			auto* row2 = boost::get<ast::MultiConstraintFormatRow >(&out[2]);
			assert(row2);
			assert(row2->size() == 1);
		}

		{
			auto* row3 = boost::get<ast::MultiVisualFormatRow>(&out[3]);
			assert(row3);
			assert(row3->orientation == ast::ORIENT_BOTH);
			assert(row3->items.size() == 1);
		}

		{
			auto* row4 = boost::get<ast::MultiConstraintFormatRow>(&out[4]);
			assert(row4);
			assert(row4->size() == 1);
		}

		std::vector<ast::ConstraintDef> defs;
    	evfl::visit::visitMultiEvfl(out, defs);

    	for(auto& c : defs)
    		std::cout << c << std::endl;

    	auto v = autolayout::View{};
    	for(auto& c :defs)
    		v.addConstraint(c);

    	assert(defs.size() == 17);
	}

    void all()
    {
        multiplier();
        constant_expr();
        percent();
        predicate();
        predicateList();
        connection();
        cascadedViews();
        view();
        viewGroup();
        extendedVisualFormat();

        mevfl();
    }
};

int main()
{
    namespace x3 = boost::spirit::x3;
    namespace ascii = x3::ascii;
    namespace al = autolayout;
    namespace ast = evfl::ast;

    evfl::test::all();

//	{
//		std::cout << std::endl;
//
//		auto input = "H:[g(33%):[a(50%)][b(123)][c(^/2)]]"s;
//		auto begin = input.begin();
//		auto end = input.end();
//
//		ast::ExtendedVisualFormat out;
//		auto ok = x3::phrase_parse(begin, end, evfl::extendedVisualFormat, ascii::space, out);
//		assert(ok);
//		assert(begin == end);
//
//		std::vector<al::ConstraintDef> output;
//		evfl::visit::visitEvfl(out, output);
//
//		for(auto c : output)
//			std::cout << c << std::endl;
//
//		std::cout << std::endl;
//	}
//
//
//    {
//        auto input = "H:|[a][b]-55%-|"s;
//        auto begin = input.begin();
//        auto end = input.end();
//        ast::ExtendedVisualFormat out;
//        auto ok = x3::phrase_parse(begin, end, evfl::extendedVisualFormat, ascii::space, out);
//        assert(ok);
//        assert(begin == end);
//
//        auto vf = boost::get<ast::VisualFormat>(out);
//        assert(vf.superTo().connector == ast::CONNECTOR_CLOSED);
//        assert(vf.toSuper().connector == ast::CONNECTOR_HYPHEN &&
//               boost::get<ast::Percentage>(vf.toSuper().predicates.get_ptr()));
//    }
//
//    auto input = "H:|-[asdf,hello(>=123@345)]-99-[jjjj(asdf*100)]-(<=hello.width*10+2,>=100)-[x(123)]-32%-|"s;
//    //auto input = "H:|~[a]-[b]-[c]~|"s;
//    //auto input = "C:a.width(b*100+1@123,100).height(100)"s;
//
//
//
//   auto start = std::chrono::high_resolution_clock::now();
//   for (auto i = 0; i < 100; i++)
//   {
//        auto begin = input.begin();
//        auto end = input.end();
//
//    	std::vector<al::ConstraintDef> output;
//        ast::ExtendedVisualFormat out;
//        auto ok = x3::phrase_parse(begin, end, evfl::extendedVisualFormat, ascii::space, out);
//        assert(ok);
//        assert(begin == end);
//        evfl::visit::visitEvfl(out, output);
//    }
//    auto finish = std::chrono::high_resolution_clock::now();
//    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count() / 1000.0 << "ms\n";

   //std::cout << "count= " << output.size() << std::endl;
   //for(auto& def : output)
   //   std::cout << def << std::endl;

}
