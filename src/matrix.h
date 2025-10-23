/***************************************
* Author: Kyle Bueche
* File: matrix.h
***************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include "image.h"

class SquareMatrix
{
    public:
        float* mat;
        int side;
        int size;
        SquareMatrix(int side)
        {
            this->mat = new float[side * side];
            this->side = side;
            this->size = side * side;
        }
        SquareMatrix(const SquareMatrix& matrix)
        {
            this->mat = new float[matrix.size];
            this->side = matrix.side;
            this->size = matrix.size;
            for (int i = 0; i < this->size; i++)
            {
                this->mat[i] = matrix.mat[i];
            }
        }
        SquareMatrix& operator=(const SquareMatrix& matrix)
        {
            this->mat = new float[matrix.size];
            this->side = matrix.side;
            this->size = matrix.size;
            for (int i = 0; i < this->size; i++)
            {
                this->mat[i] = matrix.mat[i];
            }
        }
        ~SquareMatrix() { delete[] this->mat; }
        inline float& operator[](size_t i) { return mat[i]; } // Returns the row array pointer, so myMatrix[i][j] is valid.
        inline const float& operator[](size_t i) const { return mat[i]; } // Same as above but read-only.
        inline float& operator()(size_t rowIndex, size_t colIndex) { return mat[rowIndex * side + colIndex]; } // Returns the row array pointer, so myMatrix[i][j] is valid.
        inline const float& operator()(size_t rowIndex, size_t colIndex) const { return mat[rowIndex * side + colIndex]; } // Same as above but read-only.

        void scaleRow(int row, float divideBy)
        {
            for (int j = 0; j < side; j++)
            {
                (*this)(row, j) = (*this)(row, j) / divideBy;
            }
        }

        void addScaledRowToRow(int rowToAdd, int rowToModify, float scalar)
        {
            for (int j = 0; j < side; j++)
            {
                (*this)(rowToModify, j) += (*this)(rowToAdd, j) * scalar;
            }
        }
        
        void swapRows(int row1, int row2)
        {
            for (int j = 0; j < side; j++)
            {
                std::swap((*this)(row1, j), (*this)(row2, j));
            }
        }
        
        SquareMatrix findInverse()
        {
            SquareMatrix original(*this);
            SquareMatrix inverse(this->side);
            // Construct identity matrix
            for (int i = 0; i < inverse.side; i++)
                for (int j = 0; j < inverse.side; j++) {
                    inverse(i, j) = (i == j) ? 1.0f : 0.0f;
            }
            

            // Solve identity matrix on LHS, perform same ops on RHS
            for (int col = 0; col < inverse.side; col++)
            {
                int pivotRow = -1;
                float pivotValue = 0.0f;
                for (int row = col; row < inverse.side; row++)
                {
                    if (std::fabs(original(row, col)) > pivotValue)
                    {
                        pivotRow = row;
                        pivotValue = std::fabs(original(row, col));
                    }
                }
                if (pivotRow == -1)
                {
                    std::cerr << "Error: No row to pivot on. Non invertible matrix." << std::endl;
                    return inverse;
                }
                if (std::fabs(pivotValue) < 1e-8f)
                {
                    std::cerr << "Error: Value smaller than 1e-8. Non invertible matrix." << std::endl;
                    return inverse;
                }
                if (pivotRow != col)
                {
                    original.swapRows(pivotRow, col);
                    inverse.swapRows(pivotRow, col);
                }
                original.scaleRow(col, original(col, col));
                inverse.scaleRow(col, original(col, col));
                for (int row = 0; row < inverse.side; row++)
                {
                    if (row != col && std::fabs(original(row, col)) != 0.0f)
                    {
                        original.addScaledRowToRow(col, row, -original(row, col));
                        inverse.addScaledRowToRow(col, row, -original(row, col));
                    }
                }
            }
            return inverse;
        }
            
};

#endif
