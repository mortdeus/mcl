#pragma once
/**
	@file
	@brief finite field extension class
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <mcl/fp.hpp>

namespace mcl {

template<class Fp>
struct BnT {
	typedef fp::Unit Unit;
	class FpDbl {
		Unit v_[Fp::maxSize * 2];
	public:
		// QQQ : does not check range of x strictly(use for debug)
		void setMpz(const mpz_class& x)
		{
			if (x < 0) throw cybozu::Exception("FpDbl:_setMpz:negative is not supported") << x;
			const size_t xByte = Gmp::getUnitSize(x) * sizeof(Unit);
			if (xByte >= sizeof(v_)) {
				throw cybozu::Exception("FpDbl:_setMpz:too large") << x;
			}
			memcpy(v_, Gmp::getUnit(x), xByte);
			memset(v_ + xByte / sizeof(Unit), 0, sizeof(v_) - xByte);
		}
		void getMpz(mpz_class& x) const
		{
			Gmp::setArray(x, v_, CYBOZU_NUM_OF_ARRAY(v_));
		}
		static inline void add(FpDbl& z, const FpDbl& x, const FpDbl& y) { Fp::op_.fpDbl_add(z.v_, x.v_, y.v_); }
		static inline void sub(FpDbl& z, const FpDbl& x, const FpDbl& y) { Fp::op_.fpDbl_sub(z.v_, x.v_, y.v_); }
		static inline void addNC(FpDbl& z, const FpDbl& x, const FpDbl& y) { Fp::op_.fpDbl_addNC(z.v_, x.v_, y.v_); }
		static inline void subNC(FpDbl& z, const FpDbl& x, const FpDbl& y) { Fp::op_.fpDbl_subNC(z.v_, x.v_, y.v_); }
	};
	/*
		beta = -1
		Fp2 = F[u] / (u^2 + 1)
		x = a_ + b u
	*/
	class Fp2 {
		static Fp xi_c_;
	public:
		Fp a, b;
		Fp2() { }
		Fp2(int64_t a) : a(a), b(0) { }
		Fp2(const Fp& a, const Fp& b) : a(a), b(b) { }
		Fp2(int64_t a, int64_t b) : a(a), b(b) { }
		Fp2(const std::string& a, const std::string& b, int base = 0) : a(a, base), b(b, base) {}
		Fp* get() { return &a; }
		const Fp* get() const { return &a; }
		void clear()
		{
			a.clear();
			b.clear();
		}
		static inline void add(Fp2& z, const Fp2& x, const Fp2& y) { Fp::op_.fp2_add(z.a.v_, x.a.v_, y.a.v_); }
		static inline void sub(Fp2& z, const Fp2& x, const Fp2& y) { Fp::op_.fp2_sub(z.a.v_, x.a.v_, y.a.v_); }
		static inline void mul(Fp2& z, const Fp2& x, const Fp2& y) { Fp::op_.fp2_mul(z.a.v_, x.a.v_, y.a.v_); }
		static inline void inv(Fp2& y, const Fp2& x) { Fp::op_.fp2_inv(y.a.v_, x.a.v_); }
		static inline void neg(Fp2& y, const Fp2& x) { Fp::op_.fp2_neg(y.a.v_, x.a.v_); }
		static inline void sqr(Fp2& y, const Fp2& x) { Fp::op_.fp2_sqr(y.a.v_, x.a.v_); }
		static inline void mul_xi(Fp2& y, const Fp2& x) { Fp::op_.fp2_mul_xi(y.a.v_, x.a.v_); }
		static inline void div(Fp2& z, const Fp2& x, const Fp2& y)
		{
			Fp2 rev;
			inv(rev, y);
			mul(z, x, rev);
		}
		inline friend Fp2 operator+(const Fp2& x, const Fp2& y) { Fp2 z; add(z, x, y); return z; }
		inline friend Fp2 operator-(const Fp2& x, const Fp2& y) { Fp2 z; sub(z, x, y); return z; }
		inline friend Fp2 operator*(const Fp2& x, const Fp2& y) { Fp2 z; mul(z, x, y); return z; }
		inline friend Fp2 operator/(const Fp2& x, const Fp2& y) { Fp2 z; div(z, x, y); return z; }
		Fp2& operator+=(const Fp2& x) { add(*this, *this, x); return *this; }
		Fp2& operator-=(const Fp2& x) { sub(*this, *this, x); return *this; }
		Fp2& operator*=(const Fp2& x) { mul(*this, *this, x); return *this; }
		Fp2& operator/=(const Fp2& x) { div(*this, *this, x); return *this; }
		Fp2 operator-() const { Fp2 x; neg(x, *this); return x; }
		/*
			Fp2 = '[' + <a> + ',' + <b> + ']'
		*/
		void getStr(std::string& str, int base = 10, bool withPrefix = false) const
		{
			str = '[';
			str += a.getStr(base, withPrefix);
			str += ',';
			str += b.getStr(base, withPrefix);
			str += ']';
		}
		void setStr(const std::string& str, int base = 0)
		{
			const size_t size = str.size();
			const size_t pos = str.find(',');
			if (size >= 5 && str[0] == '[' && pos != std::string::npos && str[size - 1] == ']') {
				a.setStr(str.substr(1, pos - 1), base);
				b.setStr(str.substr(pos + 1, size - pos - 2), base);
				return;
			}
			throw cybozu::Exception("Fp2:setStr:bad format") << str;
		}
		std::string getStr(int base = 10, bool withPrefix = false) const
		{
			std::string str;
			getStr(str, base, withPrefix);
			return str;
		}
		friend inline std::ostream& operator<<(std::ostream& os, const Fp2& self)
		{
			const std::ios_base::fmtflags f = os.flags();
			if (f & std::ios_base::oct) throw cybozu::Exception("Fp2:operator<<:oct is not supported");
			const int base = (f & std::ios_base::hex) ? 16 : 10;
			const bool showBase = (f & std::ios_base::showbase) != 0;
			return os << self.getStr(base, showBase);
		}
		friend inline std::istream& operator>>(std::istream& is, Fp2& self)
		{
			const std::ios_base::fmtflags f = is.flags();
			if (f & std::ios_base::oct) throw cybozu::Exception("fpT:operator>>:oct is not supported");
			const int base = (f & std::ios_base::hex) ? 16 : 0;
			std::string str;
			is >> str;
			self.setStr(str, base);
			return is;
		}
		bool isZero() const { return a.isZero() && b.isZero(); }
		bool operator==(const Fp2& rhs) const { return a == rhs.a && b == rhs.b; }
		bool operator!=(const Fp2& rhs) const { return !operator==(rhs); }
		void normalize() {} // dummy method
		template<class tag2, size_t maxBitSize2>
		static inline void power(Fp2& z, const Fp2& x, const FpT<tag2, maxBitSize2>& y)
		{
			fp::Block b;
			y.getBlock(b);
			powerArray(z, x, b.p, b.n, false);
		}
		static inline void power(Fp2& z, const Fp2& x, int y)
		{
			const fp::Unit u = abs(y);
			powerArray(z, x, &u, 1, y < 0);
		}
		static inline void power(Fp2& z, const Fp2& x, const mpz_class& y)
		{
			powerArray(z, x, Gmp::getUnit(y), abs(y.get_mpz_t()->_mp_size), y < 0);
		}
		static inline void init(int xi_c)
		{
			assert(Fp::maxSize <= 256);
			xi_c_ = xi_c;
			Fp::op_.fp2_add = fp2_addW;
			Fp::op_.fp2_sub = fp2_subW;
			Fp::op_.fp2_mul = fp2_mulW;
			Fp::op_.fp2_neg = fp2_negW;
			Fp::op_.fp2_inv = fp2_invW;
			Fp::op_.fp2_sqr = fp2_sqrW;
			Fp::op_.fp2_mul_xi = fp2_mul_xiW;
		}
	private:
		/*
			default Fp2 operator
			Fp2 = Fp[u]/(u^2 + 1)
		*/
		static inline void fp2_addW(Unit *z, const Unit *x, const Unit *y)
		{
			const Fp *px = reinterpret_cast<const Fp*>(x);
			const Fp *py = reinterpret_cast<const Fp*>(y);
			Fp *pz = reinterpret_cast<Fp*>(z);
			Fp::add(pz[0], px[0], py[0]);
			Fp::add(pz[1], px[1], py[1]);
		}
		static inline void fp2_subW(Unit *z, const Unit *x, const Unit *y)
		{
			const Fp *px = reinterpret_cast<const Fp*>(x);
			const Fp *py = reinterpret_cast<const Fp*>(y);
			Fp *pz = reinterpret_cast<Fp*>(z);
			Fp::sub(pz[0], px[0], py[0]);
			Fp::sub(pz[1], px[1], py[1]);
		}
		static inline void fp2_negW(Unit *y, const Unit *x)
		{
			const Fp *px = reinterpret_cast<const Fp*>(x);
			Fp *py = reinterpret_cast<Fp*>(y);
			Fp::neg(py[0], px[0]);
			Fp::neg(py[1], px[1]);
		}
		/*
			x = a + bu, y = c + du, u^2 = -1
			z = xy = (a + bu)(c + du) = (ac - bd) + (ad + bc)u
			ad+bc = (a + b)(c + d) - ac - bd
		*/
		static inline void fp2_mulW(Unit *z, const Unit *x, const Unit *y)
		{
			const Fp *px = reinterpret_cast<const Fp*>(x);
			const Fp *py = reinterpret_cast<const Fp*>(y);
			Fp *pz = reinterpret_cast<Fp*>(z);
			const Fp& a = px[0];
			const Fp& b = px[1];
			const Fp& c = py[0];
			const Fp& d = py[1];
			Fp t1, t2, ac, bd;
			Fp::add(t1, a, b);
			Fp::add(t2, c, d);
			t1 *= t2; // (a + b)(c + d)
			Fp::mul(ac, a, c);
			Fp::mul(bd, b, d);
			Fp::sub(pz[0], ac, bd); // ac - bd
			Fp::sub(pz[1], t1, ac);
			pz[1] -= bd;
		}
		/*
			x = a + bu, u^2 = -1
			y = x^2 = (a + bu)^2 = (a^2 - b^2) + 2abu
		*/
		static inline void fp2_sqrW(Unit *y, const Unit *x)
		{
			const Fp *px = reinterpret_cast<const Fp*>(x);
			Fp *py = reinterpret_cast<Fp*>(y);
			const Fp& a = px[0];
			const Fp& b = px[1];
			Fp aa, bb, t;
			Fp::sqr(aa, a);
			Fp::sqr(bb, b);
			Fp::mul(t, a, b);
			Fp::sub(py[0], aa, bb); // a^2 - b^2
			Fp::add(py[1], t, t); // 2ab
		}
		/*
			x = a + bu
			y = (a + bu)xi = (a + bu)(xi_c + u)
			=(a * x_ic - b) + (a + b xi_c)u
		*/
		static inline void fp2_mul_xiW(Unit *y, const Unit *x)
		{
			const Fp *px = reinterpret_cast<const Fp*>(x);
			Fp *py = reinterpret_cast<Fp*>(y);
			const Fp& a = px[0];
			const Fp& b = px[1];
			Fp t;
			Fp::mul(t, a, xi_c_);
			t -= b;
			Fp::mul(py[1], b, xi_c_);
			py[1] += a;
			py[0] = t;
		}
		/*
			x = a + bu
			1 / x = (a - bu) / (a^2 + b^2)
		*/
		static inline void fp2_invW(Unit *y, const Unit *x)
		{
			const Fp *px = reinterpret_cast<const Fp*>(x);
			Fp *py = reinterpret_cast<Fp*>(y);
			const Fp& a = px[0];
			const Fp& b = px[1];
			Fp aa, bb;
			Fp::sqr(aa, a);
			Fp::sqr(bb, b);
			aa += bb;
			Fp::inv(aa, aa); // aa = 1 / (a^2 + b^2)
			py[0] *= aa;
			py[1] *= aa;
			Fp::neg(py[1], py[1]);
		}
		static inline void powerArray(Fp2& z, const Fp2& x, const fp::Unit *y, size_t yn, bool isNegative)
		{
			Fp2 tmp;
			const Fp2 *px = &x;
			if (&z == &x) {
				tmp = x;
				px = &tmp;
			}
			z = 1;
			fp::powerGeneric(z, *px, y, yn, Fp2::mul, Fp2::sqr);
			if (isNegative) {
				Fp2::inv(z, z);
			}
		}
	};

	/*
		Fp6T = Fp2[v] / (v^3 - xi)
		xi = -u - 1
		x = a + b v + c v^2
	*/
	struct Fp6 {
		Fp2 a, b, c;
		Fp6() { }
		Fp6(int64_t a) : a(a) , b(0) , c(0) { }
		Fp6(int64_t a, int64_t b, int64_t c) : a(a) , b(b) , c(c) { }
		Fp6(const Fp2& a, const Fp2& b, const Fp2& c) : a(a) , b(b) , c(c) { }
		void clear()
		{
			a.clear();
			b.clear();
			c.clear();
		}
		Fp* get() { return a.get(); }
		const Fp* get() const { return a.get(); }
		Fp2* getFp2() { return &a; }
		const Fp2* getFp2() const { return &a; }
		bool isZero() const
		{
			return a.isZero() && b.isZero() && c.isZero();
		}
		bool operator==(const Fp6& rhs) const
		{
			return a == rhs.a && b == rhs.b && c == rhs.c;
		}
		bool operator!=(const Fp6& rhs) const { return !operator==(rhs); }
		void getStr(std::string& str, int base = 10, bool withPrefix = false) const
		{
			str = '[';
			str += a.getStr(base, withPrefix);
			str += ',';
			str += b.getStr(base, withPrefix);
			str += ',';
			str += c.getStr(base, withPrefix);
			str += ']';
		}
		void setStr(const std::string& str, int base = 0)
		{
			const size_t size = str.size();
			if (size >= 4 + 5 * 3 && str[0] == '[' && str[size - 1] == ']') { // '[' + <a> + ',' + <b> + ',' + <c> + ']'
			}
			throw cybozu::Exception("Fp6:setStr:bad format") << str;
		}
		std::string getStr(int base = 10, bool withPrefix = false) const
		{
			std::string str;
			getStr(str, base, withPrefix);
			return str;
		}
		friend std::ostream& operator<<(std::ostream& os, const Fp6& x)
		{
			return os << "[" << x.a_ << ",\n " << x.b_ << ",\n " << x.c_ << "]";
		}
		friend std::istream& operator>>(std::istream& is, Fp6& x)
		{
			char c1, c2, c3, c4;
			is >> c1 >> x.a_ >> c2 >> x.b_ >> c3 >> x.c_ >> c4;
			if (c1 == '[' && c2 == ',' && c3 == ',' && c4 == ']') return is;
			throw std::ios_base::failure("bad Fp6");
		}
		static inline void add(Fp6& z, const Fp6& x, const Fp6& y)
		{
			Fp2::add(z.a, x.a, y.a);
			Fp2::add(z.b, x.b, y.b);
			Fp2::add(z.c, x.c, y.c);
		}
		static inline void sub(Fp6& z, const Fp6& x, const Fp6& y)
		{
			Fp2::sub(z.a, x.a, y.a);
			Fp2::sub(z.b, x.b, y.b);
			Fp2::sub(z.c, x.c, y.c);
		}
		static inline void neg(Fp6& y, const Fp6& x)
		{
			Fp2::neg(y.a, x.a);
			Fp2::neg(y.b, x.b);
			Fp2::neg(y.c, x.c);
		}
		static void sqr(Fp6& z, const Fp6& x)
		{
			assert(&z != &x);
			Fp2 v3, v4, v5;
			Fp2::add(v4, x.a, x.a);
			Fp2::mul(v4, v4, x.b);
			Fp2::sqr(v5, x.c);
			Fp2::mul_xi(z.b, v5);
			z.b += v4;
			Fp2::sub(z.c, v4, v5);
			Fp2::sqr(v3, x.a);
			Fp2::sub(v4, x.a, x.b);
			v4 += x.c;
			Fp2::add(v5, x.b, x.b);
			Fp2::mul(v5, v5, x.c);
			Fp2::sqr(v4, v4);
			Fp2::mul_xi(z.a, v5);
			z.a += v3;
			z.c += v4;
			z.c += v5;
			z.c -= v3;
		}
	};
};

template<class Fp> Fp BnT<Fp>::Fp2::xi_c_;

} // mcl

