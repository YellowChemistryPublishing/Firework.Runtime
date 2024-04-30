#pragma once

#include <cmath>
#include <cstdint>
#include <ostream>
#include <string>

namespace Firework
{
	namespace Mathematics
	{
		struct Vector2Int;
		
		struct Vector2
		{
			static const Vector2 zero;
			static const Vector2 one;

			float x = 0.0f, y = 0.0f;

			constexpr Vector2() noexcept = default;
			constexpr Vector2(float init) noexcept : x(init), y(init)
			{ }
			constexpr Vector2(float x, float y) noexcept : x(x), y(y)
			{ }

			constexpr bool operator==(const Vector2&) const noexcept = default;

			constexpr Vector2 operator+() const noexcept
			{
				return *this;
			}
			constexpr Vector2 operator-() const noexcept
			{
				return Vector2(-this->x, -this->y);
			}
			
			constexpr Vector2 operator+(const Vector2& other) const noexcept
			{
				return Vector2(this->x + other.x, this->y + other.y);
			}
			constexpr Vector2 operator-(const Vector2& other) const noexcept
			{
				return Vector2(this->x - other.x, this->y - other.y);
			}
			constexpr Vector2 operator*(const Vector2& other) const noexcept
			{
				return Vector2(this->x * other.x, this->y * other.y);
			}
			constexpr Vector2 operator*(float other) const noexcept
			{
				return Vector2(this->x * other, this->y * other);
			}
			friend constexpr Vector2 operator*(float lhs, const Vector2& rhs) noexcept
			{
				return Vector2(lhs * rhs.x, lhs * rhs.y);
			}
			constexpr Vector2 operator/(const Vector2& other) const noexcept
			{
				return Vector2(this->x / other.x, this->y / other.y);
			}
			constexpr Vector2 operator/(float other) const noexcept
			{
				return Vector2(this->x / other, this->y / other);
			}
			constexpr Vector2& operator+=(const Vector2& other) noexcept
			{
				return (*this = *this + other);
			}
			constexpr Vector2& operator-=(const Vector2& other) noexcept
			{
				return (*this = *this - other);
			}
			constexpr Vector2& operator*=(const Vector2& other) noexcept
			{
				return (*this = *this * other);
			}
			constexpr Vector2& operator*=(float other) noexcept
			{
				return (*this = *this * other);
			}
			constexpr Vector2& operator/=(const Vector2& other) noexcept
			{
				return (*this = *this / other);
			}
			constexpr Vector2& operator/=(float other) noexcept
			{
				return (*this = *this / other);
			}

			constexpr explicit operator Vector2Int () const noexcept;
			inline operator std::string () const
			{
				return std::string("(").append(std::to_string(this->x)).append(", ").append(std::to_string(this->y)).append(")");
			}
			inline friend std::ostream& operator<<(std::ostream& lhs, const Vector2& rhs)
			{
				return lhs << static_cast<std::string>(rhs);
			}
		};

		inline constexpr Vector2 Vector2::zero(0.0f, 0.0f);
		inline constexpr Vector2 Vector2::one(1.0f, 1.0f);
		
		struct Vector2Int
		{
			static const Vector2Int zero;
			static const Vector2Int one;

			int32_t x = 0, y = 0;

			constexpr Vector2Int() noexcept = default;
			constexpr Vector2Int(int32_t init) noexcept : x(init), y(init)
			{ }
			constexpr Vector2Int(int32_t x, int32_t y) noexcept : x(x), y(y)
			{ }

			constexpr bool operator==(const Vector2Int& other) const noexcept = default;

			constexpr Vector2Int operator+() const noexcept
			{
				return *this;
			}
			constexpr Vector2Int operator-() const noexcept
			{
				return Vector2Int(-this->x, -this->y);
			}

			constexpr Vector2Int operator+(const Vector2Int& other) const noexcept
			{
				return Vector2Int(this->x + other.x, this->y + other.y);
			}
			constexpr Vector2Int operator-(const Vector2Int& other) const noexcept
			{
				return Vector2Int(this->x - other.x, this->y - other.y);
			}
			constexpr Vector2Int operator*(const Vector2Int& other) const noexcept
			{
				return Vector2Int(this->x * other.x, this->y * other.y);
			}
			constexpr Vector2Int operator*(int32_t other) const noexcept
			{
				return Vector2Int(this->x * other, this->y * other);
			}
			friend constexpr Vector2Int operator*(int32_t lhs, const Vector2Int& rhs) noexcept
			{
				return Vector2Int(lhs * rhs.x, lhs * rhs.y);
			}
			constexpr Vector2Int operator/(const Vector2Int& other) const noexcept
			{
				return Vector2Int(this->x / other.x, this->y / other.y);
			}
			constexpr Vector2Int operator/(int32_t other) const noexcept
			{
				return Vector2Int(this->x / other, this->y / other);
			}
			
			constexpr Vector2Int& operator+=(const Vector2Int& other) noexcept
			{
				return (*this = *this + other);
			}
			constexpr Vector2Int& operator-=(const Vector2Int& other) noexcept
			{
				return (*this = *this - other);
			}
			constexpr Vector2Int& operator*=(const Vector2Int& other) noexcept
			{
				return (*this = *this * other);
			}
			constexpr Vector2Int& operator*=(int32_t other) noexcept
			{
				return (*this = *this * other);
			}
			constexpr Vector2Int& operator/=(const Vector2Int& other) noexcept
			{
				return (*this = *this / other);
			}
			constexpr Vector2Int& operator/=(int32_t other) noexcept
			{
				return (*this = *this / other);
			}

			constexpr explicit operator Vector2 () const noexcept;
			inline operator std::string () const noexcept
			{
				return std::string("(").append(std::to_string(this->x)).append(", ").append(std::to_string(this->y)).append(")");
			}
			inline friend std::ostream& operator<<(std::ostream& lhs, const Vector2Int& rhs)
			{
				return lhs << static_cast<std::string>(rhs);
			}
		};

		inline constexpr Vector2Int Vector2Int::zero(0, 0);
		inline constexpr Vector2Int Vector2Int::one(1, 1);

		constexpr Vector2::operator Vector2Int() const noexcept
		{
			return Vector2Int(int32_t(this->x + 0.5f), int32_t(this->y + 0.5f));
		}
		constexpr Vector2Int::operator Vector2() const noexcept
		{
			return Vector2((float)this->x, (float)this->y);
		}

		struct Quaternion;
		struct Vector3
		{
			static const Vector3 zero;
			static const Vector3 one;

			static const Vector3 forward;
			static const Vector3 right;
			static const Vector3 up;

			float x = 0.0f, y = 0.0f, z = 0.0f;

			constexpr Vector3() noexcept = default;
			constexpr Vector3(float init) noexcept : x(init), y(init), z(init)
			{ }
			constexpr Vector3(float x, float y, float z) noexcept : x(x), y(y), z(z)
			{ }

			constexpr bool operator==(const Vector3&) const noexcept = default;

			constexpr Vector3 operator+() const noexcept
			{
				return *this;
			}
			constexpr Vector3 operator-() const noexcept
			{
				return Vector3(-this->x, -this->y, -this->z);
			}

			constexpr Vector3 operator+(const Vector3& other) const noexcept
			{
				return Vector3(this->x + other.x, this->y + other.y, this->z + other.z);
			}
			constexpr Vector3 operator-(const Vector3& other) const noexcept
			{
				return Vector3(this->x - other.x, this->y - other.y, this->z - other.z);
			}
			constexpr Vector3 operator*(const Vector3& other) const noexcept
			{
				return Vector3(this->x * other.x, this->y * other.y, this->z * other.z);
			}
			constexpr Vector3 operator/(const Vector3& other) const noexcept
			{
				return Vector3(this->x / other.x, this->y / other.y, this->z / other.z);
			}
			constexpr Vector3 operator*(float other) const noexcept
			{
				return Vector3(this->x * other, this->y * other, this->z * other);
			}
			constexpr Vector3 operator/(float other) const noexcept
			{
				return Vector3(this->x / other, this->y / other, this->z / other);
			}
			friend constexpr Vector3 operator*(float lhs, const Vector3& rhs)
			{
				return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
			}

			constexpr Vector3& operator+=(const Vector3& other) noexcept
			{
				return (*this = *this + other);
			}
			constexpr Vector3& operator-=(const Vector3& other) noexcept
			{
				return (*this = *this - other);
			}
			constexpr Vector3& operator*=(const Vector3& other) noexcept
			{
				return (*this = *this * other);
			}
			constexpr Vector3& operator/=(const Vector3& other) noexcept
			{
				return (*this = *this / other);
			}

			constexpr Vector3 rotate(const Quaternion& rot) const noexcept;

			inline operator std::string () const
			{
				std::string ret; ret.push_back('(');
				ret
				.append(std::to_string(this->x))
				.append(", ")
				.append(std::to_string(this->y))
				.append(", ")
				.append(std::to_string(this->z))
				.push_back(')');
				return ret;
			}
		};

		inline constexpr Vector3 Vector3::zero(0.0f, 0.0f, 0.0f);
		inline constexpr Vector3 Vector3::one(1.0f, 1.0f, 1.0f);
		inline constexpr Vector3 Vector3::forward(0.0f, 0.0f, 1.0f);
		inline constexpr Vector3 Vector3::right(1.0f, 0.0f, 0.0f);
		inline constexpr Vector3 Vector3::up(0.0f, 1.0f, 0.0f);

		struct Vector4
		{
			float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

			constexpr Vector4() noexcept = default;
			constexpr Vector4(float init) noexcept : x(init), y(init), z(init), w(init)
			{ }
			constexpr Vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w)
			{ }
		};

		struct Quaternion
		{
			static const Quaternion identity;

			inline static Quaternion fromEuler(Vector3 r) noexcept
			{
				// Shamelessly "inspired" by the glm source for their Quaternions.
				// Modified, see: https://github.com/g-truc/glm, under MIT.
				Vector3 c(cosf(r.x * 0.5f), cosf(r.y * 0.5f), cosf(r.z * 0.5f));
				Vector3 s(sinf(r.x * 0.5f), sinf(r.y * 0.5f), sinf(r.z * 0.5f));

				return Quaternion
				{
					c.x * s.y * c.z + s.x * c.y * s.z,
					s.x * c.y * c.z - c.x * s.y * s.z,
					c.x * c.y * s.z - s.x * s.y * c.z,
					c.x * c.y * c.z + s.x * s.y * s.z
				};
			}
			
			float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;

			constexpr Quaternion() noexcept = default;
			constexpr Quaternion(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w)
			{ }
			constexpr Quaternion(float real, Vector3 right) noexcept : x(right.x), y(right.y), z(right.z), w(real)
			{ }
			constexpr Quaternion(const Quaternion&) noexcept = default;

			constexpr Quaternion operator*(float other) const noexcept
			{
				return Quaternion(this->x * other, this->y * other, this->z * other, this->w * other);
			}
			constexpr Quaternion operator/(float other) const noexcept
			{
				return Quaternion(this->x / other, this->y / other, this->z / other, this->w / other);
			}
			constexpr Quaternion operator*(const Quaternion& other) const noexcept
			{
				return Quaternion
				{
					this->w * other.x + this->x * other.w + this->y * other.z - this->z * other.y,
					this->w * other.y - this->x * other.z + this->y * other.w + this->z * other.x,
					this->w * other.z + this->x * other.y - this->y * other.x + this->z * other.w,
					this->w * this->x - this->x * other.x - this->y * other.y - this->z * other.z
				};
			}
			constexpr Quaternion& operator*=(float other) noexcept
			{
				this->x *= other;
				this->y *= other;
				this->z *= other;
				this->w *= other;
				return *this;
			}
			friend constexpr Quaternion operator*(float lhs, const Quaternion& rhs) noexcept
			{
				return Quaternion(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
			}
			constexpr Quaternion& operator*=(const Quaternion& other) noexcept
			{
				*this = *this * other;
				return *this;
			}
			constexpr Quaternion& operator/=(float other) noexcept
			{
				this->x /= other;
				this->y /= other;
				this->z /= other;
				this->w /= other;
				return *this;
			}

			constexpr bool isScalar() const noexcept
			{
				return this->x == 0.0f && this->y == 0.0f && this->z == 0.0f;
			}
			constexpr bool isVectorQuaternion() const noexcept
			{
				return this->w == 0.0f;
			}

			constexpr float scalar() const noexcept
			{
				return this->w;
			}
			constexpr Vector3 vector() const noexcept
			{
				return Vector3(this->x, this->y, this->z);
			}

			constexpr Quaternion inverse() const noexcept
			{
				// Vector3 v = this->vector();
				// float d = (this->w * this->w + ::dot(v, v));
				// return Quaternion(this->w / d, -v / d);
				// I don't know if the below is better but we're keeping both just in case.
				return this->conjugate() / this->norm2();
			}
			constexpr Quaternion conjugate() const noexcept
			{
				return Quaternion(-this->x, -this->y, -this->z, this->w);
			}
			constexpr float norm2() const noexcept
			{
				return this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;
			}
			inline float norm() const noexcept
			{
				return sqrtf(this->norm2());
			}
			inline Quaternion unit()
			{
				return *this / this->norm();
			}
			inline Quaternion normalized()
			{
				return this->unit();
			}

			inline operator std::string () const
			{
				std::string ret; ret.push_back('[');
				ret
				.append(std::to_string(this->w))
				.append(", (")
				.append(std::to_string(this->x))
				.append(", ")
				.append(std::to_string(this->y))
				.append(", ")
				.append(std::to_string(this->z))
				.append(")]");
				return ret;
			}
		};

		inline constexpr Quaternion Quaternion::identity(0.0f, 0.0f, 0.0f, 1.0f);

		constexpr Vector3 Vector3::rotate(const Quaternion& rot) const noexcept
		{
			/// FIXME: Is this a hack? Why do we need operator- in front?
			return -(rot * Quaternion(0.0f, *this) * rot.conjugate()).vector();
		}

		struct Matrix4x4
		{
			constexpr static Matrix4x4 translate(const Vector3& v) noexcept
			{
				Matrix4x4 ret;
				ret[{ 0, 3 }] = v.x;
				ret[{ 1, 3 }] = v.y;
				ret[{ 2, 3 }] = v.z;
				return ret;
			}
			constexpr static Matrix4x4 scale(const Vector3& v) noexcept
			{
				Matrix4x4 ret;
				ret.data[0][0] = v.x;
				ret.data[1][1] = v.y;
				ret.data[2][2] = v.z;
				return ret;
			}
			inline static Matrix4x4 rotate(const Vector3& v) noexcept // I don't know if this works, don't use it.
			{
				Matrix4x4 rx, ry, rz;

				rx.data[1][1] = cosf(v.x);
				rx.data[2][1] = -sinf(v.x);
				rx.data[1][2] = sinf(v.x);
				rx.data[2][2] = cosf(v.x);
				
				ry.data[0][0] = cosf(v.x);
				ry.data[2][0] = sinf(v.x);
				ry.data[0][2] = -sinf(v.x);
				ry.data[2][2] = cosf(v.x);
				
				rz.data[0][0] = cosf(v.x);
				rz.data[1][0] = -sinf(v.x);
				rz.data[0][1] = sinf(v.x);
				rz.data[1][1] = cosf(v.x);

				return rx * ry * rz;
			}
			constexpr static Matrix4x4 rotate(const Quaternion& rot) noexcept
			{
				Matrix4x4 ret;

				float qxx(rot.x * rot.x);
				float qyy(rot.y * rot.y);
				float qzz(rot.z * rot.z);
				float qxz(rot.x * rot.z);
				float qxy(rot.x * rot.y);
				float qyz(rot.y * rot.z);
				float qwx(rot.w * rot.x);
				float qwy(rot.w * rot.y);
				float qwz(rot.w * rot.z);

				ret[{ 0, 0 }] = 1.0f - 2.0f * (qyy +  qzz);
				ret[{ 0, 1 }] = 2.0f * (qxy + qwz);
				ret[{ 0, 2 }] = 2.0f * (qxz - qwy);

				ret[{ 1, 0 }] = 2.0f * (qxy - qwz);
				ret[{ 1, 1 }] = 1.0f - 2.0f * (qxx +  qzz);
				ret[{ 1, 2 }] = 2.0f * (qyz + qwx);

				ret[{ 2, 0 }] = 2.0f * (qxz + qwy);
				ret[{ 2, 1 }] = 2.0f * (qyz - qwx);
				ret[{ 2, 2 }] = 1.0f - 2.0f * (qxx +  qyy);

				return ret;
			}

			float data[4][4]
			{
				{ 1.0f, 0.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f, 1.0f }
			};
			
			constexpr Matrix4x4() noexcept = default;
			constexpr Matrix4x4(float init) noexcept :
			data
			{
				{ init, init, init, init },
				{ init, init, init, init },
				{ init, init, init, init },
				{ init, init, init, init }
			}
			{ }
			constexpr Matrix4x4(const float (&matrixData)[16]) noexcept
			{
				for (size_t i = 0; i < 4; i++)
					for (size_t j = 0; j < 4; j++)
						this->data[i][j] = matrixData[j * 4 + i];
			}
			constexpr Matrix4x4(const Matrix4x4&) noexcept = default;

			// constexpr float& operator[](size_t row, size_t column) noexcept
			// {	
			// 	return this->data[column][row];
			// }
			constexpr float& operator[](const size_t (&rc)[2]) noexcept
			{
				return this->data[rc[1]][rc[0]];
			}
			constexpr const float& operator[](const size_t (&rc)[2]) const noexcept
			{
				return this->data[rc[1]][rc[0]];
			}
			
			constexpr Matrix4x4 operator+(const Matrix4x4& other) const noexcept
			{
				Matrix4x4 ret(*this);
				for (size_t i = 0; i < 4; i++)
					for (size_t j = 0; j < 4; j++)
						ret.data[i][j] = this->data[i][j] + other.data[i][j];
				return ret;
			}
			constexpr Matrix4x4 operator-(const Matrix4x4& other) const noexcept
			{
				Matrix4x4 ret(*this);
				for (size_t i = 0; i < 4; i++)
					for (size_t j = 0; j < 4; j++)
						ret.data[i][j] = this->data[i][j] - other.data[i][j];
				return ret;
			}
			constexpr Matrix4x4 operator*(const Matrix4x4& other) const noexcept
			{
				Matrix4x4 ret(0.0f);
				for (size_t i = 0; i < 4; i++)
					for (size_t j = 0; j < 4; j++)
						for (size_t k = 0; k < 4; k++)
							ret[{ i, j }] += (*this)[{ i, k }] * other[{ k, j }];
				return ret;
			}
			constexpr Vector4 operator*(const Vector4& other) const noexcept
			{
				return Vector4
				{
					this->data[0][0] * other.x + this->data[1][0] * other.y + this->data[2][0] * other.z + this->data[3][0] * other.w,
					this->data[0][1] * other.x + this->data[1][1] * other.y + this->data[2][1] * other.z + this->data[3][1] * other.w,
					this->data[0][2] * other.x + this->data[1][2] * other.y + this->data[2][2] * other.z + this->data[3][2] * other.w,
					this->data[0][3] * other.x + this->data[1][3] * other.y + this->data[2][3] * other.z + this->data[3][3] * other.w
				};
			}
			constexpr Matrix4x4& operator+=(const Matrix4x4& other) noexcept
			{
				return (*this = *this + other);
			}
			constexpr Matrix4x4& operator-=(const Matrix4x4& other) noexcept
			{
				return (*this = *this - other);
			}
			constexpr Matrix4x4& operator*=(const Matrix4x4& other) noexcept
			{
				return (*this = *this * other);
			}

			constexpr Matrix4x4 transpose()
			{
				Matrix4x4 ret(*this);
				for (size_t i = 0; i < 4; i++)
					for (size_t j = 0; j < 4; j++)
						ret[{ i, j }] = (*this)[{ j, i }];
				return ret;
			}

			inline operator std::string () const
			{
				std::string ret;
				for (size_t i = 0; i < 4; i++)
				{
					ret.push_back('[');
					for (size_t j = 0; j < 4; j++)
					{
						ret
						.append(std::to_string((*this)[{ i, j }]))
						.append(", ");
					}
					ret.pop_back(); ret.pop_back();
					ret.append("]\n");
				}
				ret.pop_back();
				return ret;
			}
		};

		#pragma region Typedefs
		typedef Vector2 vec2;
		typedef Vector2Int vec2i;

		typedef Vector3 vec3;

		typedef Vector4 vec4;

		typedef Quaternion quat;

		typedef Matrix4x4 mtx4;
		#pragma endregion

		class Math
		{
			using Vector2 = Firework::Mathematics::Vector2;
			using Vector2Int = Firework::Mathematics::Vector2Int;
			using Vector3 = Firework::Mathematics::Vector3;
			using Quaternion = Firework::Mathematics::Quaternion;
		public:
			Math() = delete;

			constexpr static long double pi = 3.14159265358979323846264338327950288l;
			constexpr static long double e = 2.71828182845904523536028747135266249l;
			constexpr static long double tau = 6.28318530717958647692528676655900576l;

			inline static float abs(const Vector2& v) noexcept
			{
				return sqrtf(v.x * v.x + v.y + v.y);
			}
			inline static float abs(const Vector2Int& v) noexcept
			{
				return sqrtf((float)v.x * (float)v.x + (float)v.y + (float)v.y);
			}
			inline static float abs(const Vector3& v) noexcept
			{
				return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
			}
			inline static float abs(const Quaternion& q) noexcept
			{
				return q.norm();
			}

			template <typename T>
			constexpr static T clamp(T val, T minVal, T maxVal)
			{
				return val < minVal ? minVal : (val > maxVal ? maxVal : val);
			}

			template <typename T>
			requires std::floating_point<T>
			constexpr static T deg2Rad(T degrees)
			{
				return (T)Math::pi / (T)180.0l * degrees;
			}
			template <typename T>
			requires std::floating_point<T>
			constexpr static T rad2Deg(T radians)
			{
				return radians * (T)180.0l / (T)Math::pi;
			}
			
			constexpr static float dot(const Vector2& lhs, const Vector2& rhs) noexcept
			{
				return lhs.x * rhs.x + lhs.y * rhs.y;
			}
			constexpr static int32_t dot(const Vector2Int& lhs, const Vector2Int& rhs) noexcept
			{
				return lhs.x * rhs.x + lhs.y * rhs.y;
			}
			constexpr static float dot(const Vector3& lhs, const Vector3& rhs) noexcept
			{
				return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
			}
			constexpr static Vector3 cross(const Vector3& lhs, const Vector3& rhs) noexcept
			{
				return Vector3
				{
					rhs.z * lhs.y - rhs.y * lhs.z,
					-rhs.z * lhs.x + rhs.x * lhs.z,
					rhs.y * lhs.x - rhs.x * lhs.y
				};
			}
		};
	}
}
