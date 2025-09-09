// Mock Eigen for testing
namespace Eigen {
    template<typename T, int Rows, int Cols>
    class Matrix {
    public:
        Matrix() = default;
        Matrix operator+(const Matrix& other) const { return Matrix{}; }
        Matrix eval() const { return *this; }
    };
    
    template<typename Derived>
    class PlainObjectBase {};
    
    using MatrixXd = Matrix<double, -1, -1>;
    using Vector3d = Matrix<double, 3, 1>;
}

void test_function() {
    using namespace Eigen;
    
    MatrixXd m1, m2;
    Vector3d v1, v2;
    
    // Should trigger warnings
    auto bad1 = m1;              // eigen-avoid-auto
    auto bad2 = m1 + m2;         // eigen-avoid-auto
    const auto bad3 = v1;        // eigen-avoid-auto
    auto& bad4 = m1;             // eigen-avoid-auto
    decltype(auto) bad5 = m1;    // eigen-avoid-auto (if BanDecltypeAuto=true)
    
    // Should be OK
    MatrixXd good1 = m1;         // OK
    MatrixXd good2 = m1 + m2;    // OK
    auto good3 = m1.eval();      // OK
    auto good4 = 42;             // OK - not Eigen
}
