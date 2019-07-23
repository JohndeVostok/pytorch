#pragma once
#ifdef BUILD_NAMEDTENSOR

#include <ATen/NamedTensor.h>
#include <ATen/core/Tensor.h>
#include <functional>

namespace at {

inline bool has_names(TensorList tensors) {
  return std::any_of(
      tensors.begin(), tensors.end(), [](const Tensor& t) { return t.is_named(); });
}

// Sets the names of `tensor` to be `names`.
CAFFE2_API Tensor& internal_set_names_inplace(Tensor& tensor, optional<DimnameList> names);
CAFFE2_API Tensor& internal_set_names_inplace(Tensor& tensor, std::vector<Dimname>&& names, bool validate_names);

// Converts dim to an positional index. Errors if `dim` cannot be used to
// refer to any dimension of tensor.
CAFFE2_API int64_t dimname_to_position(const Tensor& tensor, Dimname dim);
CAFFE2_API std::vector<int64_t> dimnames_to_positions(const Tensor& tensor, DimnameList dims);

// Unifies two DimnameList to produce a third. This is useful for implementing
// the named inference rule for binary broadcasting operations like add.
//
// There are three main constraints:
// 1) Check matching: Names must match positionally from the right.
// 2) Check misaligned: If a name `n` is in `names`, then it must appear at
//    the same index from the right in other.
// 3) The output names are obtained by unifying the names individually from the right.
CAFFE2_API optional<std::vector<Dimname>>
unify_from_right(optional<DimnameList> names, optional<DimnameList> other);

namespace namedinference {

// Propagates all names from src to result.
void propagate_names(Tensor& result, const Tensor& src);
void propagate_names(TensorImpl* result, /*const */TensorImpl* src);

// Propagates all names except for those at the excluded_idxs.
void propagate_names_except(Tensor& result, const Tensor& src, IntArrayRef excluded_idxs);

// Used for reduction ops that have a `keepdim` arg.
void propagate_names_for_reduction(Tensor& result, const Tensor& src, IntArrayRef excluded_idxs, bool keepdim);

// [Binary op interface]
// There are two main behaviors for binary ops that we are considering:
// 1. Broadcasting by position, names act as a type system to check alignment.
// 2. Broadcasting by names, names get auto-aligned before the binary op.
// I want to leave open the option of experimenting with both of them, so we're
// going to define the following interface and usage so that it is possible to
// easily switch between the two behaviors.
//
// tensor', other', outnames = unify_names_for_binary_op(tensor, other);
// result = binary_op(tensor', other')
// return result.set_names_(outnames)
//
// TODO(rzou): cleanup when we're done experimenting.
std::tuple<Tensor,Tensor,optional<std::vector<Dimname>>>
unify_names_for_binary_op(const Tensor& tensor, const Tensor& other);

} // namespace namedinference

} // namespace at
#endif
