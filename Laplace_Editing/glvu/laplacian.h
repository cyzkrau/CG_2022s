#pragma once
#include <vector>

#include <Eigen/Core>
#include <Eigen/Sparse>

template<class MatF>
float dst2(const MatF& X, int i, int j){
    float ans = (X(i, 0) - X(j, 0)) * (X(i, 0) - X(j, 0));
    ans += (X(i, 1) - X(j, 1)) * (X(i, 1) - X(j, 1));
    return ans + (X(i, 2) - X(j, 2)) * (X(i, 2) - X(j, 2));
}

template<class MatF, class MatI> 
auto Laplacian_uniform(const MatF &X, const MatI &T)
{
    X; // point position
    T;// face
    std::vector<Eigen::Triplet<float>> ijv;

    // TODO 1: compute ijv triplet for the sparse Laplacian

    for (int i = 0; i < T.rows(); i++) {
        ijv.push_back(Eigen::Triplet<float>(T(i, 0), T(i, 1), -1));
        ijv.push_back(Eigen::Triplet<float>(T(i, 0), T(i, 2), -1));
        ijv.push_back(Eigen::Triplet<float>(T(i, 1), T(i, 0), -1));
        ijv.push_back(Eigen::Triplet<float>(T(i, 1), T(i, 2), -1));
        ijv.push_back(Eigen::Triplet<float>(T(i, 2), T(i, 0), -1));
        ijv.push_back(Eigen::Triplet<float>(T(i, 2), T(i, 1), -1));
    }

    //////////////////////////////////////////////////////////////////
    int nv = X.rows();
    Eigen::SparseMatrix<float, Eigen::ColMajor> M(nv, nv);

    M.setFromTriplets(ijv.cbegin(), ijv.cend());
    for (int i = 0; i < M.outerSize(); i++)for (Eigen::SparseMatrix<float>::InnerIterator it(M, i); it; ++it)it.valueRef() = -1;
    for (int i = 0; i < M.cols(); i++)M.insert(i, i) = -M.row(i).sum();

    return M;
}

template<class MatF, class MatI>
auto Laplacian_cot(const MatF& X, const MatI& T)
{
    X; // point position
    T;// face
    std::vector<float> edge1,edge2,edge3;
    std::vector<Eigen::Triplet<float>> ijv;
    for (int i = 0; i < T.rows(); i++) {
        edge1.push_back(dst2(X, T(i, 1), T(i, 2)));
        edge2.push_back(dst2(X, T(i, 0), T(i, 2)));
        edge3.push_back(dst2(X, T(i, 1), T(i, 0)));
    }
    for (int i = 0; i < size(edge1); i++) {
        float c0 = (edge2[i] + edge3[i] - edge1[i]) / 2 / sqrt(edge2[i] * edge3[i]);
        float c1 = (edge1[i] + edge3[i] - edge2[i]) / 2 / sqrt(edge1[i] * edge3[i]);
        float c2 = (edge1[i] + edge2[i] - edge3[i]) / 2 / sqrt(edge1[i] * edge2[i]);
        ijv.push_back(Eigen::Triplet<float>(T(i, 0), T(i, 1), c2 / sqrt(1 - c2 * c2)));
        ijv.push_back(Eigen::Triplet<float>(T(i, 1), T(i, 0), c2 / sqrt(1 - c2 * c2)));
        ijv.push_back(Eigen::Triplet<float>(T(i, 1), T(i, 2), c0 / sqrt(1 - c0 * c0)));
        ijv.push_back(Eigen::Triplet<float>(T(i, 2), T(i, 1), c0 / sqrt(1 - c0 * c0)));
        ijv.push_back(Eigen::Triplet<float>(T(i, 2), T(i, 0), c1 / sqrt(1 - c1 * c1)));
        ijv.push_back(Eigen::Triplet<float>(T(i, 0), T(i, 2), c1 / sqrt(1 - c1 * c1)));
    }
    //////////////////////////////////////////////////////////////////
    int nv = X.rows();
    Eigen::SparseMatrix<float, Eigen::ColMajor> M(nv, nv);

    M.setFromTriplets(ijv.cbegin(), ijv.cend());
    for (int i = 0; i < M.cols(); i++)M.insert(i, i) = -M.row(i).sum();

    return M;
}

