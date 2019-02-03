
#ifndef _VEC3D_H
#define _VEC3D_H

namespace pcv
{
	struct vector3
	{
		float v[3];
		vector3() { v[0] = v[1] = v[2] = 0.0f; }

		vector3(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }

		float operator[] (int i) const { return v[i]; }

		vector3 operator - (const vector3 &other)
		{
			return vector3(v[0] - other[0], v[1] - other[1], v[2] - other[2]);
		}

		vector3 operator + (const vector3 &other)
		{
			return vector3(v[0] + other[0], v[1] + other[1], v[2] + other[2]);
		}

		vector3 operator * (float prod)
		{
			return vector3(v[0] * prod, v[1] * prod, v[2] * prod);
		}

		// dot product
		float operator * (vector3 &a)
		{
			return (v[0] * a[0] + v[1] * a[1] + v[2] * a[2]);
		}

		// Cross product
		vector3 operator ^ (const vector3 &a)
		{
			return vector3(a[1] * v[2] - a[2] * v[1], a[2] * v[0] - a[0] * v[2], a[0] * v[1] - a[1] * v[0]);
		}

		// Rotate vector P around axis defined by starting point S and unit vector A ( direction).
		static vector3 rotate(vector3 &P, vector3 &S, vector3 &A, float cos_angle, float sin_angle)
		{
			vector3 D = P - S;
			vector3 N0 = A * (D * A);
			vector3 N1 = D - N0;
			vector3 N2 = N1 ^ A;
			return vector3(S + N0 + N1 * cos_angle + N2 * sin_angle);
		}

	};
}

#endif