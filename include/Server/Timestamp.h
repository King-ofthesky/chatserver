#pragma once 

#include<iostream>
#include<string>
#include<time.h>

class Timestamp{
    public:

    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch){};

    // explicit Timestamp(int64_t microSecondsSinceEpoch);

    static Timestamp now();

    std::string toString() const;

    protected:

    private:  
    int64_t microSecondsSinceEpoch_;//表示时间  64位有符合整数

};