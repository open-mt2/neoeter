#include <support/TestAssert.h>

#include <Neoeter/Render/StdAfx.h>
#include <Neoeter/Render/GrpMath.h>
#include <Neoeter/Render/Ray.h>

#include <cmath>

namespace
{

	constexpr float kEpsilon = 1e-4f;

	bool closeTo(float a, float b, float eps = kEpsilon)
	{
		return std::fabs(a - b) <= eps;
	}

	bool closeTo(const D3DXVECTOR3& a, const D3DXVECTOR3& b, float eps = kEpsilon)
	{
		return closeTo(a.x, b.x, eps) && closeTo(a.y, b.y, eps) && closeTo(a.z, b.z, eps);
	}

	bool closeTo(const D3DXQUATERNION& a, const D3DXQUATERNION& b, float eps = kEpsilon)
	{
		return closeTo(a.x, b.x, eps) && closeTo(a.y, b.y, eps) && closeTo(a.z, b.z, eps) &&
			   closeTo(a.w, b.w, eps);
	}

	void test_cross_product_2d()
	{
		TEST_SECTION("CrossProduct2D");

		TEST_ASSERT(closeTo(CrossProduct2D(1.0f, 0.0f, 0.0f, 1.0f), 1.0f));
		TEST_ASSERT(closeTo(CrossProduct2D(0.0f, 1.0f, 1.0f, 0.0f), -1.0f));
		TEST_ASSERT(closeTo(CrossProduct2D(2.0f, 4.0f, 1.0f, 2.0f), 0.0f));

		TEST_PASS("signed area and collinearity");
	}

	void test_point_in_triangle_2d()
	{
		TEST_SECTION("IsInTriangle2D");

		const float ax = 0.0f, ay = 0.0f;
		const float bx = 4.0f, by = 0.0f;
		const float cx = 0.0f, cy = 4.0f;

		TEST_ASSERT(IsInTriangle2D(ax, ay, bx, by, cx, cy, 1.0f, 1.0f));
		TEST_ASSERT(!IsInTriangle2D(ax, ay, bx, by, cx, cy, 3.0f, 3.0f));
		TEST_ASSERT(!IsInTriangle2D(ax, ay, bx, by, cx, cy, -1.0f, 1.0f));

		// Vertices and edge midpoints are inside (the half-open question does not
		// apply: the original renderer treats the boundary as covered).
		TEST_ASSERT(IsInTriangle2D(ax, ay, bx, by, cx, cy, 0.0f, 0.0f));
		TEST_ASSERT(IsInTriangle2D(ax, ay, bx, by, cx, cy, 4.0f, 0.0f));
		TEST_ASSERT(IsInTriangle2D(ax, ay, bx, by, cx, cy, 0.0f, 4.0f));
		TEST_ASSERT(IsInTriangle2D(ax, ay, bx, by, cx, cy, 2.0f, 0.0f));
		TEST_ASSERT(IsInTriangle2D(ax, ay, bx, by, cx, cy, 2.0f, 2.0f));
		TEST_ASSERT(IsInTriangle2D(ax, ay, bx, by, cx, cy, 0.0f, 2.0f));

		// Points on the supporting lines but outside the triangle are rejected.
		TEST_ASSERT(!IsInTriangle2D(ax, ay, bx, by, cx, cy, 5.0f, 0.0f));
		TEST_ASSERT(!IsInTriangle2D(ax, ay, bx, by, cx, cy, 0.0f, -1.0f));

		TEST_PASS("interior, boundary and line-extension cases");
	}

	void test_clamp_degree()
	{
		TEST_SECTION("ClampDegree");

		TEST_ASSERT(closeTo(ClampDegree(0.0f), 0.0f));
		TEST_ASSERT(closeTo(ClampDegree(359.5f), 359.5f));
		TEST_ASSERT(closeTo(ClampDegree(360.0f), 0.0f));
		TEST_ASSERT(closeTo(ClampDegree(400.0f), 40.0f));
		TEST_ASSERT(closeTo(ClampDegree(-1.0f), 359.0f));
		TEST_ASSERT(closeTo(ClampDegree(-30.0f), 330.0f));

		TEST_PASS("one-wrap normalisation matches the movement code");
	}

	void test_vector_and_quaternion_blend()
	{
		TEST_SECTION("blend helpers");

		const D3DXVECTOR3 a(1.0f, 2.0f, 3.0f);
		const D3DXVECTOR3 b(5.0f, 6.0f, 7.0f);
		D3DXVECTOR3 out;

		D3DXVec3Blend(&out, &a, &b, 0.0f);
		TEST_ASSERT(closeTo(out, a));
		D3DXVec3Blend(&out, &a, &b, 1.0f);
		TEST_ASSERT(closeTo(out, b));
		D3DXVec3Blend(&out, &a, &b, 0.25f);
		TEST_ASSERT(closeTo(out, D3DXVECTOR3(2.0f, 3.0f, 4.0f)));

		const D3DXQUATERNION qa(0.0f, 0.0f, 0.0f, 1.0f);
		const D3DXQUATERNION qb(0.0f, 0.0f, 1.0f, 0.0f);
		D3DXQUATERNION qout;
		D3DXQuaternionBlend(&qout, &qa, &qb, 0.5f);
		TEST_ASSERT(closeTo(qout, D3DXQUATERNION(0.0f, 0.0f, 0.5f, 0.5f)));

		TEST_PASS("linear endpoints and quarter point");
	}

	void test_vector_distance()
	{
		TEST_SECTION("GetVector3Distance");

		// The name says distance but the implementation returns the squared XZ
		// distance: z is ignored and no square root is taken.
		const D3DXVECTOR3 a(1.0f, 2.0f, 3.0f);
		const D3DXVECTOR3 b(4.0f, 6.0f, 50.0f);
		TEST_ASSERT(closeTo(GetVector3Distance(a, b), 25.0f));
		TEST_ASSERT(closeTo(GetVector3Distance(a, a), 0.0f));

		TEST_PASS("squared planar distance, z independent");
	}

	void test_segment_point_distance()
	{
		TEST_SECTION("square_distance_between_linesegment_and_point");

		const D3DXVECTOR3 p1(0.0f, 0.0f, 0.0f);
		const D3DXVECTOR3 p2(10.0f, 0.0f, 0.0f);

		TEST_ASSERT(closeTo(square_distance_between_linesegment_and_point(p1, p2, D3DXVECTOR3(5.0f, 3.0f, 0.0f)), 9.0f));
		TEST_ASSERT(closeTo(square_distance_between_linesegment_and_point(p1, p2, D3DXVECTOR3(-2.0f, 0.0f, 0.0f)), 4.0f));
		TEST_ASSERT(closeTo(square_distance_between_linesegment_and_point(p1, p2, D3DXVECTOR3(12.0f, 0.0f, 0.0f)), 4.0f));
		TEST_ASSERT(closeTo(square_distance_between_linesegment_and_point(p1, p2, D3DXVECTOR3(5.0f, 0.0f, 0.0f)), 0.0f));
		TEST_ASSERT(closeTo(square_distance_between_linesegment_and_point(p1, p2, D3DXVECTOR3(5.0f, 0.0f, 7.0f)), 49.0f));

		// Degenerate segment collapses to point distance.
		TEST_ASSERT(closeTo(square_distance_between_linesegment_and_point(p1, p1, D3DXVECTOR3(3.0f, 4.0f, 0.0f)), 25.0f));

		TEST_PASS("endpoint clamp, perpendicular and degenerate segment");
	}

	void test_quaternion_rotation_helpers()
	{
		TEST_SECTION("quaternion rotation helpers");

		const float half = D3DX_PI / 4.0f; // 45 degrees
		const D3DXQUATERNION qz(0.0f, 0.0f, std::sin(half), std::cos(half));

		const D3DXVECTOR3 x(1.0f, 0.0f, 0.0f);
		D3DXVECTOR3 direct;
		Vec3TransformQuaternion(&direct, &x, &qz);
		TEST_ASSERT(closeTo(direct, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 1e-4f));

		// D3DXVec3Rotation composes in the opposite order (q^-1 v q) and is unused
		// by the client; only the identity case is pinned here.
		D3DXVECTOR3 viaD3dx;
		D3DXVec3Rotation(&viaD3dx, &x, &D3DXQUATERNION(0.0f, 0.0f, 0.0f, 1.0f));
		TEST_ASSERT(closeTo(viaD3dx, x));

		// SafeRotationNormalizedArc handles the parallel and anti-parallel cases
		// that make the plain arc formula divide by zero.
		const D3DXQUATERNION same = SafeRotationNormalizedArc(x, x);
		TEST_ASSERT(closeTo(same, D3DXQUATERNION(0.0f, 0.0f, 0.0f, 1.0f)));

		const D3DXQUATERNION opposite = SafeRotationNormalizedArc(x, D3DXVECTOR3(-1.0f, 0.0f, 0.0f));
		TEST_ASSERT(closeTo(opposite, D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f)));

		D3DXVECTOR3 flipped;
		Vec3TransformQuaternionSafe(&flipped, &x, &opposite);
		TEST_ASSERT(closeTo(flipped, D3DXVECTOR3(-1.0f, 0.0f, 0.0f)));

		TEST_PASS("90 degree rotation, identity and 180 degree arcs");
	}

	void test_ray()
	{
		TEST_SECTION("CRay");

		CRay ray(D3DXVECTOR3(1.0f, 2.0f, 3.0f), D3DXVECTOR3(0.0f, 0.0f, 2.0f), 10.0f);

		D3DXVECTOR3 dir;
		float range = 0.0f;
		ray.GetDirection(&dir, &range);
		TEST_ASSERT(closeTo(dir, D3DXVECTOR3(0.0f, 0.0f, 1.0f)));
		TEST_ASSERT(closeTo(range, 10.0f));

		D3DXVECTOR3 end;
		ray.GetEndPoint(&end);
		TEST_ASSERT(closeTo(end, D3DXVECTOR3(1.0f, 2.0f, 13.0f)));

		CRay copy;
		copy = ray;
		D3DXVECTOR3 start;
		copy.GetStartPoint(&start);
		TEST_ASSERT(closeTo(start, D3DXVECTOR3(1.0f, 2.0f, 3.0f)));

		copy.SetDirection(D3DXVECTOR3(3.0f, 4.0f, 0.0f), 5.0f);
		copy.GetDirection(&dir, &range);
		TEST_ASSERT(closeTo(dir, D3DXVECTOR3(0.6f, 0.8f, 0.0f)));
		copy.GetEndPoint(&end);
		TEST_ASSERT(closeTo(end, D3DXVECTOR3(4.0f, 6.0f, 3.0f)));

		TEST_PASS("direction normalisation and endpoint recomputation");
	}

} // namespace

int main()
{
	test_cross_product_2d();
	test_point_in_triangle_2d();
	test_clamp_degree();
	test_vector_and_quaternion_blend();
	test_vector_distance();
	test_segment_point_distance();
	test_quaternion_rotation_helpers();
	test_ray();

	std::printf("[test_grpmath] all checks passed\n");
	return 0;
}
