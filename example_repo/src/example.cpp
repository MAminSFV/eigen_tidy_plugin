#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <vector>

void test_eigen() {
  using namespace Eigen;

  // Initialize some matrices and vectors
  MatrixXd m1 = MatrixXd::Random(3, 3);
  MatrixXd m2 = MatrixXd::Identity(3, 3);
  Vector3d v1(1.0, 2.0, 3.0);
  Vector3d v2(4.0, 5.0, 6.0);

  // Fixed-size types
  Matrix3d m3x3_1 = Matrix3d::Identity();
  Matrix3d m3x3_2 = Matrix3d::Random();

  // Array types
  ArrayXXd a1 = ArrayXXd::Random(3, 3);
  ArrayXXd a2 = ArrayXXd::Ones(3, 3);

  // ===== Should trigger warnings =====

  // 1. Plain object assignment with auto
  auto bad1 = m1;     // eigen-avoid-auto: MatrixXd
  auto bad2 = v1;     // eigen-avoid-auto: Vector3d
  auto bad3 = m3x3_1; // eigen-avoid-auto: Matrix3d

  // 2. Expression templates with auto
  auto bad4 = m1 + m2;              // eigen-avoid-auto: expression template
  auto bad5 = m1 * m2;              // eigen-avoid-auto: expression template
  auto bad6 = m1.transpose();       // eigen-avoid-auto: expression template
  auto bad7 = m1.block(0, 0, 2, 2); // eigen-avoid-auto: expression template
  auto bad8 = v1 + v2;              // eigen-avoid-auto: expression template
  auto bad9 = v1.cross(v2);         // eigen-avoid-auto: expression template

  // 3. Array expressions with auto
  auto bad10 = a1 * a2;   // eigen-avoid-auto: coefficient-wise multiply
  auto bad11 = a1.sqrt(); // eigen-avoid-auto: expression template

  // 4. With qualifiers
  const auto bad12 = m1;  // eigen-avoid-auto
  auto &bad13 = m1;       // eigen-avoid-auto
  auto &&bad14 = m1 + m2; // eigen-avoid-auto

  // 5. decltype(auto)
  decltype(auto) bad15 = m1;        // eigen-avoid-auto (if BanDecltypeAuto=true)
  decltype(auto) bad16 = (m1 + m2); // eigen-avoid-auto

  // 6. Geometry module types
  Quaterniond q1(1, 0, 0, 0);
  Quaterniond q2(0, 1, 0, 0);
  auto bad17 = q1;      // eigen-avoid-auto: Quaterniond
  auto bad18 = q1 * q2; // eigen-avoid-auto: expression

  AngleAxisd aa(M_PI / 4, Vector3d::UnitZ());
  auto bad19 = aa; // eigen-avoid-auto: AngleAxisd

  // 7. Using .eval() with auto (still bad - must use explicit type)
  auto bad20 = m1.eval();           // eigen-avoid-auto: still returns Eigen type
  auto bad21 = (m1 + m2).eval();    // eigen-avoid-auto: still returns Eigen type
  auto bad22 = v1.cross(v2).eval(); // eigen-avoid-auto: still returns Eigen type

  // 8. Range-based for loops with auto (now also bad - no longer allowed)
  std::vector<MatrixXd> matrices = {m1, m2};
  for (auto &mat : matrices) { // eigen-avoid-auto: even in range-based for
    (void)mat;
  }
  for (const auto &mat : matrices) { // eigen-avoid-auto: even with const
    (void)mat;
  }

  // ===== Should be OK =====

  // 1. Explicit type declarations
  MatrixXd good1 = m1;
  MatrixXd good2 = m1 + m2;
  Vector3d good3 = v1 + v2;
  Matrix3d good4 = m3x3_1 * m3x3_2;
  ArrayXXd good5 = a1 * a2;

  // 2. Using .eval() with explicit type
  MatrixXd good6 = m1.eval();           // OK: explicit type
  MatrixXd good7 = (m1 + m2).eval();    // OK: explicit type
  Vector3d good8 = v1.cross(v2).eval(); // OK: explicit type

  // 3. Non-Eigen types
  auto good9 = 42;       // OK: int
  auto good10 = 3.14;    // OK: double
  auto good11 = "hello"; // OK: const char*

  // 4. Geometry types with explicit declaration
  Quaterniond good12 = q1 * q2; // OK: explicit type
  AngleAxisd good13 = aa;       // OK: explicit type

  // 5. Range-based for loops with explicit type
  for (MatrixXd &mat : matrices) { // OK: explicit type
    (void)mat;
  }
  for (const MatrixXd &mat : matrices) { // OK: explicit type
    (void)mat;
  }

  // 6. Using results (to avoid unused variable warnings)
  (void)bad1;
  (void)bad2;
  (void)bad3;
  (void)bad4;
  (void)bad5;
  (void)bad6;
  (void)bad7;
  (void)bad8;
  (void)bad9;
  (void)bad10;
  (void)bad11;
  (void)bad12;
  (void)bad13;
  (void)bad14;
  (void)bad15;
  (void)bad16;
  (void)bad17;
  (void)bad18;
  (void)bad19;
  (void)bad20;
  (void)bad21;
  (void)bad22;
  (void)good1;
  (void)good2;
  (void)good3;
  (void)good4;
  (void)good5;
  (void)good6;
  (void)good7;
  (void)good8;
  (void)good9;
  (void)good10;
  (void)good11;
  (void)good12;
  (void)good13;
}

#ifdef TEST_MAIN
int main() {
  test_eigen();
  return 0;
}
#endif
