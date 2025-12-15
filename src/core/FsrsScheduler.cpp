#include "FsrsScheduler.h"
#include <algorithm>

FsrsScheduler::FsrsScheduler() {
}

double FsrsScheduler::init_stability(int rating) {
    return std::max(0.1, w[rating - 1]);
}

double FsrsScheduler::init_difficulty(int rating) {
    return std::min(10.0, std::max(1.0, w[4] - w[5] * (rating - 3)));
}

double FsrsScheduler::next_difficulty(double d, int rating) {
    double next_d = d - w[6] * (rating - 3);
    return std::min(10.0, std::max(1.0, w[7] * w[4] + (1 - w[7]) * next_d));
}

double FsrsScheduler::next_stability(double s, double d, int rating) {
    if (rating == FsrsRating::Hard) {
        return s * (1 + std::exp(w[8]) * (11 - d) * std::pow(s, -w[9]) * (std::exp((1 - w[10]) * w[11]) - 1));
    } else if (rating == FsrsRating::Good) {
        return s * (1 + std::exp(w[8]) * (11 - d) * std::pow(s, -w[9]) * (std::exp(w[10] * w[11]) - 1));
    } else { 
        return s * (1 + std::exp(w[8]) * (11 - d) * std::pow(s, -w[9]) * (std::exp((1 + w[12]) * w[11]) - 1));
    }
}

double FsrsScheduler::next_forget_stability(double s, double d, int rating) {
    return std::min(s, w[11] * std::pow(d, -w[12]) * (std::pow(s + 1, w[13]) - 1) * std::exp(w[14] * (1 - rating)));
}

int FsrsScheduler::next_interval(double s) {
    int new_interval = std::round(s * 9); 
    return std::max(1, new_interval);
}

FsrsCard FsrsScheduler::schedule(FsrsCard card, FsrsRating::Rating rating, QDateTime now) {
    FsrsCard newCard = card;
    newCard.lastReview = now;
    newCard.reps += 1;

    if (card.state == 0) { 
        newCard.stability = init_stability(rating);
        newCard.difficulty = init_difficulty(rating);
        newCard.state = (rating == FsrsRating::Again) ? 1 : 2; 
        
        if (rating == FsrsRating::Again) {
             newCard.scheduledDays = 0;
             newCard.due = now.addSecs(60); 
        } else if (rating == FsrsRating::Hard) {
             newCard.scheduledDays = 0;
             newCard.due = now.addSecs(300); 
        } else if (rating == FsrsRating::Good) {
             newCard.scheduledDays = 0;
             newCard.due = now.addSecs(600); 
        } else { 
             newCard.scheduledDays = next_interval(newCard.stability);
             newCard.due = now.addDays(newCard.scheduledDays);
             newCard.state = 2; 
        }
    } else if (card.state == 1 || card.state == 3) { 
        if (rating == FsrsRating::Again) {
            newCard.scheduledDays = 0;
            newCard.due = now.addSecs(60);
        } else if (rating == FsrsRating::Good) {
            newCard.state = 2; 
            newCard.stability = init_stability(FsrsRating::Good); 
            newCard.difficulty = init_difficulty(FsrsRating::Good);
            newCard.scheduledDays = next_interval(newCard.stability);
            newCard.due = now.addDays(newCard.scheduledDays);
        }
    } else { 
        if (rating == FsrsRating::Again) {
            newCard.lapses += 1;
            newCard.state = 3; 
            newCard.stability = next_forget_stability(card.stability, card.difficulty, rating);
            newCard.difficulty = next_difficulty(card.difficulty, rating);
            newCard.scheduledDays = 0;
            newCard.due = now.addSecs(60);
        } else {
            newCard.stability = next_stability(card.stability, card.difficulty, rating);
            newCard.difficulty = next_difficulty(card.difficulty, rating);
            newCard.scheduledDays = next_interval(newCard.stability);
            newCard.due = now.addDays(newCard.scheduledDays);
        }
    }

    return newCard;
}
