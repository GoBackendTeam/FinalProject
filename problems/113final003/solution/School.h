#ifndef SCHOOL_H_
#define SCHOOL_H_

#include <cmath>
#include <iostream>
#include <string>


class School {
protected:
    std::string name;
    int studentAmount;
    int studentAmountNextYear;

public:
    School(std::string name, int studentAmount)
        : name(name)
        , studentAmount(studentAmount)
        , studentAmountNextYear(studentAmount) {}

    virtual ~School() {}

    virtual void admissions(float amount) {
        if (amount <= 0 || std::floor(amount) != amount) {
            std::cout << "Invalid amount" << std::endl;
            return;
        }
        studentAmount += static_cast<int>(amount);
    }

    virtual void dropouts(float amount) {
        if (amount <= 0 || std::floor(amount) != amount) {
            std::cout << "Invalid amount" << std::endl;
            return;
        }
        if (amount > studentAmount) {
            std::cout << "Invalid student count" << std::endl;
            return;
        }
        studentAmount -= static_cast<int>(amount);
    }

    virtual void transfer(float amount, School& toSchool) {
        if (amount <= 0 || std::floor(amount) != amount) {
            std::cout << "Invalid amount" << std::endl;
            return;
        }
        if (amount > studentAmount) {
            std::cout << "Invalid student count" << std::endl;
            return;
        }
        this->dropouts(amount);
        toSchool.admissions(amount);
    }

    friend std::ostream& operator<<(std::ostream& os, const School& s) {
        os << s.name << "\t" << s.studentAmount << "\t" << s.studentAmountNextYear;
        return os;
    }
};

class PrivateSchool : public School {
private:
    int dropoutCount;

public:
    PrivateSchool(std::string name, int studentAmount)
        : School(name, studentAmount)
        , dropoutCount(0) {}

    void dropouts(float amount) override {
        if (amount <= 0 || std::floor(amount) != amount) {
            std::cout << "Invalid amount" << std::endl;
            return;
        }
        if (amount > studentAmount) {
            std::cout << "Invalid student count" << std::endl;
            return;
        }

        School::dropouts(amount);
        dropoutCount++;
        if (dropoutCount > 1) {
            studentAmountNextYear -= 100;
        }
    }
};

class PublicSchool : public School {
private:
    double growing_rate;

public:
    PublicSchool(std::string name, int studentAmount)
        : School(name, studentAmount)
        , growing_rate(0.05) {}

    void apply_growth() {
        studentAmountNextYear += static_cast<int>(std::ceil(studentAmountNextYear * growing_rate));
    }

    void dropouts(float amount) override {
        if (amount <= 0 || std::floor(amount) != amount) {
            std::cout << "Invalid amount" << std::endl;
            return;
        }
        if (amount > studentAmount) {
            std::cout << "Invalid student count" << std::endl;
            return;
        }

        School::dropouts(amount);

        if (amount > 100) {
            studentAmountNextYear -= static_cast<int>(std::ceil(studentAmountNextYear * 0.05));
        }
    }
};

#endif // SCHOOL_H_
