#pragma once

#include <cstdlib>
#include <type_traits>

namespace heatsink {
	/**
	 * To ensure propery interop with the C-style memory consuming GL methods,
	 * most types should be "standard layout" such that their bytes can be
	 * copied properly. To the reduce the amount of times "requires 
	 * (is_standard_layout)" must be typed, this concept is defined.
	 */
	template<class T>
	concept standard_layout = std::is_standard_layout_v<T>;
	
	namespace detail {
		// The result type from an invocation of `operator []`.
		template<class T>
		using subscript_t = std::remove_cvref_t<decltype(std::declval<T>()[0])>;

		// Check if this is a higher-level type in a mutidimensional type; that
		// is, an array-like type whose elements are also tensors.
		template<class T>
		concept subscriptable = standard_layout<T> && !std::is_pointer_v<T> &&
			requires { typename subscript_t<T>; };
	}

	/**
	 * In this context, a tensor is defined as a type containing only an
	 * n-dimensional array of arithmetic values. This includes scalars, arrays,
	 * and aggregates of a single (tensor) type. To be considered a tensor, an
	 * aggregate must also have an `operator []` defined, that returns a result
	 * of the same type as its member variable. While this method checks for
	 * most of these properties, it cannot prevent a class from "pretending" it
	 * it a tensor; see example `struct d` below.
	 *
	 * Tensors:
	 * ```
	 * int a;
	 * double b[4][4];
	 * struct c {
	 *     float v[3];
	 *     float operator [](int i) const;
	 * };
	 * glm::vec4 d;
	 * ```
	 *
	 * Non-tensors:
	 * ```
	 * int* a;
	 * struct b {
	 *     float v[2];
	 * };
	 * struct c {
	 *     float v[2];
	 *     int operator [](int i) const;
	 * };
	 * struct d {
	 *     float v[2];
	 *     int m;
	 *     float operator [](int i) const;
	 * };
	 */
    template<class T>
    struct is_tensor;
	template<class T>
	constexpr auto is_tensor_v = is_tensor<T>::value;

	/**
	 * A concept definition for `is_tensor`. The rest of the tensor traits
	 * methods are constrained around this concept.
	 */
	template<class T>
	concept tensor = is_tensor_v<T>;

	namespace detail {
		// Separately identify multi-dimensional tensors for specific traits
		// method overrides.
		template<class T>
		concept tensor_array = tensor<T> &&
			requires { typename subscript_t<T>; };
	}

	/**
	 * A value representing the dimensional rank of the given type. This is
	 * determined by testing how many times `operator []` can be called on a
	 * type before the result has no such operator.
	 */
	template<tensor T>
	struct tensor_rank;
	template<tensor T>
	constexpr auto tensor_rank_v = tensor_rank<T>::value;

	/**
	 * A value representing the desired dimensional extent of the given type.
	 * The `N` parameter represents the dimension to calculate the extent of,
	 * where `0` is the uppermost dimension. This value is determined by
	 * comparing the size of a tensor to a single element of its subscript type,
	 * as given by `operator []`.
	 */
	template<tensor T, std::size_t N = 0>
	struct tensor_extent;
	template<tensor T, std::size_t N = 0>
	constexpr auto tensor_extent_v = tensor_extent<T>::value;

	/**
	 * A basic array type that is equivalent to the given type. This takes an
	 * opaque type (such as `glm::mat4`) and converts it to a primitive array
	 * type (such as `float[4][4]`), based on the internal element type.
	 */
	template<tensor T>
	struct tensor_decay;
	template<tensor T>
	using tensor_decay_t = typename tensor_decay<T>::type;
}

namespace heatsink {
	// For a zero-dimensional type, a tensor only has to be arithmetic.
    template<class T>
    struct is_tensor : std::bool_constant<std::is_arithmetic_v<T>> {};
	// Otherwise, it must satisfy the tensor constraints recursively.
	template<detail::subscriptable T>
	struct is_tensor<T> : std::bool_constant<is_tensor<detail::subscript_t<T>>::value> {};

	// A zero-dimensional tensor always has a rank of zero.
	template<tensor T>
	struct tensor_rank : std::integral_constant<std::size_t, 0> {};
	// Otherwise, its rank is equal to the number of tensor dimensions below it.
	template<detail::tensor_array T>
	struct tensor_rank<T> : std::integral_constant<std::size_t,
		1u + tensor_rank<detail::subscript_t<T>>::value
	> {};

	// A zero-dimensional tensor always has an extent of zero.
	template<tensor T, std::size_t N>
	struct tensor_extent : std::integral_constant<std::size_t, 0> {};
	// Otherwise, assuming the current dimension is the one of interest, the
	// extent can be calculated using the size of the element type.
	template<detail::tensor_array T>
	struct tensor_extent<T, 0> : std::integral_constant<std::size_t,
		sizeof(T) / sizeof(detail::subscript_t<T>)
	> {};
	// In the final case, recurse until the dimension of interest is reached.
	template<detail::tensor_array T, std::size_t N>
	struct tensor_extent<T, N> : tensor_extent<detail::subscript_t<T>, (N - 1)> {};

	// A zero-dimensional tensor (as described above) must be arithmetic.
	template<tensor T>
	struct tensor_decay {
		using type = std::remove_cvref_t<T>;
	};
	// Otherwise, recursively construct a type based on the raw element type.
	// Eventually, an arithmetic type will be reached and used as the base.
	template<detail::tensor_array T>
	struct tensor_decay<T> {
		using type = typename tensor_decay<detail::subscript_t<T>>::type[tensor_extent<T>::value];
	};
}
