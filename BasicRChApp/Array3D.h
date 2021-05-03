#pragma once
#include <array>
#include <vector>

/*
    TODOs:
      > Emplace
      > C-tors
      > iterators
      > swap?
*/

namespace rch {

template <typename TData, std::size_t Dim3, std::size_t Dim2, std::size_t Dim1>
class Array3D {
 public:
  /*
      Modifiers
  */

  void assign(const TData& newVal, std::size_t idx3, std::size_t idx2,
              std::size_t idx1) {
    m_data[calcDataIdx(idx3, idx2, idx1)] = newVal;
  }

  void assign(TData&& newVal, std::size_t idx3, std::size_t idx2,
              std::size_t idx1) {
    m_data[calcDataIdx(idx3, idx2, idx1)] = std::move(newVal);
  }

  /*
      Element access
  */

  const TData& val(std::size_t idx3, std::size_t idx2, std::size_t idx1) const {
    return m_data[calcDataIdx(idx3, idx2, idx1)];
  };

  // Returns m_data[idx], where m_data is the inner, linear, array buffer
  const TData& valLin(std::size_t idx) const { return m_data[idx]; };

  // Returns the value at given indices if these are not out of bounds
  // or the defaultVal otherwise
  const TData& dval(const TData defaultVal, std::size_t idx3, std::size_t idx2,
                    std::size_t idx1) const {
    if (checkBounds(idx3, idx2, idx1)) {
      return m_data[calcDataIdx(idx3, idx2, idx1)];
    }

    return defaultVal;
  };

  TData& obj(std::size_t idx3, std::size_t idx2, std::size_t idx1) {
    return m_data[calcDataIdx(idx3, idx2, idx1)];
  };

  // Returns ref to m_data[idx], where m_data is the inner, linear, array buffer
  TData& objLin(std::size_t idx) { return m_data[idx]; };

  /*
      Attributes
  */

  constexpr const std::size_t& size() const { return s_size; }

  constexpr std::size_t dim3() const { return Dim3; }
  constexpr std::size_t dim2() const { return Dim2; }
  constexpr std::size_t dim1() const { return Dim1; }

  constexpr std::array<std::size_t, 3>& dimensions() const { return s_dims; }

  /*
      Utility functions
  */

  // Checks if given indices are NOT out of range.
  constexpr bool checkBounds(const std::array<std::size_t, 3>& inds) const {
    // By definition std::size_t has to be >= 0
    return ((inds[0] < Dim3) && (inds[1] < Dim2) && (inds[2] < Dim1));
  }

  // Overload for raw indices
  constexpr bool checkBounds(std::size_t idx3, std::size_t idx2,
                             std::size_t idx1) const {
    return checkBounds({idx3, idx2, idx1});
  }

  // Checks if given indices (int-s) are NOT out of range. Ints might not
  // be able to cover all potential indices due to them being std::size_t.
  constexpr bool checkBoundsSign(const std::array<int, 3>& inds) const {
    return ((0 <= inds[0] && inds[0] < Dim3) && (0 <= inds[1] && inds[1] < Dim2) &&
            (0 <= inds[2] && inds[2] < Dim1));
  }

  // Overload for raw indices
  constexpr bool checkBoundsSign(int idx3, int idx2, int idx1) const {
    return checkBoundsSign({idx3, idx2, idx1});
  }

  // Calculates the index of Array3D[idx3][idx2][idx1] in the inner linear
  // buffer
  constexpr std::size_t calcDataIdx(std::size_t idx3, std::size_t idx2,
                                    std::size_t idx1) const {
    return idx3 * s_offsets[0] + idx2 * s_offsets[1] + idx1 * s_offsets[2];
  }

 private:
  std::array<TData, Dim3* Dim2* Dim1> m_data = {};

  static constexpr std::size_t s_dimCount = 3;
  static constexpr std::size_t s_size = Dim3 * Dim2 * Dim1;

  static constexpr std::array<std::size_t, s_dimCount> s_dims = {Dim3, Dim2, Dim1};
  static constexpr std::array<std::size_t, s_dimCount> s_offsets = {Dim2 * Dim1, Dim1, 1};
};

/*
    Out-of-class Utilities 
*/

template<typename TData, std::size_t Dim3,
         std::size_t Dim2, std::size_t Dim1>
inline std::vector<TData> subsetOf(Array3D<TData, Dim3, Dim2, Dim1>& arr,
                                   const std::array<std::size_t, 3>& startInds,
                                   const std::array<std::size_t, 3>& endInds) {
  if (arr.checkBounds(startInds) == false) { return std::vector<TData>(); }
  if (arr.checkBounds(endInds)   == false) { return std::vector<TData>(); }
  
  std::size_t startIdx = arr.calcDataIdx(startInds[0], startInds[1], startInds[2]);
  std::size_t endIdx = arr.calcDataIdx(endInds[0], endInds[1], endInds[2]);
  
  if (endIdx < startIdx) { return std::vector<TData>(); }
  
  std::vector<TData> result;
  result.reserve(endIdx - startIdx + 1);

  for (auto i = startIdx; i <= endIdx; ++i) {
      result.emplace_back(arr.valLin(i));
  }

  return result;
};

}  // namespace rch