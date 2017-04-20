#include "BestChooser.hpp"
#include "LookaheadChooser.hpp"
#include "PrincessMovingChooser.hpp"
#include "RandomChooser.hpp"
#include "StrategyParser.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <algorithm>
#include <stdexcept>
#include <memory>
#include <iostream>

namespace {

namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template<typename T, typename... Args>
auto make_shared_(Args&&... args) {
    return phoenix::construct<std::shared_ptr<T>>(
            phoenix::new_<T>(std::forward<Args>(args)...));
}

template<typename Iterator>
struct strategy : qi::grammar<Iterator, ChoosingStrategy(), ascii::space_type> {
    strategy(std::mt19937& rng) : strategy::base_type(start) {
        using qi::lit;
        using qi::_val;
        using qi::double_;
        using qi::_1;
        using qi::_2;

        start = ("ChoosingStrategy" >> lit('(') >> chooser >> lit(')'))[
                _val = phoenix::construct<ChoosingStrategy>(_1)];
        chooser %= randomChooser | lookaheadChooser | princessMovingChooser
                | bestChooser;
        randomChooser = (lit("RandomChooser") >> lit('(') >> lit(')'))[
                _val = make_shared_<RandomChooser>(phoenix::ref(rng))];
        lookaheadChooser = (lit("LookaheadChooser") >> lit('(')
                >> chooser >> ',' >> double_ >> lit(')'))[
                        _val = make_shared_<LookaheadChooser>(_1, _2)];
        princessMovingChooser = (lit("PrincessMovingChooser") >> lit('(')
                >> chooser >> ',' >> double_ >> lit(')'))[
                        _val = make_shared_<PrincessMovingChooser>(_1, _2)];
        bestChooser = (lit("BestChooser") >> lit('(')
                >> chooser >> lit(')'))[
                        _val = make_shared_<BestChooser>(_1)];
    }

    qi::rule<Iterator, ChoosingStrategy(), ascii::space_type> start;
    qi::rule<Iterator, std::shared_ptr<IChooser>(), ascii::space_type>
            chooser;
    qi::rule<Iterator, std::shared_ptr<RandomChooser>(), ascii::space_type>
            randomChooser;
    qi::rule<Iterator, std::shared_ptr<LookaheadChooser>(), ascii::space_type>
            lookaheadChooser;
    qi::rule<Iterator, std::shared_ptr<PrincessMovingChooser>(),
            ascii::space_type> princessMovingChooser;
    qi::rule<Iterator, std::shared_ptr<BestChooser>(),
            ascii::space_type> bestChooser;
};


} // unnamed namespace

ChoosingStrategy parseStrategy(const std::string& input, std::mt19937& rng) {
    auto begin = input.begin();
    auto end = input.end();
    strategy<std::string::const_iterator> parser{rng};
    ChoosingStrategy result;
    bool success = qi::phrase_parse(begin, end, parser, ascii::space, result);
    if (!success) {
        std::cerr << "Parse error\n";
        throw std::logic_error{"Parse error"};
    }
    if (begin != end) {
        std::cerr << "Parse error at\n";
        std::copy(begin, end, std::ostream_iterator<char>(std::cerr));
        std::cerr << "\n";
        throw std::logic_error{"Parse error"};
    }
    return result;
}