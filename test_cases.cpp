// Mock Eigen for testing
namespace Eigen {
    // Forward declarations
    template<typename Derived> class MatrixBase;
    template<typename Lhs, typename Rhs> class CwiseBinaryOp;
    enum { Dynamic = -1 };

    // Expression template for additions
    template<typename Lhs, typename Rhs>
    class SumExpr : public MatrixBase<SumExpr<Lhs, Rhs>> {
    public:
        SumExpr(const Lhs& lhs, const Rhs& rhs) : m_lhs(lhs), m_rhs(rhs) {}
        SumExpr eval() const { return *this; }
    private:
        const Lhs& m_lhs;
        const Rhs& m_rhs;
    };

    template<typename Derived>
    class PlainObjectBase : public MatrixBase<Derived> {};

    template<typename T, int Rows, int Cols>
    class Matrix : public PlainObjectBase<Matrix<T, Rows, Cols>> {
    public:
        Matrix() = default;
        Matrix eval() const { return *this; }
    };

    template<typename Derived>
    class MatrixBase {
      public:
        template<typename OtherDerived>
        SumExpr<Derived, OtherDerived> operator+(const MatrixBase<OtherDerived>& other) const {
            return SumExpr<Derived, OtherDerived>(*static_cast<const Derived*>(this), *static_cast<const OtherDerived*>(&other));
        }
    };

    using MatrixXd = Matrix<double, Dynamic, Dynamic>;
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
    MatrixXd good2 = (m1 + m2).eval(); // OK
    auto good3 = m1.eval();      // OK
    auto good4 = 42;             // OK - not Eigen
}
