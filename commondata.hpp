#ifndef COMMONDATA_HPP
#define COMMONDATA_HPP
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "utils/box_utils.h"

namespace aivision {


typedef struct{
    float score=0.;
    cv::Rect2f bbox;
    int label=-1;
    uint64 numClasses;
    std::vector<float> embed_vector;
} ObjAttributes;


struct ObjState
{
    cv::Point2f position;
    float area;
    float area_decay;
    float ratio;
    cv::Point2f speed;
    cv::Point2f acceleration;
    cv::Rect2f bbox;
};

struct StateParams{
    /* State parameters
     * describe object state
     */
    ObjState estimatedState;
    ObjState predictedState;

    cv::Point2f avg_speed;
    cv::Point2f avg_acceleration;
    cv::Point2f avg_direction;
    float gammaSpeed ;
    float gammaArea ;
    float gammaAreaDecay;
    float gammaRatio ;
    float gammaAcceleration ;
    float gammaPos ;
    float avg_area_decay;
    float gammaAvg;
    float avgDecayFactor = 1.0;
};

struct TrackletInfo
{

};

class TrackletStruct
{
public:
    bool isActive;
    bool isValid;
    bool isFinished;
    bool onTraject;
    bool isCounted;
    bool isValidForCount;
    int trackedFrames;
    int failedFrames;
    int trackingMaxWait = 5;
    int trajectFrames;
    std::vector<cv::Point2f> speeds, trace;
private:
    StateParams states;
    aivision::ObjAttributes oldAttr;

    void predict(){
        ObjState &est_st = this->states.estimatedState;
        ObjState &pred_st = this->states.predictedState;
        pred_st.speed = (est_st.speed + est_st.acceleration);
        //p_speed = cv::norm(speed)*avg_direction + acceleration;
        pred_st.position = est_st.position + pred_st.speed;
        pred_st.area = est_st.area * (est_st.area_decay);
        pred_st.ratio = est_st.ratio;
        pred_st.bbox = convertSpec2Box(pred_st.position, pred_st.area, pred_st.ratio);
        pred_st.acceleration = est_st.acceleration;
        pred_st.area_decay = est_st.area_decay;
    }
public:
    const ObjState& getPredictionState() const
    {
        return this->states.predictedState;
    }
    const ObjState& getEstimationState() const
    {
        return this->states.estimatedState;
    }
    TrackletStruct() {
        states.gammaSpeed = 0.9;
        states.gammaArea = 0.9;
        states.gammaAreaDecay = 0.9;
        states.gammaRatio = 0.9;
        states.gammaAcceleration = 0.9;
        states.gammaPos = 0.9;
    }

    void init(aivision::ObjAttributes init_attr)
    {
        oldAttr = init_attr;
        ObjState &est_st = this->states.estimatedState;
        auto box_spec = convertBox2Spec(init_attr.bbox);
        est_st.position = box_spec.center;
        est_st.area = box_spec.area;
        est_st.area_decay = 1.0;//pow(sections.dist_ratio_from_start_point,decay_factor);
        est_st.ratio = box_spec.ratio;
        est_st.speed = cv::Point2f(0,0);
        est_st.acceleration = cv::Point2f(0., 0.);
        isActive = true;
        isValid = true;
        isFinished = false;
        onTraject = false;
        isCounted = false;
        isValidForCount = false;
        trackedFrames = 0;
        trajectFrames = 0;
        failedFrames = 0;
        states.avg_speed = est_st.speed;
        states.avg_acceleration = cv::Point2f(0,0);
        states.avg_area_decay = 1.;
        states.gammaAvg = 0.9;

        predict();
    }
    void update(aivision::ObjAttributes new_attr)
    {
        auto new_bbox = new_attr.bbox;
        auto new_spec = convertBox2Spec(new_bbox);
        auto old_spec = convertBox2Spec(oldAttr.bbox);
        cv::Point2f measured_pos = new_spec.center;
        cv::Point2f measured_speed = new_spec.center - old_spec.center;
        auto measured_area = new_spec.area;
        auto measured_ratio = new_spec.ratio;
        auto measured_area_decay = new_spec.area/old_spec.area;
        ObjState &est_st = this->states.estimatedState;
        ObjState &pred_st = this->states.predictedState;
        cv::Point2f measured_acceleration;
        if(speeds.size()>2)
            measured_acceleration = measured_speed - est_st.speed;
        else
            measured_acceleration = cv::Point2f(0.,0.);
        cv::Point2f estimated_pos = pred_st.position + states.gammaPos * (measured_pos - pred_st.position);
        auto estimated_speed = pred_st.speed + states.gammaSpeed * (measured_speed - pred_st.speed );
        auto estimated_area = pred_st.area +  states.gammaArea * (measured_area - pred_st.area);
        auto estimated_area_decay = pred_st.area_decay +  states.gammaAreaDecay * (measured_area_decay - pred_st.area_decay);
        auto estimated_ratio = pred_st.ratio + states.gammaRatio * (measured_ratio - pred_st.ratio);
        auto estimated_acceleration = pred_st.acceleration +  states.gammaAcceleration * (measured_acceleration - pred_st.acceleration);

        if(estimated_area_decay>1.1)
            estimated_area_decay = 1.1;
        if(estimated_area_decay<0.9)
            estimated_area_decay = 0.9;
        est_st.speed = estimated_speed;
        est_st.area = estimated_area;
        est_st.area_decay = estimated_area_decay;
        est_st.ratio = estimated_ratio;
        est_st.acceleration = estimated_acceleration;
        est_st.position = estimated_pos;
        est_st.bbox = convertSpec2Box(est_st.position, est_st.area, est_st.ratio);

        states.avg_area_decay += states.gammaAvg*(est_st.area_decay - states.avg_area_decay);
        int p0 = trace.size()-4;
        if(p0<0)p0=0;
        speeds.emplace_back(est_st.speed);
        //check for movement:
        cv::Point2f last_p = trace.back();
        cv::Point2f avg_pos = last_p + 0.4*(estimated_pos - last_p);
        trace.push_back(avg_pos);
        cv::Point2f smv = avg_pos - last_p;
        states.avg_acceleration += 0.2*(smv - states.avg_speed - states.avg_acceleration);
        states.avg_speed += 0.2*(smv - states.avg_speed);

        // Predict for next step
        predict();

    }


    int checkStatus(int max_wait_to_track){
        int FRAMES_TO_CHANGE_STATUS = 4;
        int info = -1;
        if(trackedFrames>=1 & trackedFrames<FRAMES_TO_CHANGE_STATUS & trajectFrames<3)
            info = 1;
        else if(trackedFrames>=FRAMES_TO_CHANGE_STATUS & trajectFrames<max_wait_to_track){
            info = 2;
        }else{
            info = 0;
            isValid = false;
        }
        return info;
    }
    const StateParams &getStates() const;
};



struct ObjInfo{
    cv::Rect bbox;
    int avg_speed;
    bool stopped;
    int cls;
    float score;
    int id;
    cv::Scalar color;
    bool unauthorized_traffic = false;
    bool unauthorized_stop = false;
    bool deviation;
    bool reverse;
    bool lane_change;
    std::vector<int> lane_sequence;
    bool valid_speed;
    int tracked_frames = 0;
    int lane;
    uint64 capture_time = 0;
    uint64 finished_time = 0;
    uint64 stop_time=0;
    float max_deviation=0;
    std::vector<cv::Point2d> trace;
    float between_lanes_ratio = 0;
    bool between_lanes = false;
};


struct PTZC{
    float x;
    float y;
    float z;
};


enum Det {
    tl_x = 0,
    tl_y = 1,
    br_x = 2,
    br_y = 3,
    score = 4,
    class_idx = 5
};

typedef struct{
    cv::Point2f pt;
    int lane;
    int intrusion_section;

}TraceInfo;



struct Voter{
private:
    std::vector<std::pair<float, int>> voters;
public:
    int vote = -1;
    int last_id = -1;
    float last_score = 0;
    float maxScore = 0.;
    int continuous_id = -1;
    int continuous_count = 0;
    float continuous_score = 0;
    void Init(int num_classes)
    {
        for(int i=0;i<num_classes;i++)
            voters.push_back({0.0,1});//score,num_updates
    }
    std::pair<float, int> GetSecond()
    {
        int mi=0;
        float ms=0;
        for(int i=0;i<this->voters.size();i++)
        {
            if(i==vote)continue;
            if(ms<this->voters.at(i).first){
                ms = this->voters.at(i).first;
                mi = i;
            }
        }
        return {ms,mi};
    }
    void Update(int cls, float score,bool apply_all=false){
        last_id = cls;
        last_score = score;
        if(cls>=voters.size() | cls<0)
            return;
        voters.at(cls).first = voters.at(cls).first + 1./(float)voters.at(cls).second * (score - voters.at(cls).first);
        if(apply_all)
            for(int c=0;c<voters.size();c++)
            {
                if(c==cls)
                    continue;
                voters.at(c).first = voters.at(c).first + 1./(float)voters.at(c).second * (0.0f - voters.at(c).first);

            }
        voters.at(cls).second++;
        if(voters.at(cls).first>maxScore){
            maxScore = voters.at(cls).first;
            vote = cls;
        }

    }
    void ResetContinuous()
    {
        continuous_id = -1;
        continuous_score = 0;
        continuous_count = 0;
    }

    void SumUpdate(int cls, float score){
        if(continuous_id<0)
        {
            continuous_id = cls;
            continuous_score = score;
            continuous_count = 1;
        }else
        {
            if(continuous_id==cls)
            {
                continuous_id = cls;
                continuous_score += score;
                continuous_count += 1;
            }
        }
        last_id = cls;
        last_score = score;
        if(cls>=voters.size() | cls<0)
            return;
        voters.at(cls).first += score;
        voters.at(cls).second++;
        if(voters.at(cls).first>maxScore){
            maxScore = voters.at(cls).first;
            vote = cls;
        }

    }
};

inline const StateParams &TrackletStruct::getStates() const
{
    return states;
}



}


#endif // COMMON_HPP
