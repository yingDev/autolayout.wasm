#pragma once
#include <string>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/version.hpp>
#include <boost/optional/optional.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include "ast.hpp"
#include <boost/fusion/container/vector.hpp>
static_assert(SPIRIT_X3_VERSION == 0x3003, "wrong spirit version");

namespace evfl
{
	namespace x3 = boost::spirit::x3;
    namespace ascii = x3::ascii;
    using x3::eps;
    using x3::lit;
    using x3::_val;
    using x3::_attr;
    using ascii::char_;
    using x3::uint_;
    using x3::string;
    using x3::omit;
    using x3::rule;

    struct attribute_ : x3::symbols<ast::Attribute>
    {
        attribute_()
        {
            add
            (".left",ast::ATTR_LEFT)
            (".right",ast::ATTR_RIGHT)
            (".top",ast::ATTR_TOP)
            (".bottom",ast::ATTR_BOTTOM)
            (".width",ast::ATTR_WIDTH)
            (".height",ast::ATTR_HEIGHT)
            (".centerX",ast::ATTR_CENTERX)
            (".centerY",ast::ATTR_CENTERY)
			(".l",ast::ATTR_LEFT)
			(".r",ast::ATTR_RIGHT)
			(".t",ast::ATTR_TOP)
			(".b",ast::ATTR_BOTTOM)
			(".w",ast::ATTR_WIDTH)
			(".h",ast::ATTR_HEIGHT)
            (".cx",ast::ATTR_CENTERX)
            (".cy",ast::ATTR_CENTERY);
        }
    } attribute;

    struct orient_ : x3::symbols<ast::Orientation>
    {
        orient_()
        {
        	add
        	("H:",ast::ORIENT_H)
        	("V:",ast::ORIENT_V)
        	("HV:",ast::ORIENT_BOTH)
        	("VH:",ast::ORIENT_BOTH);
        }
    } orient;

    struct relation_ : x3::symbols<ast::Relation>
    {
        relation_()
        {
        	add
        	("==",ast::REL_EQU)
        	(">=",ast::REL_GEQ)
        	("<=",ast::REL_LEQ);
        }
    } relation;

    struct numsign_ : x3::symbols<int>
    {
        numsign_()
        {
        	add
        	("-", -1)
        	("+", +1);
        }
    } numsign;

    struct opsign_ : x3::symbols<ast::OpSign>
    {
        opsign_()
        {
            add
            ("*",  ast::OPSIGN_MUL)
            ("/",  ast::OPSIGN_DIV);
        }
    } opsign;

    //todo: move out
    struct alnumstr_parser : x3::parser<alnumstr_parser>
	{
		typedef std::string attribute_type;

		template <typename Iterator, typename Context, typename RContext, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context const& ctx, RContext& rctx, Attribute& attr) const
		{
			auto i = first;
			char c = *i;
			if( (c >= 'A' && c <='Z') || (c >= 'a' && c <= 'z')  )//ch.parse(i, last, ctx, rctx, c))
			{
				auto str = std::string(); //.push_back(c);
				str.push_back(c);
				++i;

				while(i != last)
				{
					c = *i;
					if( (c >= 'A' && c <='Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '_') )
					{
						str.push_back(c);
						++i;
					}
					else break;
				}
				attr = std::move(str);
				first = i;
				return true;
			}
			return false;
		}
	} alnumstr;

    enum NUM_SIGN_OPTION  { SIGNOPT_UNSIGNED=0, SIGNOPT_SIGNED=1, SIGNOPT_FORCE_SIGNED =2};

    template<NUM_SIGN_OPTION SIGNOP>
    struct number_parser : x3::parser<number_parser<SIGNOP>>
	{
    	typedef double attribute_type;
		static bool const has_attribute = true;

		template <typename Iterator, typename Context, typename RContext, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context const& ctx, RContext& rctx, Attribute& attr) const
		{
			auto i = first;
			int sign = 1;
			unsigned integral;

			if(SIGNOP >= SIGNOPT_SIGNED)
			{
				if(*i == '-')
				{
					sign = -1;
					++i;
				} else if(*i == '+')
					++i;
				else if(SIGNOP == SIGNOPT_FORCE_SIGNED)
					return false;
			}

			if(uint_.parse(i, last, ctx, rctx, integral))
			{
				unsigned fraction = 0;

				if(*i == '.')
				{
					++i;
					if(!uint_.parse(i, last, ctx, rctx, fraction))
						--i;
				}
				unsigned divide = 10;
				while (divide < fraction)
					divide *= 10;

				attr = (integral + ((double)fraction) / divide) * sign;

				first = i;
				return true;
			}

			return false;
		}
	};

    const number_parser<SIGNOPT_SIGNED> number;
    const number_parser<SIGNOPT_FORCE_SIGNED> deltanumber; //+n -n

    auto const viewName = alnumstr;
	auto const superview = char_('|');
    auto const priority = '@' >> uint_;
    auto const constant = number;
	auto const constantExpr = deltanumber;

    const rule<class multiplier, ast::Multiplier> multiplier("multiplier");
    auto const multiplier_def =
    		opsign >> number;

    const rule<class percent, ast::Percentage> percent("percent");
	auto const percent_def =
			number >> '%' >> -constantExpr;

	auto const viewPred =
			   ((string("-") >> !lit(".center")  ) | string("^") | viewName)
			>> -attribute >> -multiplier >> -constantExpr;

    const rule<class predicate, ast::Predicate> predicate("predicate");
    auto const predicate_def =
            -relation >> ( // ObjectOfPredicate
					percent
				  | constant
				  | viewPred
            ) >> -priority;

    const auto longPredicate = '(' >> (predicate % ',') >> ')';

    const rule<class predicateList, ast::PredicateList> predicateList("predicateList");
    auto const predicateList_def =
			  priority
    		| percent
    		| constant
            | longPredicate; //(>=123,<=456

    const rule<class connection, ast::Connection> connection("connection");
    auto const connection_def =
              ("->" >> x3::attr(boost::none)  >> x3::attr(ast::CONNECTOR_ARROW))
            | ("-"  >> -(predicateList >>'-') >> x3::attr(ast::CONNECTOR_HYPHEN))
            | ("~"  >> -(predicateList >>'~') >> x3::attr(ast::CONNECTOR_TILDE))
            | (""   >> x3::attr(boost::none)  >> x3::attr(ast::CONNECTOR_CLOSED));

    const rule<class view, ast::View> view("view");

    const rule<class viewGroup, ast::ViewGroup> viewGroup("viewGroup");
    auto const viewGroup_def =
    		'[' >> (view % ',') >> ']';

    const rule<class cascadedViews, ast::CascadedViews> cascadedViews("cascadedViews");
    auto const cascadedViews_def =
    		':' >> +(connection >> viewGroup) >> connection;

    auto const view_def =
			   viewName //name
            >> -longPredicate
            >> -cascadedViews;

    auto const visualFmtContent = -superview >> +(connection >> viewGroup) >> connection >> -superview;
    auto const constraintFmtContent = (viewName | ('['>> (viewName % ',') >>']')) >> +(attribute >> '(' >> (predicate % ',') >> ')');

//	auto const extendedVisualFormat_def =
//			     ("C:" >> constraintFmtContent)
//			   | (orient >> visualFmtContent);
//	const rule<class extendedVisualFormat, ast::ExtendedVisualFormat> extendedVisualFormat = "evfl";

	auto const spaces = +lit(' ');
	auto const lineSeparator = *omit[char_("; \n\r\t")];

	const rule<class multiExtendedVisualFormat, ast::MultiExtendedVisualFormat> multiExtendedVisualFormat = "multi-evfl";
	auto const multiExtendedVisualFormat_def =
			lineSeparator >> (
				(
						("C:" >> constraintFmtContent % spaces )
					  | (orient >> (x3::attr(ast::ORIENT_NONE) >> visualFmtContent) % spaces )
				) % lineSeparator
			) >> lineSeparator;

	BOOST_SPIRIT_DEFINE(multiplier,percent, predicateList, predicate, connection, cascadedViews, viewGroup, view, multiExtendedVisualFormat);
}
