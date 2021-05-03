#pragma once
#include <utility>

/** 
 *  2D Matrix abstraction that returns an arbitrary default value
 *  when out-of-bounds indices are referenced. By default constructors
 *  set every cell to default value.
 *   
 *  ------------------------ DEV-NOTES --------------------------
 *  TODO: 1-dimensional array -> faster! (?) 
 *  PTODO: Can we improve the default ("RCMatrix()") c-tor? 
 *  PTODO: wygodniejsza wersja dla klienta set(r, c, v) dla klienta
 */
template <typename T>
class RCMatrix {
 public:
  RCMatrix() : mtx_(nullptr), rows_(-1), cols_(-1), def_val_(0) {}

  //
  // After reserving memory sets every value to def_val
  RCMatrix(int rows, int cols, T def_val)
      : rows_(rows), cols_(cols), def_val_(def_val) {
    mtx_ = new T*[rows_];
    for (auto i = 0; i < rows_; i++) {
      mtx_[i] = new T[cols_];
    }

    clear_vals();
  }

  RCMatrix(RCMatrix&&) noexcept;
  RCMatrix& operator=(RCMatrix&& rv) noexcept;
  RCMatrix<T>(const RCMatrix<T>& orig) noexcept; 
  RCMatrix<T>& operator=(const RCMatrix<T>& orig) noexcept;

  virtual ~RCMatrix<T>();

  static bool swap(RCMatrix<T>& mat1, RCMatrix<T>& mat2); /*PTODO: unnecessary?*/

  T operator()(int row, int col);
  bool set(int row, int col, T val);
  void clear_vals();

 private:
  T** mtx_;
  int rows_, cols_;
  T def_val_; // default value
};

/**************************************************************************/
/**************************** Static Functions ****************************/
/**************************************************************************/

template <typename T>
bool RCMatrix<T>::swap(RCMatrix<T>& mat1, RCMatrix<T>& mat2) {
  std::swap(mat1.mtx_, mat2.mtx_);
  std::swap(mat1.rows_, mat2.rows_);
  std::swap(mat1.cols_, mat2.cols_);
  std::swap(mat1.def_val_, mat2.def_val_);

  return true;
}

/**************************************************************************/
/*************************** General Functions ****************************/
/**************************************************************************/

template <typename T>
T RCMatrix<T>::operator()(int row, int col) {
  if ((col < 0 || col >= cols_) || (row < 0 || row >= rows_)) {
    return def_val_;
  }
  return mtx_[row][col];
}

template <typename T>
bool RCMatrix<T>::set(int row, int col, T val) {
  if ((col < 0 || col >= cols_) || (row < 0 || row >= rows_)) {
    return false;
  }
  mtx_[row][col] = val;
  return true;
}

template <typename T>
void RCMatrix<T>::clear_vals() {
  for (auto i = 0; i < rows_; i++) {
    for (auto j = 0; j < cols_; j++) {
      mtx_[i][j] = def_val_;
    }
  }
}

/**************************************************************************/
/***************************** C-tors, D-tors *****************************/
/**************************************************************************/

//
// Doesn't copy the default value
template <typename T>
RCMatrix<T>& RCMatrix<T>::operator=(const RCMatrix<T>& orig) noexcept {
  if (orig.cols_ != this->cols_ || orig.rows_ != this->rows_) {
    // PTODO
    throw "Cannot perform a copy on matrices of different sizes.";
  }

  for (auto i = 0; i < rows_; i++) {
    for (auto j = 0; j < cols_; j++) {
      this->mtx_[i][j] = orig.mtx_[i][j];
    }
  }
}

//
// Doesn't copy the default value
template <typename T>
RCMatrix<T>::RCMatrix<T>(const RCMatrix<T>& orig) noexcept {
  if (orig.cols_ != this->cols_ || orig.rows_ != this->rows_) {
    // PTODO
    throw "Cannot perform a copy on matrices of different sizes.";
  }

  for (auto i = 0; i < rows_; i++) {
    for (auto j = 0; j < cols_; j++) {
      this->mtx_[i][j] = orig.mtx_[i][j];
    }
  }
}

template <typename T>
RCMatrix<T>& RCMatrix<T>::operator=(RCMatrix&& rv) noexcept {
  this->mtx_ = rv.mtx_;
  this->rows_ = rv.rows_;
  this->cols_ = rv.cols_;
  this->def_val_ = rv.def_val_;

  rv.mtx_ = nullptr;
  rv.rows_ = -1;
  rv.cols_ = -1;

  return *this;
}

template <typename T>
RCMatrix<T>::RCMatrix(RCMatrix && rv) noexcept {
  this->mtx_ = rv.mtx_;
  this->rows_ = rv.rows_;
  this->cols_ = rv.cols_;
  this->def_val_ = rv.def_val_;

  rv.mtx_ = nullptr;
  rv.rows_ = -1;
  rv.cols_ = -1;

 /* return *this;*/
}

template <typename T>
RCMatrix<T>::~RCMatrix<T>() {
  for (auto i = 0; i < rows_; i++) {
    delete[] mtx_[i];
  }
  delete[] mtx_;
}