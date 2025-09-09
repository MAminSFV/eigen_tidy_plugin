// Simple test to debug auto detection
namespace Eigen {
    template<typename T, int Rows, int Cols>
    class Matrix {
    public:
        Matrix() = default;
    };
    using MatrixXd = Matrix<double, -1, -1>;
}

void test() {
    using namespace Eigen;
    MatrixXd m1;

    auto case1 = m1;        // Should be caught
    const auto case2 = m1;  // Should be caught
    auto& case3 = m1;       // Should be caught
}
