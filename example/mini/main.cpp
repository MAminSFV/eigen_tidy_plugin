#ifdef MOCK_EIGEN
// Mock Eigen types for demonstration when Eigen is not available
namespace Eigen {
    template<typename T, int Rows, int Cols>
    class Matrix {
    public:
        Matrix() = default;
        Matrix(const Matrix&) = default;
        Matrix& operator=(const Matrix&) = default;
        Matrix(int rows, int cols) {} // Constructor for dynamic sizes

        Matrix operator+(const Matrix& other) const { return Matrix{}; }
        Matrix operator*(const Matrix& other) const { return Matrix{}; }

        Matrix eval() const { return *this; }
    };

    template<typename Derived>
    class PlainObjectBase {
    public:
        PlainObjectBase() = default;
    };

    using MatrixXd = Matrix<double, -1, -1>;
    using Vector3d = Matrix<double, 3, 1>;

    // Mock expression template
    template<typename LHS, typename RHS>
    class BinaryOp {
    public:
        BinaryOp(const LHS& lhs, const RHS& rhs) : lhs_(lhs), rhs_(rhs) {}
        MatrixXd eval() const { return MatrixXd{}; }
    private:
        const LHS& lhs_;
        const RHS& rhs_;
    };
}
#else
#include <Eigen/Dense>
#endif

#include <iostream>
#include <vector>

int main() {
    using namespace Eigen;

    MatrixXd matrix1(3, 3);
    MatrixXd matrix2(3, 3);
    Vector3d vec1, vec2;

    // These should trigger warnings:

    // Basic auto with Eigen types
    auto bad1 = matrix1;                    // eigen-avoid-auto warning
    auto bad2 = matrix1 + matrix2;          // eigen-avoid-auto warning
    auto bad3 = matrix1 * matrix2;          // eigen-avoid-auto warning
    auto bad4 = vec1 + vec2;                // eigen-avoid-auto warning

    // Auto with references/pointers/const
    const auto bad5 = matrix1;              // eigen-avoid-auto warning
    auto& bad6 = matrix1;                   // eigen-avoid-auto warning
    auto* bad7 = &matrix1;                  // eigen-avoid-auto warning (pointer to Eigen type)

    // decltype(auto) cases (if BanDecltypeAuto is true)
    decltype(auto) bad8 = matrix1;          // eigen-avoid-auto warning
    decltype(auto) bad9 = (matrix1 + matrix2); // eigen-avoid-auto warning

    // Range-based for loops (warning depends on AllowInRangeFor option)
    std::vector<MatrixXd> matrices{matrix1, matrix2};
    for (auto m : matrices) {               // May warn depending on config
        std::cout << "Matrix\n";
    }

    // These should be OK:

    // Explicit types
    MatrixXd good1 = matrix1;               // OK
    MatrixXd good2 = matrix1 + matrix2;     // OK - forces evaluation
    Vector3d good3 = vec1 + vec2;           // OK - forces evaluation

    // Using eval() explicitly
    auto good4 = (matrix1 + matrix2).eval(); // OK - explicit evaluation
    auto good5 = matrix1.eval();            // OK - explicit evaluation

    // Non-Eigen types
    auto good6 = 42;                        // OK - not Eigen
    auto good7 = std::string("hello");      // OK - not Eigen
    auto good8 = matrices.begin();          // OK - iterator, not Eigen

    // Explicit const/ref with proper types
    const MatrixXd& good9 = matrix1;        // OK - explicit type
    MatrixXd* good10 = &matrix1;            // OK - explicit pointer type

    std::cout << "Example completed. Check for clang-tidy warnings!\n";

    return 0;
}
