#pragma once
#include <iostream>
#include<vector>
#include"nn/common.h"



namespace aivision {


enum DISTANCE_METRIC{COSINE=0,EUCLIDEAN};
std::vector<float> vectorNormalize(const std::vector<float> &pred);

double distance(std::vector<float> f1, std::vector<float> f2,DISTANCE_METRIC metric=COSINE);

std::vector<double> computeDistances(aivision::nn::ObjectAttributes query ,
                                      std::vector<ObjectAttributes> gallery,
                                      DISTANCE_METRIC metric=COSINE);




namespace utils{

float boxDistance(cv::Rect2f bbox1, cv::Rect2f bbox2, std::string metric="diou");
std::vector<std::pair<int,float>> objBoxSimilarities(aivision::nn::ObjectAttributes obj,std::vector<aivision::nn::ObjectAttributes> objects, std::string metric);

}

}
