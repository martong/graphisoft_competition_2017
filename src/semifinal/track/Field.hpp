#ifndef SEMIFINAL_TRACK_FIELD_HPP
#define SEMIFINAL_TRACK_FIELD_HPP

#include "Point.hpp"

#include <boost/optional.hpp>

#include <algorithm>
#include <array>
#include <assert.h>
#include <vector>

constexpr std::size_t numNeighbors = 4;
using Neighbors = std::array<Point, numNeighbors>;

inline constexpr
std::size_t oppositeDirection(std::size_t direction) {
    return (direction + numNeighbors / 2) % numNeighbors;
}

enum Directions : std::size_t {
    left, up, right, down
};

constexpr Neighbors neighbors{
        {-p10, -p01, p10, p01}};

constexpr const Neighbors& getNeighbors(Point) {
    return neighbors;
}

struct Field {

    int type = 0;
    int monitor = -1; // monitor ID, -1 = none

    Field() : type(0), monitor(-1), princesses() {}
    template<typename PrincessList = std::initializer_list<int>>
    Field(int type, int monitor, const PrincessList& princessList = {})
        : type(type), monitor(monitor), princesses{0} {
        for (int princess : princessList) {
            addPrincess(princess);
        }
    }

    void removePrincess(int p) {
        princesses &= ~(1 << p);
    }

    void addPrincess(int p) {
        princesses |= 1 << p;
    }

    bool hasPrincess(int p) const {
        return princesses & (1 << p);
    }

    bool noPrincess() const {
        return princesses == 0;
    }

    friend bool operator==(const Field& lhs, const Field& rhs) {
        return lhs.type == rhs.type && lhs.monitor == rhs.monitor &&
               lhs.princesses == rhs.princesses;
    }

private:
    int princesses;
};

constexpr std::size_t numFieldTypes = 16;
extern const std::array<std::string, numFieldTypes> fieldTypes;

constexpr std::array<int, 10> playerColors {{31, 32, 33, 34, 35, 36, 91, 92, 93, 94}};

constexpr int defaultColor = 0;
constexpr int monitorColor = 97;

std::string setColor(int background, int foreground);

inline std::string clearColor() { return "\e[0m"; }
inline std::string clearScreen() { return "\e[2J"; }

inline
bool canMove(int fieldType, std::size_t direction) {
    constexpr std::array<int, numNeighbors> directionBits{{
            2, 1, 8, 4}};
    assert(direction <  numNeighbors);
    return fieldType & directionBits[direction];
}

const std::vector<int>& getIsomorphs(int fieldType);

constexpr int BOXHEIGHT = 5;
constexpr int BOXWIDTH = 9;

struct ColorInfo {
    std::string baseColor;
    std::string monitorColor;
    std::vector<std::string> princessColors;
    bool blocked = false;
};

struct ColorDrawer {
    ColorDrawer(std::string& line, const std::string& color,
            std::string base = clearColor())
        : line(line), base(std::move(base)) {
        line.append(color);
    }
    ~ColorDrawer() {
        line.append(base);
    }
    std::string& line;
    std::string base;
};

std::string getBoxLine(const Field& field, unsigned i,
        const boost::optional<ColorInfo>& colorInfo = boost::none);

std::string toBox(const Field& field);

#endif // SEMIFINAL_TRACK_FIELD_HPP
