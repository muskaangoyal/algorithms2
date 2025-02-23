#pragma once
//
//  assignment3.hpp
//  Algorithms: Design and Analysis, Part 2
//
//  Programming Assignment 3
//
//  Created by Nenad Mancevic on 4/18/15.
//  Copyright (c) 2015 Nenad Mancevic. All rights reserved.
//
//  Problem 1: Knapsack Problem
//
/***********************
In this programming problem and the next you'll code up the knapsack algorithm from lecture.
Let's start with a warm-up. Download the text file here (http://bit.ly/1O3eGd3). 
This file describes a knapsack instance, and it has the following format:
[knapsack_size][number_of_items]
[value_1] [weight_1]
[value_2] [weight_2]
...
For example, the third line of the file is "50074 659", indicating that the second item
has value 50074 and size 659, respectively.
You can assume that all numbers are positive. You should assume that item weights 
and the knapsack capacity are integers.
In the box below, type in the value of the optimal solution.
***********************/
//
//  Problem 2: Knapsack big
//
/***********************
This problem also asks you to solve a knapsack instance, but a much bigger one.
Download the text file here (http://bit.ly/1b8wL7H). 
This file describes a knapsack instance, and it has the following format:
[knapsack_size][number_of_items]
[value_1] [weight_1]
[value_2] [weight_2]
...
For example, the third line of the file is "50074 834558", indicating that the second item
has value 50074 and size 834558, respectively. As before, you should assume that item weights
and the knapsack capacity are integers.

This instance is so big that the straightforward iterative implemetation uses an infeasible 
amount of time and space. So you will have to be creative to compute an optimal solution. 
One idea is to go back to a recursive implementation, solving subproblems --- and, of course, 
caching the results to avoid redundant work --- only on an "as needed" basis. 
Also, be sure to think about appropriate data structures for storing and looking up solutions to subproblems.
In the box below, type in the value of the optimal solution.
************************/
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>    // std::max, std::min
#include "lib/profile.hpp"

namespace assignment3
{
    void read_file(const std::string& fileName, std::vector<int>& values, std::vector<int>& weights, int& W)
    {
        PROFILE("Read file for " + fileName);
        std::ifstream file(fileName, std::ios::in);
        if (file.is_open())
        {
            int n;
            file >> W >> n; // knapsack size, number of items

            int v, w;
            while (file >> v >> w)
            {
                values.push_back(v);
                weights.push_back(w);
            }
            file.close();
        }
        PROFILE_STOP();
    }

    namespace knapsack_1
    {
        int knapsack(const std::vector<int>& values, const std::vector<int>& weights, int W)
        {
            assert(values.size() == weights.size());    // values have to have coresponding weights
            size_t n = values.size();

            // define and initialize resulting matrix (rows are integral weights up to W (inclusive), so 0,1,2,3...,W, columns are items up to n (inclusive))
            std::vector<std::vector<int>> A(W+1, std::vector<int>(n+1));

            for (size_t i = 1; i <= values.size(); i++)
            {
                for (int x = 0; x <= W; x++)
                {
                    if (weights[i-1] > x)
                        A[x][i] = A[x][i - 1];
                    else
                        A[x][i] = std::max(A[x][i - 1], A[x - weights[i-1]][i - 1] + values[i-1]);
                }
            }

            return A[W][n]; // bottom right cell contains optimal value
        }

        bool testCase1()
        {
            std::vector<int> test_v = { 3,2,4,4 };
            std::vector<int> test_wt = { 4,3,2,3 };
            int test_W = 6;
            int optimal_value = knapsack(test_v, test_wt, test_W);

            assert(optimal_value == 8);
            return true;
        }

        void run_algorithm(const std::string& fileName)
        {
            std::vector<int> values;
            std::vector<int> weights;
            int W;

            read_file(fileName, values, weights, W);
            PROFILE("Knapsack 1")
            int optimal_value = knapsack(values, weights, W);
            PROFILE_STOP();
            std::cout << "Optimal value for knapsack problem is: " << optimal_value << std::endl;
        }
    } // namespace
    namespace knapsack_big
    {
        static std::unordered_map<int, std::vector<std::tuple<int,int>>> saved;  // key is W and value is item,value

        int knapsack_rec_memoization(int n, std::vector<int>& values, std::vector<int>& weights, const int W)
        {
            if (saved.count(W) > 0)
            {
                for (auto el : saved[W])
                {
                    if (std::get<0>(el) == n)   // if we have item's value for the given W and item number, let's return it
                        return std::get<1>(el);
                }
            }
            int withLastItem = -1;
            int withoutLastItem = -1;
            if (n <= 0)
                return 0;
           
            if (W < weights[n-1]) // last item
                withLastItem = -1;
            else
            {
                withLastItem = values[n - 1] + knapsack_rec_memoization(n - 1, values, weights, W - weights[n - 1]);
            }
            withoutLastItem = knapsack_rec_memoization(n - 1, values, weights, W);
            saved[W].push_back(std::make_tuple(n, std::max(withLastItem, withoutLastItem)));
            return std::max(withLastItem, withoutLastItem);
        }

        int knapsack_smallArray(std::vector<int>& values, std::vector<int>& weights, const int W)
        {
            // optimized version! (two arrays with at most W+1 elements)
            int *A = new int[W + 1];    // row 1 (current element being processed)
            int *B = new int[W + 1];    // row 0 (previous)
            for (int i = 0; i <= W; i++)
            {
                A[i] = 0;
                B[i] = 0;
            }

            for (size_t i = 1; i <= values.size(); i++)
            {
                for (int x = 1; x <= W; x++)
                {
                    if (weights[i - 1] > x)
                    {
                        A[x] = B[x];
                    }
                    else
                    {
                        A[x] = std::max(B[x], B[x - weights[i - 1]] + values[i - 1]);
                    }
                }

                // interchange only after row is filled (only swap pointers!)
                int* tmp = A;
                A = B;
                B = tmp;
            }

            int res = B[W]; // since the last swap, our final row is in B now
            delete[] A;
            delete[] B;
            return res;

        /**** Version that works too. Only a bit slower.
            //// define and initialize resulting matrix (rows are integral weights up to W (inclusive), so 0,1,2,3...,W, columns are items up to n (inclusive))
            /* std::vector<std::vector<int>> A;    // 2D-array 2x(n+1)
            A.resize(2);
            for (int i = 0; i < 2; i++)
            {
                A[i].resize(W + 1);
            }
            for (size_t i = 1; i <= values.size(); i++)
            {
                for (int x = 1; x <= W; x++)
                {
                    if (weights[i - 1] > x)
                    {
                        A[1][x] = A[0][x];
                    }
                    else
                    {
                        A[1][x] = std::max(A[0][x], A[0][x - weights[i - 1]] + values[i - 1]);
                    }
                }
                // interchange only after row is filled
                for (int x = 1; x <= W; x++)
                {
                    A[0][x] = A[1][x];  // interchange
                }
            }

            return A[1][W]; // bottom right cell contains optimal value
            *****/
        }

        bool testCase1()
        {
            std::vector<int> test_v = { 3,2,4,4 };
            std::vector<int> test_wt = { 4,3,2,3 };
            int test_W = 6;
            int optimal_value = knapsack_smallArray(test_v, test_wt, test_W); //knapsack_rec_memoization(4, test_v, test_wt, test_W);

            assert(optimal_value == 8);
            return true;
        }

        void run_algorithm(const std::string& fileName)
        {
            std::vector<int> values;
            std::vector<int> weights;
            int W;

            read_file(fileName, values, weights, W);

            PROFILE("Knapsack Recursive Memoization");
            int optimal_value = knapsack_rec_memoization((int)values.size(), values, weights, W);
            PROFILE_STOP();
            long long elapsedRM = tools::Profile::getInstance()->elapsed();

            assert(optimal_value == 4243395);

            PROFILE("Knapsack Original Smaller Array");
            optimal_value = knapsack_smallArray(values, weights, W);
            PROFILE_STOP();
            long long elapsedIter = tools::Profile::getInstance()->elapsed();

            assert(optimal_value == 4243395);

            std::cout << "Optimal value for big knapsack problem is: " << optimal_value << std::endl;
            if (elapsedIter < elapsedRM)
                std::cout << "Iterative algorithm outperforms recursive with memoization by: " << elapsedRM - elapsedIter << "ms" << std::endl;
            else
                std::cout << "Recursive algorithm with memoization outperforms iterative by: " << elapsedIter - elapsedRM << "ms" << std::endl;
        }
    } // namespace

    std::vector<std::vector<double>> binaryTree()
    {
        std::vector<int> vec = { 1,2,3,4,5,6,7 };
        std::vector<double> w = { 0.05, 0.4, 0.08,0.04,0.1,0.1,0.23 };

        std::vector<std::vector<double>> A(7, std::vector<double>(7));
        for (size_t s = 0; s < vec.size(); s++)
        {
            for (size_t i = 0; i < vec.size() - s; i++)
            {
                for (size_t r = i; r <= i + s; r++)
                {
                    double currMin = 999999999;
                    for (size_t k = i; k <= i + s; k++)
                    {
                        double resLeft = 0;
                        if ((int)i >= int(r - 1))
                            resLeft = 0;
                        else
                            resLeft = A[i][r - 1];

                        double resRight = 0;
                        if ((r + 1) >= (i + s))
                            resRight = 0;
                        else
                            resRight = A[r + 1][i + s];

                        currMin = std::min(currMin, w[k] + resLeft + resRight);
                    }
                    A[i][i + s] = currMin;
                }
            }
        }
        return A;
    }
} // namespace