/**
    @file
    @author  Alexander Sherikov

    @copyright 2017 Alexander Sherikov. Licensed under the Apache License,
    Version 2.0. (see LICENSE or http://www.apache.org/licenses/LICENSE-2.0)

    @brief
*/

#pragma once


namespace qpmad
{
    class FactorizationData
    {
        public:
            QPMatrix    QLi_aka_J;
            QPMatrix    R;
            MatrixIndex primal_size_;


        public:
            template <class t_MatrixType>
                void initialize(const t_MatrixType &H,
                                const MatrixIndex primal_size)
            {
                primal_size_ = primal_size;

                QLi_aka_J.resize(primal_size_, primal_size_);
                QLi_aka_J.triangularView<Eigen::Lower>().setZero();
                TriangularInversion::compute(QLi_aka_J, H);

                R.resize(primal_size_, primal_size_);
            }


            bool update(const MatrixIndex R_col,
                        const double tolerance)
            {
                GivensReflection    givens;
                for (MatrixIndex i = primal_size_-1; i > R_col; --i)
                {
                    givens.computeAndApply(R(i-1, R_col), R(i, R_col), 0.0);
                    givens.applyColumnWise(QLi_aka_J, 0, primal_size_, i-1, i);
                }

                if (std::abs(R(R_col, R_col)) < tolerance)
                {
                    return (false);
                }
                else
                {
                    return (true);
                }
            }


            void downdate(  const MatrixIndex R_col_index,
                            const MatrixIndex R_cols,
                            const double tolerance)
            {
                GivensReflection    givens;
                for (MatrixIndex i = R_col_index + 1; i < R_cols; ++i)
                {
                    Eigen::MatrixXd tmp = R.block(0,
                                0,
                                R_cols,
                                R_cols).triangularView<Eigen::Upper>();

                    givens.computeAndApply(R(i-1, i), R(i, i), 0.0);
                    givens.applyColumnWise(QLi_aka_J, 0, primal_size_, i-1, i);
                    givens.applyRowWise(R, i+1, R_cols, i-1, i);

                    /// @todo no need to copy the part corresponding to equalities
                    /// @todo block copy?
                    R.col(i-1).segment(0, i) = R.col(i).segment(0, i);
                }
            }


            template<   class t_VectorType,
                        class t_RowVectorType>
                void computeEqualityPrimalStep( t_VectorType            & step_direction,
                                                const t_RowVectorType   & ctr,
                                                const MatrixIndex       active_set_size)
            {
                // vector 'd'
                R.col(active_set_size).noalias() = QLi_aka_J.transpose() * ctr.transpose();

                step_direction.noalias() =
                    - QLi_aka_J.rightCols(primal_size_ - active_set_size)
                    * R.col(active_set_size).tail(primal_size_ - active_set_size);
            }


            template<   class t_VectorType0,
                        class t_VectorType1,
                        class t_RowVectorType>
                void computeInequalitySteps(t_VectorType0           & primal_step_direction,
                                            t_VectorType1           & dual_step_direction,
                                            const t_RowVectorType   & ctr,
                                            const ConstraintStatus::Status ctr_type,
                                            const ActiveSet         &active_set)
            {
                if (ConstraintStatus::ACTIVE_LOWER_BOUND == ctr_type)
                {
                    R.col(active_set.size_).noalias() =
                        - QLi_aka_J.transpose() * ctr.transpose();
                }
                else
                {
                    R.col(active_set.size_).noalias() =
                        QLi_aka_J.transpose() * ctr.transpose();
                }

                primal_step_direction.noalias() =
                    - QLi_aka_J.rightCols(primal_size_ - active_set.size_)
                    * R.col(active_set.size_).tail(primal_size_ - active_set.size_);

                dual_step_direction.segment(active_set.num_equalities_, active_set.num_inequalities_).noalias() =
                    - R.block(active_set.num_equalities_,
                            active_set.num_equalities_,
                            active_set.num_inequalities_,
                            active_set.num_inequalities_).triangularView<Eigen::Upper>().solve(
                                R.col(active_set.size_).segment(
                                    active_set.num_equalities_,
                                    active_set.num_inequalities_));
            }


            template<   class t_VectorType,
                        class t_RowVectorType>
                void computeInequalityDualStep( t_VectorType            & dual_step_direction,
                                                const t_RowVectorType   & ctr,
                                                const ConstraintStatus::Status ctr_type,
                                                const ActiveSet         & active_set)
            {
                if (ConstraintStatus::ACTIVE_LOWER_BOUND == ctr_type)
                {
                    dual_step_direction.segment(active_set.num_equalities_, active_set.num_inequalities_).noalias() =
                        QLi_aka_J.transpose().bottomRows(active_set.num_inequalities_)
                        * ctr.transpose();
                }
                else
                {
                    dual_step_direction.segment(active_set.num_equalities_, active_set.num_inequalities_).noalias() =
                        - QLi_aka_J.transpose().bottomRows(active_set.num_inequalities_)
                        * ctr.transpose();
                }

                R.block(active_set.num_equalities_,
                        active_set.num_equalities_,
                        active_set.num_inequalities_,
                        active_set.num_inequalities_).triangularView<Eigen::Upper>().solveInPlace(
                            dual_step_direction.segment(active_set.num_equalities_, active_set.num_inequalities_));
            }
    };
}
