//
//  riscv-types.h
//

#ifndef riscv_types_h
#define riscv_types_h

#include <cstdint>

namespace riscv {

	/*
	 * Host type checks
	 */

	static_assert(sizeof(signed long long) == sizeof(uint64_t), "requires 64-bit signed long long");
	static_assert(sizeof(unsigned long long) == sizeof(uint64_t), "requires 64-bit unsigned long long");
	static_assert(sizeof(float) == sizeof(uint32_t), "requires 32-bit float");
	static_assert(sizeof(double) == sizeof(uint64_t), "requires 64-bit double");

	/*
	 * Short-hand type aliases
	 *
	 * The purpose is to use a placeholder type globally for the natural
	 * register width of the target. The placeholder type names match the
	 * C pseudo-code cast notation in meta/instructions
	 */

	typedef int8_t             s8;
	typedef uint8_t            u8;
	typedef int16_t            s16;
	typedef uint16_t           u16;
	typedef int32_t            s32;
	typedef uint32_t           u32;
	typedef int64_t            s64;
	typedef uint64_t           u64;
	typedef signed __int128   s128;
	typedef unsigned __int128 u128;
	typedef double             f64;
	typedef float              f32;

	/*
	 * Width-typed immediate template aliases
	 */

	template <uint64_t W> struct offset_t;
	template <uint64_t W> struct ptr_t;
	template <uint64_t W> struct simm_t;
	template <uint64_t W> struct uimm_t;

	using offset32 = offset_t<32>;
	using offset21 = offset_t<21>;
	using offset13 = offset_t<13>;
	using offset12 = offset_t<12>;
	using ptr64 = ptr_t<64>;
	using ptr32 = ptr_t<32>;
	using simm32 = simm_t<32>;
	using uimm32 = uimm_t<32>;
	using simm20 = simm_t<20>;
	using uimm20 = uimm_t<20>;
	using simm12 = simm_t<12>;
	using uimm12 = uimm_t<12>;
	using uimm6 = uimm_t<6>;
	using uimm5 = uimm_t<5>;
	using ireg5 = uimm_t<5>;
	using freg5 = uimm_t<5>;
	using arg4 = uimm_t<4>;
	using arg3 = uimm_t<3>;
	using arg1 = uimm_t<1>;

	/*
	 * Width-typed immediate template definitions
	 */

	template <uint64_t W> struct offset_t
	{
		enum : uint64_t { width = W };
		enum : intptr_t { min = -(1LL<<(W-1)), max = (1LL<<(W-1))-1 };
		enum : bool { is_signed = true, is_integral = false, is_offset = true, is_pointer = false };
		typedef intptr_t value_type;
		intptr_t imm;
		offset_t(intptr_t imm) : imm(imm) {}
		bool valid() { return imm <= max && imm >= min; }
		operator intptr_t() const { return imm; }
	};

	template <uint64_t W> struct ptr_t
	{
		enum : uint64_t { width = W };
		enum : uintptr_t { min = 0, max = W == 64 ? ~0 : (1ULL<< W)-1 };
		enum : bool { is_signed = false, is_integral = true, is_offset = false, is_pointer = true };
		typedef uintptr_t value_type;
		uintptr_t imm;
		ptr_t(uintptr_t imm) : imm(imm) {}
		bool valid() { return imm <= max; }
		operator uintptr_t() const { return imm; }
	};

	template <uint64_t W> struct simm_t
	{
		enum : uint64_t { width = W };
		enum : int64_t { min = -(1LL<<(W-1)), max = (1LL<<(W-1))-1 };
		enum : bool { is_signed = true, is_integral = true, is_offset = false, is_pointer = false };
		typedef int64_t value_type;
		int64_t imm;
		simm_t(int64_t imm) : imm(imm) {}
		bool valid() { return imm <= max && imm >= min; }
		operator int64_t() const { return imm; }
	};

	template <uint64_t W> struct uimm_t
	{
		enum : uint64_t { width = W };
		enum : uint64_t { min = 0, max = (1ULL<< W)-1 };
		enum : bool { is_signed = false, is_integral = true, is_offset = false, is_pointer = false };
		typedef uint64_t value_type;
		uint64_t imm;
		uimm_t(uint64_t imm) : imm(imm) {}
		bool valid() { return imm <= max; }
		operator uint64_t() const { return imm; }
	};

	/*
	 * Width-typed immediate constructor wrappers
	 *
	 * (template constructors cannot take template parameters)
	 *
	 * simm<N>(uint64_t imm)
	 * uimm<N>(uint64_t imm)
	 */

	template <uint64_t W> constexpr simm_t<W> simm(const uint64_t imm) { return simm_t<W>(imm); }
	template <uint64_t W> constexpr uimm_t<W> uimm(const uint64_t imm) { return uimm_t<W>(imm); }

	/* Sign extension template */

	template <typename T, unsigned B>
	inline T sign_extend(const T x)
	{
		struct { T x:B; } s;
		return s.x = x;
	}

	/*
	 * Bit range template
	 *
	 * N	=   decoded MSB offset
	 * M	=   decoded LSB offset
	 */

	template<int N, int M = N>
	struct B
	{
		enum { n = N };
		enum { m = M };
		enum { width = N - M + 1 };

		static_assert(N >= M, "N ≥ M");
	};

	/*
	 * Immediate bit range segment
	 *
	 * K	=   instruction MSB offset
	 * L	=   instruction LSB offset
	 * Args =   decoded bit offsets i.e. B<N:M>, ...
	 */

	template<int K, int L, typename... Args>
	struct S;

	template<int K, int L>
	struct S<K,L>
	{
		enum { offset = 0 };

		static_assert((L > 0), "L > 0");
		static_assert((K < sizeof(1ULL) << 3), "K < sizeof(1ULL) << 3");

		static inline constexpr uint64_t decode(uint64_t inst) { return 0; }
		static inline constexpr uint64_t encode(uint64_t imm) { return 0; }
	};

	template<int K, int L, typename H, typename... T>
	struct S<K,L,H,T...> : S<K,L,T...>
	{
		typedef S<K,L,T...> I;

		static_assert((L > 0), "L > 0");
		static_assert((K < sizeof(1ULL) << 3), "K < sizeof(1ULL) << 3");

		enum { offset = I::offset + H::width };
		enum { shift = offset + L - H::width - H::m };

		static inline constexpr uint64_t decode(uint64_t inst) {
			const uint64_t mask = ((uint64_t(1) << (H::n + 1)) - 1) ^ ((uint64_t(1) << H::m) - 1);
			return ((shift < 0 ? inst << -shift : inst >> shift) & mask) | I::decode(inst);
		}

		static inline constexpr uint64_t encode(uint64_t imm) {
			const uint64_t mask = ((uint64_t(1) << (H::n + 1)) - 1) ^ ((uint64_t(1) << H::m) - 1);
			return ((shift < 0 ? (imm & mask) >> -shift : (imm & mask) << shift)) | I::encode(imm);
		}
	};

	/*
	 * Immediate bit range notation template
	 *
	 * W	=   number of bits for sign extension
	 * Args =   bit range segments i.e. S<K,L, B<N:M>, ...>, ...
	 */

	template<typename R, int W, typename... Args>
	struct imm_arg_impl_t;

	template<typename R, int W>
	struct imm_arg_impl_t<R,W>
	{
		static inline constexpr R decode(uint64_t inst) { return 0; }
		static inline constexpr R encode(uint64_t imm) { return 0; }
	};

	template<typename R, int W, typename H, typename... T>
	struct imm_arg_impl_t<R,W,H,T...> : imm_arg_impl_t<R,W,T...>
	{
		typedef imm_arg_impl_t<R,W,T...> I;

		static inline constexpr R decode(uint64_t inst) { return I::decode(inst) | H::decode(inst); }
		static inline constexpr R encode(uint64_t imm) { return I::encode(imm) | H::encode(imm); }
	};

	template<int W, typename... Args>
	struct simm_arg_t : imm_arg_impl_t<int64_t,W,Args...>
	{
		typedef imm_arg_impl_t<int64_t,W,Args...> I;

		static constexpr int64_t decode(uint64_t inst) { return sign_extend<int64_t,W>(I::decode(inst)); }
		static constexpr int64_t encode(uint64_t imm) { return I::encode(imm); }
	};

	template<int W, typename... Args>
	struct uimm_arg_t : imm_arg_impl_t<uint64_t,W,Args...>
	{
		typedef imm_arg_impl_t<uint64_t,W,Args...> I;

		static constexpr uint64_t decode(uint64_t inst) { return I::decode(inst); }
		static constexpr uint64_t encode(uint64_t imm) { return I::encode(imm); }
	};

}

#endif
