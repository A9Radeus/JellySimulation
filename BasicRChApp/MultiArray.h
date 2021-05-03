#pragma once
//#include <array>
//#include <iostream>
//#include <string>
//#include <tuple>
//#include <utility>
//#include <type_traits>
//
///**
// * DEV-NOTES
// *
// * TODO:
// *  > FIX the calcIdx (it should be dim1 * dim2 * i, dim1 * j, k; for [i][j][k])
// *  > Move, Assign -constructors
// *
// * PTODO:
// *  > for index calculations figure out this method (make it work) :
// *      std::size_t iter = 1;
// *      std::size_t arrIdx = ((args * s_dims[iter++]) + ...);
// *    also @see TODO
// */
//
//namespace rch {
//
///**
// * [WIP]
// *
// * Multi-Dimensional Array
// *
// * Template Parameters:
// *    TData -- data type of the inner array (std::array<TData, (Dimensions * ...)> m_data{})
// *    Dimensions -- list of max-values for the given dimension; uses matrix-like / c_array order, i.e.:
// *      IF Dimensions == {a, b, c, d} THEN
// *        1) m_data.max_size() := a*b*c*d
// *        2) "a" is the number of cuboids and "b, c, d" are the dimensions of all cuboids.
// *
// */
//template <typename TData, std::size_t... Dimensions>
//class MultiArray {
//public:
//  template <typename... TArgs,
//    typename std::enable_if_t<sizeof...(TArgs) == sizeof...(Dimensions),
//    bool> = true>
//  void assign(const TData& newVal, TArgs&&... indices) {
//    m_data[calcDataIdx(std::forward<TArgs>(indices)...)] = newVal;
//  }
//
//  template <typename... TArgs,
//    typename std::enable_if_t<sizeof...(TArgs) == sizeof...(Dimensions),
//    bool> = true>
//  void assign(TData&& newVal, TArgs&&... indices) {
//    m_data[calcDataIdx(std::forward<TArgs>(indices)...)] = std::move(newVal);
//  }
//
//  template <typename... TArgs,
//    typename std::enable_if_t<sizeof...(TArgs) == sizeof...(Dimensions),
//    bool> = true>
//  const TData& val(TArgs&&... indices) const {
//    return m_data[calcDataIdx(std::forward<TArgs>(indices)...)];
//  };
//
//  const TData& val(std::size_t idx) const { return m_data[idx]; };
//
//  // ptodo: rename to dval?
//  // Returns a COPY of m_data value at given indicies with bounds checking.
//  // Returns a COPY of "defaultVal" if indieces are out of bounds.
//  template <typename... TArgs,
//    typename std::enable_if_t<sizeof...(TArgs) == sizeof...(Dimensions),
//      bool> = true>
//  TData valDef(const TData defaultVal, TArgs&&... indices) const {
//    std::size_t idx = 0;
//    if (calcDataIdx(idx, std::forward<TArgs>(indices)...)) {
//      return m_data[idx];
//    } else {
//      return defaultVal;    
//    }
//  };
//
//  template <typename... TArgs,
//            typename std::enable_if_t<sizeof...(TArgs) == sizeof...(Dimensions),
//                                      bool> = true>
//  TData& obj(TArgs&&... indices) {
//    return m_data[calcDataIdx(std::forward<TArgs>(indices)...)];
//  };
//
//  TData& obj(std::size_t idx) {
//    return m_data[idx];
//  };
//
//  std::size_t dimensions() const { return s_dimCount; };
//
//  template <typename... TArgs,
//            typename std::enable_if_t<sizeof...(TArgs) == sizeof...(Dimensions),
//                                      bool> = true>
//  std::size_t calcDataIdx(TArgs&&... indices) const {
//    std::size_t resIdx = 0;
//    std::array<std::size_t, s_dimCount> arrInds = {indices...};
//    for (std::size_t k = 0; k < s_dimCount - 1; k++) {
//      resIdx += arrInds[k] * s_dims[k + 1];
//    }
//    resIdx += arrInds.back();
//    return resIdx;
//  }
//
//  // PTODO: return [bool, std::size_t] instead
//  //        or  potentially use exceptions
//  template <typename... TArgs,
//            typename std::enable_if_t<sizeof...(TArgs) == sizeof...(Dimensions),
//                                      bool> = true>
//  bool calcDataIdx_Check(std::size_t& resIdx, TArgs&&... indices) const {
//    resIdx = 0;
//    std::array<std::size_t, s_dimCount> arrInds = {indices...};
//
//    for (std::size_t k = 0; k < s_dimCount - 1; k++) {
//      if ((arrInds[k] >= s_dims[k]) || (arrInds[k] < 0)) {
//        return false;
//      }
//      resIdx += arrInds[k] * s_dims[k + 1];
//    }
//    
//    if ((arrInds.back() >= s_dims.back()) || (arrInds.back() < 0)) {
//      return false;
//    }
//    resIdx += arrInds.back();
//    
//    return true;
//  }
//
//private:
//  static constexpr std::size_t s_dimCount = sizeof...(Dimensions);
//  std::array<TData, (Dimensions * ...)> m_data{};
//  static constexpr std::array<std::size_t, s_dimCount> s_dims{Dimensions...};
//  static constexpr std::array<std::size_t, s_dimCount> s_offsets{};
//
//  
//  template <std::size_t... Tail,
//            typename std::enable_if_t<
//                sizeof...(Tail) == sizeof...(Dimensions - 1), bool> = true>
//  static std::array<std::size_t, s_dimCount> calcOffsets(std::size_t head,
//                                                         std::size_t Tail...) {
//    std::array<std::size_t, s_dimCount> result{1};
//    calcOffsetsCore<1, Tail...>(result, std::size_t Tail...);
//    return result;
//  }
//
//  template <std::size_t downto, std::size_t... Tail>
//  static void calcOffsetsCore(
//      std::array<std::size_t, s_dimCount>& resArr, std::size_t head,
//      std::size_t... Tail) {
//    for (std::size_t i = s_dimCount - 1; i <= downto; ++i) {
//      resArr[i] *= head;
//    }
//    calcOffsetsCore<downto + 1, Tail...>(result, std::size_t... Tail);
//    return result;
//  }
//};
//
//}  // namespace rch