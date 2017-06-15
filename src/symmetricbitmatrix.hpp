#ifndef SYMMETRIC_BIT_MATRIX_H
#define SYMMETRIC_BIT_MATRIX_H

#include <cstdint>
#include <boost/dynamic_bitset.hpp>

/**
 * @brief A symmetric square matrix of boolean values.
 */
class SymmetricBitMatrix {
  public:
    /**
     * @brief Constructor
     *
     * The boolean values are by default all set to @c false.
     * @param size      The dimensions of the matrix to create.
     */
    SymmetricBitMatrix(int size)
        :size_{size},
         bits_{(size * (size + 1)) / 2} {}

    /**
     * @brief Accessor for the size of the matrix.
     * @return The number of rows, or number of columns in the matrix.
     */
    int size() const noexcept {
        return size_;
    }

    /**
     * @brief Accessor for a boolean value at a specific position.
     * @param row       The row to query.
     * @param col       The column to query.
     * @return The boolean value at the specified position.
     */
    bool Value(int row, int col) const {
        return bits_[GetPos(row, col)];
    }

    /**
     * @brief Mutator for a boolean value at a specific position.
     * @param row       The row to set.
     * @param col       The column to set.
     * @param value     The boolean value to set.
     */
    void SetValue(int row, int col, bool value) {
        bits_[GetPos(row, col)] = value;
    }

    SymmetricBitMatrix(SymmetricBitMatrix const& rhs) = default;
    SymmetricBitMatrix(SymmetricBitMatrix&& rhs) = default;
    SymmetricBitMatrix& operator=(SymmetricBitMatrix const& rhs) = delete;
    SymmetricBitMatrix& operator=(SymmetricBitMatrix&& rhs) = delete;

  private:
    int size_;
    boost::dynamic_bitset<uintptr_t> bits_; // Machine word size blocks

    static int constexpr GetPos(int row, int col) noexcept {
        if (row < col) {
            int temp = row;
            row = col;
            col = temp;
        }
        return (row * (row + 1)) / 2 + col;
    }
};

#endif //SYMMETRIC_BIT_MATRIX_H
