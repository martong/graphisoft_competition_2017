#include <array>
#include <iostream>
#include <vector>

// Number of rotation matrices: 24
using RotationMatrix = std::array<std::array<int, 3>, 3>;

RotationMatrix operator*(RotationMatrix lhs, RotationMatrix rhs) {
    // ABij = SUM(Aik * Bkj)
    return {{
            {{
                lhs[0][0] * rhs[0][0] + lhs[0][1] * rhs[1][0] + lhs[0][2] * rhs[2][0],
                lhs[0][0] * rhs[0][1] + lhs[0][1] * rhs[1][1] + lhs[0][2] * rhs[2][1],
                lhs[0][0] * rhs[0][2] + lhs[0][1] * rhs[1][2] + lhs[0][2] * rhs[2][2]
            }},
            {{
                lhs[1][0] * rhs[0][0] + lhs[1][1] * rhs[1][0] + lhs[1][2] * rhs[2][0],
                lhs[1][0] * rhs[0][1] + lhs[1][1] * rhs[1][1] + lhs[1][2] * rhs[2][1],
                lhs[1][0] * rhs[0][2] + lhs[1][1] * rhs[1][2] + lhs[1][2] * rhs[2][2]
            }},
            {{
                lhs[2][0] * rhs[0][0] + lhs[2][1] * rhs[1][0] + lhs[2][2] * rhs[2][0],
                lhs[2][0] * rhs[0][1] + lhs[2][1] * rhs[1][1] + lhs[2][2] * rhs[2][1],
                lhs[2][0] * rhs[0][2] + lhs[2][1] * rhs[1][2] + lhs[2][2] * rhs[2][2]
            }},
    }};
}

const std::vector<RotationMatrix> rotations = {
    {{
        {{1, 0, 0}},
        {{0, 1, 0}},
        {{0, 0, 1}},
    }},
    {{
        {{-1, 0, 0}},
        {{0, -1, 0}},
        {{0, 0, 1}},
    }},
    {{
        {{-1, 0, 0}},
        {{0, 0, -1}},
        {{0, -1, 0}},
    }},
    {{
        {{-1, 0, 0}},
        {{0, 0, 1}},
        {{0, 1, 0}},
    }},
    {{
        {{-1, 0, 0}},
        {{0, 1, 0}},
        {{0, 0, -1}},
    }},
    {{
        {{0, -1, 0}},
        {{-1, 0, 0}},
        {{0, 0, -1}},
    }},
    {{
        {{0, -1, 0}},
        {{0, 0, -1}},
        {{1, 0, 0}},
    }},
    {{
        {{0, -1, 0}},
        {{0, 0, 1}},
        {{-1, 0, 0}},
    }},
    {{
        {{0, -1, 0}},
        {{1, 0, 0}},
        {{0, 0, 1}},
    }},
    {{
        {{0, 0, -1}},
        {{-1, 0, 0}},
        {{0, 1, 0}},
    }},
    {{
        {{0, 0, -1}},
        {{0, -1, 0}},
        {{-1, 0, 0}},
    }},
    {{
        {{0, 0, -1}},
        {{0, 1, 0}},
        {{1, 0, 0}},
    }},
    {{
        {{0, 0, -1}},
        {{1, 0, 0}},
        {{0, -1, 0}},
    }},
    {{
        {{0, 0, 1}},
        {{-1, 0, 0}},
        {{0, -1, 0}},
    }},
    {{
        {{0, 0, 1}},
        {{0, -1, 0}},
        {{1, 0, 0}},
    }},
    {{
        {{0, 0, 1}},
        {{0, 1, 0}},
        {{-1, 0, 0}},
    }},
    {{
        {{0, 0, 1}},
        {{1, 0, 0}},
        {{0, 1, 0}},
    }},
    {{
        {{0, 1, 0}},
        {{-1, 0, 0}},
        {{0, 0, 1}},
    }},
    {{
        {{0, 1, 0}},
        {{0, 0, -1}},
        {{-1, 0, 0}},
    }},
    {{
        {{0, 1, 0}},
        {{0, 0, 1}},
        {{1, 0, 0}},
    }},
    {{
        {{0, 1, 0}},
        {{1, 0, 0}},
        {{0, 0, -1}},
    }},
    {{
        {{1, 0, 0}},
        {{0, -1, 0}},
        {{0, 0, -1}},
    }},
    {{
        {{1, 0, 0}},
        {{0, 0, -1}},
        {{0, 1, 0}},
    }},
    {{
        {{1, 0, 0}},
        {{0, 0, 1}},
        {{0, -1, 0}},
    }},
};

std::vector<RotationMatrix> localRotations = {{
        {{
           {{-1,  0,  0}},
           {{ 0, -1,  0}},
           {{ 0,  0,  1}}
        }},
        {{
           {{ 1,  0,  0}},
           {{ 0,  0,  1}},
           {{ 0, -1,  0}}
        }},
        {{
           {{ 1,  0,  0}},
           {{ 0, -1,  0}},
           {{ 0,  0, -1}}
        }},
        {{
           {{ 1,  0,  0}},
           {{ 0,  0, -1}},
           {{ 0,  1,  0}}
        }},
        {{
           {{ 0,  1,  0}},
           {{-1,  0,  0}},
           {{ 0,  0,  1}}
        }},
        {{
           {{ 0,  0,  1}},
           {{ 0,  1,  0}},
           {{-1,  0,  0}}
        }},
        {{
           {{-1,  0,  0}},
           {{ 0,  1,  0}},
           {{ 0,  0, -1}}
        }},
        {{
                {{ 0,  0, -1}},
                {{ 0,  1,  0}},
                {{ 1,  0,  0}}
            }},
        {{
                {{-1,  0,  0}},
                {{ 0,  0,  1}},
                {{ 0,  1,  0}}
            }},
        {{
                {{ 0,  1,  0}},
                {{-1,  0,  0}},
                {{ 0,  0,  1}}
            }},
        {{
                {{-1,  0,  0}},
                {{ 0, -1,  0}},
                {{ 0,  0,  1}}
            }},
        {{
                {{ 0, -1,  0}},
                {{ 1,  0,  0}},
                {{ 0,  0,  1}}
            }},
        {{
                {{-1,  0,  0}},
                {{ 0,  1,  0}},
                {{ 0,  0, -1}}
            }},
        {{
                {{ 0,  1,  0}},
                {{-1,  0,  0}},
                {{ 0,  0,  1}}
            }},
        {{
                {{-1,  0,  0}},
                {{ 0, -1,  0}},
                {{ 0,  0,  1}}
            }},
        {{
                {{ 0, -1,  0}},
                {{ 1,  0,  0}},
                {{ 0,  0,  1}}
            }},
        {{
                {{-1,  0,  0}},
                {{ 0,  0,  1}},
                {{ 0,  1,  0}}
            }},
        {{
                {{ 0,  0,  1}},
                {{ 0,  1,  0}},
                {{-1,  0,  0}}
            }},
        {{
                {{-1,  0,  0}},
                {{ 0,  1,  0}},
                {{ 0,  0, -1}}
            }},
        {{
                {{ 0,  0, -1}},
                {{ 0,  1,  0}},
                {{ 1,  0,  0}}
            }},
        {{
                {{ 0, -1,  0}},
                {{ 1,  0,  0}},
                {{ 0,  0,  1}}
            }},
        {{
                {{ 1,  0,  0}},
                {{ 0,  0,  1}},
                {{ 0, -1,  0}}
            }},
        {{
                {{ 1,  0,  0}},
                {{ 0, -1,  0}},
                {{ 0,  0, -1}}
            }},
    }};

int main() {
    const RotationMatrix identity = {{ {{1, 0, 0}}, {{0, 1, 0}}, {{0, 0, 1}} }};
    RotationMatrix m = identity;
    for (std::size_t i = 0; i < localRotations.size(); ++i) {
        m = m * localRotations[i];
        std::cout << (m == rotations[i+1]) << std::endl;
    }
}
