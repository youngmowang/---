#pragma once
#include <QDateTime>
#include <cmath>

struct FsrsCard {
    int id = -1;
    int wordId = -1;
    int state = 0; 
    QDateTime due;
    double stability = 0.0;
    double difficulty = 0.0;
    int elapsedDays = 0;
    int scheduledDays = 0;
    int reps = 0;
    int lapses = 0;
    QDateTime lastReview;
};

struct FsrsRating {
    enum Rating {
        Again = 1,
        Hard = 2,
        Good = 3,
        Easy = 4
    };
};

class FsrsScheduler {
public:
    FsrsScheduler();
    
    FsrsCard schedule(FsrsCard card, FsrsRating::Rating rating, QDateTime now = QDateTime::currentDateTime());

private:
    double w[17] = {
        0.4, 0.6, 2.4, 5.8, 4.93, 0.94, 0.86, 0.01, 1.49, 0.14, 0.94, 
        2.18, 0.05, 0.34, 1.26, 0.29, 2.61
    };

    double init_stability(int rating);
    double init_difficulty(int rating);
    double next_difficulty(double d, int rating);
    double next_stability(double s, double d, int rating);
    double next_forget_stability(double s, double d, int rating);
    int next_interval(double s);
};
